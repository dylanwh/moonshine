
all: build
	cd build && cmake .. && make --no-print-directory moonshine

install:
	make --no-print-directory -C build install

test:
	make --no-print-directory -C build test

api-docs:
	make --no-print-directory -C build api-docs

posthook:
	#-perl /srv/darcs/changes | perl /srv/darcs/subbotclient.pl

purge:
	rm -rf build

build:
	mkdir -p build

.PHONY: all posthook purge install test
