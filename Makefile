include config.mk

spoon: closure.o keyboard.o screen.o term.o protocol.o main.o
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


-include .depends
.depends: $(wildcard *.c *.m)
	@gcc -MM $^ > $@

.PHONY: clean
