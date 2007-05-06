include config.mk

haver: hacks.o protocol.o haver.o


clean:
	rm -f *.o haver
