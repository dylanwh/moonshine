include config.mk

spoon: closure.o signal.o keyboard.o screen.o term.o protocol.o main.o buffer.o
	@echo LD $@
	@$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	@echo CC $<
	@$(CC) $(CFLAGS) -c $<

%.o: %.m
	@echo CC $<
	@$(CC) $(CFLAGS) $(OBJCFLAGS) -c $<

clean:
	rm -f *.o spoon

posthook:
	-perl /srv/darcs/changes | perl /srv/darcs/subbotclient.pl

.PHONY: clean posthook

-include .depends
.depends: $(wildcard *.c *.m)
	@gcc -MM $^ > $@
