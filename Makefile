CFLAGS=`pkg-config --cflags gtk4`
LIBS=`pkg-config --libs gtk4`

main: main.c
	gcc $(CFLAGS) main.c -o main $(LIBS)