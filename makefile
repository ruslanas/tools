CC=gcc
CFLAGS = -Wall -s -O3
TARGET = scan hx

all: $(TARGET)

scan: scan.c
	$(CC) $(CFLAGS) scan.c -o bin/scan.exe

hx: hx.c
	$(CC) $(CFLAGS) hx.c -o bin/hx.exe
