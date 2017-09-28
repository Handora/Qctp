CC=gcc
CFLAGS=-I. -lssl
DEPS = parse.h y.tab.h
OBJ = liso.o daemon.o pool.o ssl.o cgi.o loog.o parse.o y.tab.o lex.yy.o util.o get.o head.o post.o qio.o response.o
FLAGS = -g -Wall
PRODUCT = -D PRODUCTION

default:all

all: liso

lex.yy.c: lexer.l
	flex -o $@ $^

y.tab.c: parser.y
	yacc -o $@ -d $^

*.o: *.c
	$(CC) $(FLAGS) -c -o $@ $< $(CFLAGS) $(PRODUCT)

liso: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(PRODUCT)

clean:
	rm -f *~ $(OBJ) liso lex.yy.c y.tab.c y.tab.h logFile
