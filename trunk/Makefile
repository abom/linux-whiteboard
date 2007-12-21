
demo: main.o wii.o
	gcc -g main.o wii.o -o demo -lcwiid -lmeschach -lXtst \
		`sdl-config --cflags` `sdl-config --libs` -I/usr/include/SDL/

clean:
	rm -f demo *.o

main.o: main.c
	gcc main.c -g -c -I/usr/include/SDL/

wii.o: wii.c
	gcc wii.c -c -g


