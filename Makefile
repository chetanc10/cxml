
CFLAGS := -Wall
CFLAGS += -O0
CFLAGS += -g

all:
	gcc ${CFLAGS} *.c

clean:
	rm -rf a.out *.o
