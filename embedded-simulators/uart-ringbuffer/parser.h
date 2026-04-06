#ifndef PARSER_H
#define PARSER_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    STATE_IDLE = 0,
    STATE_SEEN_A,
    STATE_SEEN_AT,
    STATE_CMD_NAME,
    STATE_CMD_PARAM,
    STATE_SKIP_TO_EOL,
} ParserState;

typedef struct {
    ParserState state;

    char cmd_name[16];
    char param[32];
    uint8_t name_len;
    uint8_t param_len;
    uint8_t is_query;
    uint8_t saw_cr;

    uint32_t cmd_count;
    uint32_t parse_errors;
} Parser;

typedef struct {
    bool ready;          /* one command completed */
    bool is_query;       /* ends with '?' */
    char cmd_name[16];   /* null-terminated */
    char param[32];      /* null-terminated (may be empty) */
} ParsedCommand;

void parser_init(Parser *p);

/* Feed one byte. Returns true if it completed a command and filled out_cmd. */
bool parser_feed(Parser *p, uint8_t ch, ParsedCommand *out_cmd);

#endif
