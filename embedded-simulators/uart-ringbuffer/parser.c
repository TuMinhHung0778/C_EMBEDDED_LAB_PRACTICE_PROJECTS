#include "parser.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>

static void reset_buffers(Parser *p)
{
    p->name_len = 0U;
    p->param_len = 0U;
    p->is_query = 0U;
    p->saw_cr = 0U;
    p->cmd_name[0] = '\0';
    p->param[0] = '\0';
}

static void to_output(const Parser *p, ParsedCommand *out)
{
    memset(out, 0, sizeof(*out));
    out->ready = true;
    out->is_query = (p->is_query != 0U);
    /* Bounded copy with guaranteed null-termination; avoid strncpy truncation warnings. */
    (void)memcpy(out->cmd_name, p->cmd_name, sizeof(out->cmd_name) - 1U);
    out->cmd_name[sizeof(out->cmd_name) - 1U] = '\0';
    (void)memcpy(out->param, p->param, sizeof(out->param) - 1U);
    out->param[sizeof(out->param) - 1U] = '\0';
}

static bool finalize_if_crlf(Parser *p, uint8_t ch, ParsedCommand *out_cmd)
{
    if (p->saw_cr && ch == (uint8_t)'\n') {
        to_output(p, out_cmd);
        p->cmd_count++;
        p->state = STATE_IDLE;
        reset_buffers(p);
        return true;
    }
    if (ch == (uint8_t)'\r') {
        p->saw_cr = 1U;
    } else {
        p->saw_cr = 0U;
    }
    return false;
}

static void parser_error(Parser *p)
{
    p->parse_errors++;
    p->state = STATE_SKIP_TO_EOL;
    p->saw_cr = 0U;
    reset_buffers(p);
}

void parser_init(Parser *p)
{
    assert(p != NULL);
    memset(p, 0, sizeof(*p));
    p->state = STATE_IDLE;
    reset_buffers(p);
}

bool parser_feed(Parser *p, uint8_t ch, ParsedCommand *out_cmd)
{
    assert(p != NULL);
    assert(out_cmd != NULL);
    out_cmd->ready = false;

    switch (p->state) {
    case STATE_IDLE:
        if (ch == (uint8_t)'\r' || ch == (uint8_t)'\n') {
            return false;
        }
        if (ch == (uint8_t)'A') {
            p->state = STATE_SEEN_A;
            return false;
        }
        /* Any non-empty line not starting with 'A' is malformed for this parser. */
        parser_error(p);
        return false;

    case STATE_SEEN_A:
        if (ch == (uint8_t)'T') {
            p->state = STATE_SEEN_AT;
        } else {
            p->state = STATE_IDLE;
        }
        return false;

    case STATE_SEEN_AT:
        if (ch == (uint8_t)'\r') {
            p->saw_cr = 1U;
            p->cmd_name[0] = 'O';
            p->cmd_name[1] = 'K';
            p->cmd_name[2] = '\0';
            p->state = STATE_CMD_NAME;
            return false;
        }
        if (ch == (uint8_t)'\n') {
            /* accept bare "AT\n" as OK */
            p->cmd_name[0] = 'O';
            p->cmd_name[1] = 'K';
            p->cmd_name[2] = '\0';
            to_output(p, out_cmd);
            p->cmd_count++;
            p->state = STATE_IDLE;
            reset_buffers(p);
            return true;
        }
        if (ch == (uint8_t)'+') {
            p->state = STATE_CMD_NAME;
            return false;
        }
        /* Any other char after AT is invalid per our supported grammar. */
        parser_error(p);
        return false;

    case STATE_CMD_NAME:
        if (ch == (uint8_t)'\r') {
            p->cmd_name[p->name_len] = '\0';
            p->saw_cr = 1U;
            return false;
        }
        if (ch == (uint8_t)'\n') {
            p->cmd_name[p->name_len] = '\0';
            to_output(p, out_cmd);
            p->cmd_count++;
            p->state = STATE_IDLE;
            reset_buffers(p);
            return true;
        }
        if (ch == (uint8_t)'?') {
            p->is_query = 1U;
            p->cmd_name[p->name_len] = '\0';
            p->state = STATE_CMD_PARAM; /* reuse CRLF finalize helper */
            return false;
        }
        if (ch == (uint8_t)'=') {
            p->cmd_name[p->name_len] = '\0';
            p->state = STATE_CMD_PARAM;
            return false;
        }

        if (!isalpha((int)ch) && !isdigit((int)ch) && ch != (uint8_t)'_') {
            parser_error(p);
            return false;
        }
        if (p->name_len + 1U >= (uint8_t)sizeof(p->cmd_name)) {
            parser_error(p);
            return false;
        }
        p->cmd_name[p->name_len++] = (char)ch;
        p->cmd_name[p->name_len] = '\0';
        return false;

    case STATE_CMD_PARAM:
        /* After '?' we only accept CRLF; after '=' we accept param bytes until CRLF. */
        if (p->is_query) {
            return finalize_if_crlf(p, ch, out_cmd);
        }

        if (ch == (uint8_t)'\r' || ch == (uint8_t)'\n') {
            p->param[p->param_len] = '\0';
            return finalize_if_crlf(p, ch, out_cmd);
        }

        if (p->param_len + 1U >= (uint8_t)sizeof(p->param)) {
            parser_error(p);
            return false;
        }
        p->param[p->param_len++] = (char)ch;
        p->param[p->param_len] = '\0';
        return false;

    case STATE_SKIP_TO_EOL:
        /* Recovery: ignore everything until CRLF, then go idle. */
        if (p->saw_cr && ch == (uint8_t)'\n') {
            p->state = STATE_IDLE;
            p->saw_cr = 0U;
            return false;
        }
        p->saw_cr = (ch == (uint8_t)'\r') ? 1U : 0U;
        return false;

    default:
        parser_error(p);
        return false;
    }
}

