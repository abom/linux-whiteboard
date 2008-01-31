# Temporary solution until it is changed to autoconf
CC = g++
CFLAGS += -Wall -Wextra -O0 -g -ggdb -pipe

demo: main.o wii.o auxiliary.o gui.o events.o wiicursor.o
	${CC} ${CFLAGS}	main.o wii.o auxiliary.o gui.o events.o wiicursor.o -o demo \
		-lcwiid `pkg-config --libs xtst` `sdl-config --libs` -lpthread -lSGE

clean:
	rm -vf demo release.tar.gz *.o *-tests

main.o: main.cpp matrix.h
	${CC} ${CFLAGS} -c main.cpp

wii.o: wii.cpp
	${CC} ${CFLAGS} -c wii.cpp

auxiliary.o: auxiliary.cpp matrix.h
	${CC} ${CFLAGS} -c auxiliary.cpp

gui.o: gui.cpp
	${CC} ${CFLAGS} -c gui.cpp `sdl-config --cflags`

events.o: events.cpp
	${CC} ${CFLAGS} -c events.cpp

wiicursor.o: wiicursor.cpp matrix.h
	${CC} ${CFLAGS} -c wiicursor.cpp

release: demo README.txt TODO
	tar -zcvf release.tar.gz demo README.txt
