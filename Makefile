.PHONY: all
all: Coder

Coder:	code.c queue.o
		gcc code.c -o code -L. ./libCodec.so -pthread

queue.o:
	gcc -c queue.c -o queue.o

.PHONY: clean
clean:
	-rm code queue.o 2>/dev/null 