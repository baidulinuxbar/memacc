libtl07.so:tl07.o
	gcc -shared -o libtl07.so tl07.o
tl07.o:tl07.c
	gcc -c -fpic -o tl07.o tl07.c `pkg-config --cflags --libs gtk+-2.0` -I/workarea/cprogram/include/
install:
	cp libtl07.so /workarea/cprogram/mlib/
clean:
	rm *.o *.so

