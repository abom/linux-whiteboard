
demo: main.o wii.o matrix.o
	gcc -g main.o wii.o matrix.o -o demo -lcwiid -lXtst \
		`sdl-config --cflags` `sdl-config --libs` -I/usr/include/SDL/

clean:
	rm -f demo release.tar.gz *.o

main.o: main.c matrix.h
	gcc main.c -g -c -I/usr/include/SDL/

wii.o: wii.c
	gcc wii.c -c -g

matrix.o: matrix.c matrix.h
	gcc matrix.c -c -g

release: demo README.txt
	tar zcvf release.tar.gz demo README.txt
