all: build 

build: lib.o ksender kreceiver 

lib.o: lib.c lib.h
	gcc -c lib.c -o $@

ksender: ksender.o link_emulator/lib.o lib.h
	gcc -g -O2 ksender.o link_emulator/lib.o lib.o -o ksender

kreceiver: kreceiver.o link_emulator/lib.o lib.h
	gcc -g -O2 kreceiver.o link_emulator/lib.o lib.o -o kreceiver

.c.o: 
	gcc -Wall -g -c $? 

clean:
	-rm -f *.o ksender kreceiver 
