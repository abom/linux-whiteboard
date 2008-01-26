# Temporary solution until it is changed to autoconf
CC = gcc
CFLAGS += -Wall -Wextra -O0 -g -ggdb -pipe

demo: main.o wii.o matrix.o auxiliary.o
	${CC} ${CFLAGS}	main.o wii.o matrix.o auxiliary.o -o demo \
		-lcwiid `pkg-config --libs xtst` `sdl-config --libs`

clean:
	rm -vf demo release.tar.gz *.o

main.o: main.c matrix.h wii.h auxiliary.h common.h
	${CC} ${CFLAGS} -c main.c `sdl-config --cflags`

wii.o: wii.c matrix.h common.h
	${CC} ${CFLAGS} -c wii.c

matrix.o: matrix.c common.h
	${CC} ${CFLAGS} -c matrix.c

auxiliary.o: auxiliary.c
	${CC} ${CFLAGS} -c auxiliary.c `sdl-config --cflags`

release: demo README.txt TODO
	tar -zcvf release.tar.gz demo README.txt TODO
