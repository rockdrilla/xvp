#!/usr/bin/make -f
SHELL       :=/bin/sh
.SHELLFLAGS :=-efc
MAKEFLAGS   +=--no-print-directory

CROSS ?=
CC    =$(CROSS)gcc
CXX   =$(CROSS)g++
STRIP =$(CROSS)strip

CFLAGS   ?=-O2 -g0 -fPIE -fstack-protector-strong
CPPFLAGS ?=-Wall -Wextra -Werror=format-security -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2
CXXFLAGS ?=
LDFLAGS  ?=-Wl,-z,relro -Wl,-z,now -pie

NO_WARN = attributes class-memaccess unused-function unused-result
CPPFLAGS += $(foreach w,$(NO_WARN),-Wno-$(w))

NO_CXX = rtti exceptions
CXXFLAGS +=$(foreach f,$(NO_CXX),-fno-$(f))

.DEFAULT: all
.PHONY: all clean
all: xvp

%.c.o: %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) -o $@ $^

%.cc.o: %.cc
	$(CXX) -c $(CFLAGS) $(CPPFLAGS) $(CXXFLAGS) -o $@ $^

xvp: xvp.cc.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ && \
	$(STRIP) --strip-unneeded $@

clean:
	rm -f xvp xvp.cc.o
