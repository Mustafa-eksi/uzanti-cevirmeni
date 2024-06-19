all: main
CFLAGS=`pkg-config --cflags gtk4` -Wall -ggdb -std=c++20 -fno-omit-frame-pointer
LIBS=`pkg-config --libs gtk4`
prefix=/usr

main: src/*
	g++ $(CFLAGS) src/main.cpp -o uzanti-cevirmeni $(LIBS)
install:
	install uzanti-cevirmeni $(DESTDIR)$(prefix)/bin
