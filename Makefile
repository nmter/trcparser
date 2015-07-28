CC := gcc
CFLAGS := -Wall -g
DIR := $(shell pwd)
TRCPARSER_SRC := $(wildcard *.c)
TRCPARSER_OBJS_N := $(patsubst %.c,%.o,$(TRCPARSER_SRC))

all: trcparser
	# @echo $(DIR) 
	@echo $(TRCPARSERSRC)

TRCPARSER_OBJS := $(TRCPARSERSRC:.c=.o)
$(TRCPARSER_OBJS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
trcparser: $(TRCPARSER_OBJS_N)
	echo $(TRCPARSER_SRC)
	echo $(TRCPARSER_OBJS_N)
	$(CC) $(CFLAGS) -o $@ $(TRCPARSER_OBJS_N)
clean:
	rm trcparser $(TRCPARSER_OBJS_N)