HDR = lz78.o bitio.o main.o info.o
SRC = ${HDR:.h=.c}
ESRC = main.c
CFLAGS=-Wall
BIN=/bin

all: lz78
	cp src/lz78 lz78
	
lz78: src/$(HDR)
	$(CC) $(CFLAGS) src/$(HDR) -o src/lz78

main.o:	src/main.c src/lz78.h
		cc -c src/main.c

lz78.o: src/lz78.c src/lz78.h src/bitio.h src/info.h
		cc -c src/lz78.c

bitio.o: src/bitio.c src/bitio.h
		cc -c src/bitio.c

info.o:	src/info.c src/info.h src/bitio.h
		cc -c src/info.c 

install:
	@echo \install..\N
	@sudo install -g 0 -o 0 -m 0644 man/lz78.1 /usr/share/man/man1/
	@sudo gzip /usr/share/man/man1/lz78.1
	@sudo mandb
	@sudo cp src/lz78 $(BIN) && echo Install OK

clean:
	$(RM) src/lz78
