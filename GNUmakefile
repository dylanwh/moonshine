

all: configure compile

build:
	mkdir -p build

configure: build
	cd build && cmake ..
	
compile: configure
	make -C build --no-print-directory moonshine

purge:
	rm -rf build

install:
	make --no-print-directory -C build install

test:
	make --no-print-directory -C build test

api-docs:
	make --no-print-directory -C build api-docs

posthook:
	-perl /srv/darcs/changes | perl /srv/darcs/subbotclient.pl

.PHONY: all posthook purge install test configure compile 
