CC        := gcc
LDFLAGS   :=  -lgc -lslang $(shell pkg-config glib-2.0 --libs)
CFLAGS    := -Wall -std=gnu99 $(shell pkg-config glib-2.0 --cflags)
OBJCFLAGS := $(CFLAGS)
