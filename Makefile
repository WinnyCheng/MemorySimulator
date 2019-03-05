CFLAGS = -g

all: pa4

pa4: pa4.o
	gcc $(CFLAGS) -Wall pa4.o -o pa4

pa4.o: pa4.c
	gcc $(CFLAGS) -Wall -c pa4.c

clean:
	rm -f *o pa4