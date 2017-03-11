CC=gcc
CFLAGS = -Wall -s -O3
TARGET = hx relocate

all: $(TARGET)

scan: scan.c
	$(CC) $(CFLAGS) scan.c -o bin/scan.exe

hx: hx.c
	$(CC) $(CFLAGS) hx.c -o bin/hx.exe -lm

parser: parser.c
	$(CC) $(CFLAGS) parser.c -o bin/parser.exe

pe: pe.c types.h
	$(CC) $(CFLAGS) pe.c types.h -o bin/pe.exe

relocate: relocate.c
	$(CC) $(CFLAGS) relocate.c -o bin/relocate

run: relocate
	relocate -r ../maggot/egg.img ./egg.img
