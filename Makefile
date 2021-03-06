HDR = lz78.o bitio.o main.o info.o
SRC = ${HDR:.h=.c}
ESRC = main.c
CFLAGS=-Wall -O2 -Werror
BIN=/bin
RM= rm

all: lz78
	
	
lz78: $(HDR)
	$(CC) $(CFLAGS) $(HDR) -o lz78 -lcrypto

main.o:	main.c lz78.h
		cc $(CFLAGS) -c main.c

lz78.o: lz78.c lz78.h bitio.h info.h
		cc $(CFLAGS) -c lz78.c

bitio.o: bitio.c bitio.h
		cc $(CFLAGS) -c bitio.c

info.o:	info.c info.h bitio.h
		cc $(CFLAGS) -c info.c 

install:
	@echo \install..\N
	@sudo install -g 0 -o 0 -m 0644 man/lz78.1 /usr/share/man/man1/
	@sudo gzip /usr/share/man/man1/lz78.1
	@sudo mandb
	@sudo cp lz78 $(BIN) && echo Install OK

clean:
	@if test -s "lz78" | test -s "bitio.o" | test -s "lz78.o" | test -s "info.o" | test -s "main.o"; \
	then\
		$(RM) lz78 main.o bitio.o info.o lz78.o && echo Files removed.. ; \
		else\
			echo Clean...; \
	fi ;


	
