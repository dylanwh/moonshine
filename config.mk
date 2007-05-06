CC        := gcc
LDFLAGS   :=  -lslang $(shell pkg-config glib-2.0 --libs)
CFLAGS    := -Wall -Werror -std=gnu99 $(shell pkg-config glib-2.0 --cflags)
OBJCFLAGS := $(CFLAGS)
