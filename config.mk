CC        := gcc
LDFLAGS   :=  -lslang $(shell pkg-config glib-2.0 gnet-2.0 --libs)
CFLAGS    := -g -Winline -Wall -Werror -std=gnu99 -I/usr/include/slang-2 $(shell pkg-config glib-2.0 gnet-2.0 --cflags)
OBJCFLAGS := $(CFLAGS) -fgnu-runtime #-fobjc-direct-dispatch


ifdef DEBIAN
CFLAGS += -I/usr/include/lua5.1
endif
