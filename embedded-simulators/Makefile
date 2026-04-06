# Project 1 build (see embedded-simulator-practice.md).
# From this directory: make && ./scheduler_sim   (add .exe on Windows)
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -O2

SCHED_DIR = scheduler
SCHED_SRC = $(SCHED_DIR)/scheduler.c $(SCHED_DIR)/main.c
SCHED_OUT = scheduler_sim

.PHONY: all clean scheduler

all: scheduler

scheduler: $(SCHED_SRC) $(SCHED_DIR)/scheduler.h
	$(CC) $(CFLAGS) $(SCHED_SRC) -o $(SCHED_OUT) -lm

clean:
	rm -f $(SCHED_OUT) $(SCHED_OUT).exe
