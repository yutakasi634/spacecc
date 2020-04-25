CFLAGS=-std=c11 -g -static

spacecc: spacecc.c

test: spacecc
	./test.sh

clean:
	rm -f spacecc *.o *~ tmp*

.PHONY: test clean
