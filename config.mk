PACKAGES = lua glib-2.0 gnet-2.0

LDFLAGS   := -lslang $(shell pkg-config $(PACKAGES) --libs)
CFLAGS    := -g -Winline -Wall -Werror -std=gnu99 -I/usr/include/slang-2 \
			 $(shell pkg-config $(PACKAGES) --cflags)
