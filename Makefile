all:
	mkdir -p build
	cd build && cmake .. && make --no-print-directory


posthook:
	-perl /srv/darcs/changes | perl /srv/darcs/subbotclient.pl
