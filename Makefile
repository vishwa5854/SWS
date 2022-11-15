PROG=	sws
OBJS=	main.o util.o readdirs.o sws.o handler.o
CFLAGS=	-Wall -Werror -Wextra -g

all: ${PROG}

depend:
	mkdep -- ${CFLAGS} *.c

${PROG}: ${OBJS}
	@echo $@ depends on $?
	${CC} -lm ${OBJS} -o ${PROG} ${LDFLAGS}

clean:
	rm -f ${PROG} ${OBJS}