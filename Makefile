CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -pedantic -pthread

all: pressure_test

pressure_test: pressure_test.c ts_queue.c ts_queue.h
	$(CC) $(CFLAGS) pressure_test.c ts_queue.c -o pressure_test

run: pressure_test
	./pressure_test

clean:
	rm -f pressure_test pressure_test.exe
