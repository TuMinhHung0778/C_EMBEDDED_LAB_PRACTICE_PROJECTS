# Build (see embedded-simulator-practice.md).
# From this directory:
#   make all
#   ./scheduler_sim   (or scheduler_sim.exe on Windows)
#   ./uart_sim        (or uart_sim.exe on Windows)
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

SCHED_DIR = scheduler
SCHED_SRC = $(SCHED_DIR)/scheduler.c $(SCHED_DIR)/main.c
SCHED_OUT = scheduler_sim

UART_DIR = uart-ringbuffer
UART_SRC = $(UART_DIR)/ringbuffer.c $(UART_DIR)/parser.c $(UART_DIR)/main.c
UART_OUT = uart_sim

.PHONY: all clean scheduler uart

all: scheduler uart

scheduler: $(SCHED_SRC) $(SCHED_DIR)/scheduler.h
	$(CC) $(CFLAGS) $(SCHED_SRC) -o $(SCHED_OUT) -lm

uart: $(UART_SRC) $(UART_DIR)/ringbuffer.h $(UART_DIR)/parser.h
	$(CC) $(CFLAGS) $(UART_SRC) -o $(UART_OUT)

clean:
	rm -f $(SCHED_OUT) $(SCHED_OUT).exe $(UART_OUT) $(UART_OUT).exe
