CC=cc
FLAGS=-Wall -Wextra -Werror

all:
	${CC} ${FLAGS} -o cgi cgi.c cgi.h

clean:
	rm -f ./cgi ./cgi.c ./cgi.h
