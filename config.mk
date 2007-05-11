CC        := gcc
LDFLAGS   :=  -llua -lslang $(shell pkg-config glib-2.0 gnet-2.0 --libs)
CFLAGS    := -O2 -Wall -Werror -std=gnu99 -I/usr/include/slang-2 $(shell pkg-config glib-2.0 gnet-2.0 --cflags)
OBJCFLAGS := $(CFLAGS) -fgnu-runtime #-fobjc-direct-dispatch
