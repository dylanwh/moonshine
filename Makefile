include config.mk

haver: protocol.o haver.o

clean:
	rm -f *.o haver

.PHONY: clean
