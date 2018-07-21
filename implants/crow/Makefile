
CC = gcc
CFLAGS += -Wall -Werror -Wextra

.PHONY: all clean run test_process_runner

all: crow

crow: crow.o logc.o
	$(CC) $(CFLAGS) -o crow crow.o logc.o

crow.o: crow.c
	$(CC) $(CFLAGS) -c crow.c

logc.o: logc/src/log.h logc/src/log.c
	$(CC) $(CFLAGS) -c -o logc.o logc/src/log.c

process_runner.o: process_runner.c process_runner.h
	$(CC) $(CFLAGS) -c process_runner.c

test_process_runner: process_runner.c process_runner.h
	$(CC) $(CFLAGS) -DTESTING -c process_runner.c
	$(CC) $(CFLAGS) -o test_process_runner process_runner.o logc.o
	./test_process_runner

clean:
	rm -f *.o crow
	rm -f test_process_runner
	rm -f a.out

run:
	./crow -vvvv -b 0

