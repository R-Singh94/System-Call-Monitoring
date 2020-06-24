CC=gcc
CFLAGS=-g -Wall -Wformat=0
DIR= ${CURDIR}

all: monitor LOG_DIR

monitor: file_stat.o monitor.o
	$(CC) $(CFLAGS) -o monitor monitor.o file_stat.o

monitor.o: monitor.c
	$(CC) $(CFLAGS) -c monitor.c

file_stat.o: file_stat.h file_stat.c
	$(CC) $(CFLAGS) -c file_stat.c

LOG_DIR:
	mkdir -p $(DIR)/logs

clean:
	rm file_stat.o monitor.o monitor
	rm -rf logs