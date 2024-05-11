.PHONY: all
all: ui main
PKGS=gtkmm-4.0
CFLAGS=-Wall -ggdb -std=c++20 `pkg-config --cflags $(PKGS)` -O2 -fno-omit-frame-pointer
LIBS=`pkg-config --libs $(PKGS)`

ui: ui.blp
	blueprint-compiler compile ui.blp --output uc.ui

main: src/*
	g++ src/main.cpp -o uzanti-cevirici $(CFLAGS) $(LIBS)
