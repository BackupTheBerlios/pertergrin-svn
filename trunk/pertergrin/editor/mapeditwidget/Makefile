all: testmapedit

mapeditclass.o: mapeditclass.c mapeditclass.h
	gcc -Wall -O2 -g -c `gtk-config --cflags` -o $@ $<

testmapedit.o: testmapedit.c mapeditclass.h
	gcc -Wall -O2 -g -c `gtk-config --cflags` -o $@ $<

testmapedit: testmapedit.o mapeditclass.o
	gcc -Wall -O2 -g `gdk-pixbuf-config --libs` -lglade testmapedit.o mapeditclass.o -o $@
