mapeditclass.o: mapeditclass.c
	gcc -Wall -O2 -g -c `gtk-config --cflags` mapeditclass.c

testmapedit.o: testmapedit.c
	gcc -Wall -O2 -g -c `gtk-config --cflags` testmapedit.c

testmapedit: testmapedit.o
	gcc -Wall -O2 -g `gtk-config --libs` testmapedit.o mapeditclass.o
