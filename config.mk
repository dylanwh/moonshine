CC        := gcc
LDFLAGS   :=  -lobjc -lslang $(shell pkg-config glib-2.0 gnet-2.0 --libs)
CFLAGS    := -Wall -Werror -std=gnu99 $(shell pkg-config glib-2.0 gnet-2.0 --cflags)
OBJCFLAGS := $(CFLAGS) -fgnu-runtime #-fobjc-direct-dispatch
