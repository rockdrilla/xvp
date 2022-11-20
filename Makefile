#!/usr/bin/make -f
SHELL       :=/bin/sh
.SHELLFLAGS :=-efc
MAKEFLAGS   +=--no-print-directory

CFLAGS   ?=-O2 -g0 -fPIE -fstack-protector-strong
CPPFLAGS ?=-Wall -Wextra -Werror=format-security -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2
LDFLAGS  ?=-Wl,-z,relro -Wl,-z,now

NO_WARN = attributes unused-function unused-parameter unused-result
CPPFLAGS += $(foreach w,$(NO_WARN),-Wno-$(w))

.DEFAULT: all
.PHONY: all
all: xvp

xvp: xvp.c
	gcc $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f xvp
