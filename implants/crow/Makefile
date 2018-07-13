
CC = gcc
CFLAGS += -Wall -Werror -Wextra

all: crow

crow: crow.o logc.o
	$(CC) $(CFLAGS) -o crow crow.o logc.o

crow.o: crow.c
	$(CC) $(CFLAGS) -c crow.c

logc.o: logc/src/log.h logc/src/log.c
	$(CC) $(CFLAGS) -c -o logc.o logc/src/log.c

clean:
	rm -f *.o crow

run:
	./crow -vvvv -b 0

