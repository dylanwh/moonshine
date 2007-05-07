include config.mk

haver: screen.o ui.o protocol.o haver.o

clean:
	rm -f *.o haver

.PHONY: clean
