
all: build
	cd build && cmake .. && make --no-print-directory

install:
	make --no-print-directory -C build install

posthook:
	-perl /srv/darcs/changes | perl /srv/darcs/subbotclient.pl

purge:
	rm -rf build

build:
	mkdir -p build

.PHONY: all posthook purge install
