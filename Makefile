CFLAGS += -std=c89 -pedantic
CFLAGS += -O3
CFLAGS += -Wno-variadic-macros -Wno-long-long -Wall -Werror
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -g0 -fno-unwind-tables -s
CFLAGS += -fno-asynchronous-unwind-tables

RELEASEFLAGS = -static -no-pie

LDFLAGS += -lm

UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
CC ?= clang
endif
ifeq ($(UNAME), Linux)
CC ?= gcc
endif

.PHONY: bin release lib clean
default: bin
bin: main.c oppai.c
	$(CC) $(CFLAGS) $(LDFLAGS) main.c -o oppai
release: main.c oppai.c
	$(CC) -static -no-pie $(CFLAGS) main.c -o oppai $(LDFLAGS)
lib: main.c oppai.c
	$(CC) $(CFLAGS) $(LDFLAGS) -DOPPAI_IMPLEMENTATION -c -fpic oppai.c
	$(CC) $(CFLAGS) $(LDFLAGS) -DOPPAI_IMPLEMENTATION -shared oppai.o -o liboppai.so
clean:
	rm -f *.o *.so *.tar.xz
