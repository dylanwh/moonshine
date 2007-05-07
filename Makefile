include config.mk

spoon: keymap.o screen.o ui.o protocol.o spoon.o


%.o: %.c
	@echo CC $<
	@$(CC) $(CFLAGS) -c $<

%.o: %.m
	@echo CC $<
	@$(CC) $(CFLAGS) $(OBJCFLAGS) -c $<

clean:
	rm -f *.o spoon

.PHONY: clean
