all: main generate_pot update_pos generate_mos
CFLAGS=`pkg-config --cflags gtk4` -Wall -ggdb -std=c++20 -fno-omit-frame-pointer
LIBS=`pkg-config --libs gtk4`
prefix=/usr
LOCALE_DIR=$(DESTDIR)$(prefix)/share/locale

main: src/*
	g++ $(CFLAGS) src/main.cpp -o uzanti-cevirmeni $(LIBS)

install:
	install uzanti-cevirmeni $(DESTDIR)$(prefix)/bin
	install -d $(LOCALE_DIR)/tr/LC_MESSAGES
	install translations/tr.mo $(LOCALE_DIR)/tr/LC_MESSAGES/uzanti-cevirmeni.mo

uninstall:
	rm $(DESTDIR)$(prefix)/bin/uzanti-cevirmeni
	rm $(LOCALE_DIR)/tr/LC_MESSAGES/uzanti-cevirmeni.mo

generate_pot: src/*
	xgettext --from-code=UTF-8 --add-comments --keyword=_ --keyword=C_:1c,2 -c++ ./src/main.cpp ./src/backends/* -o ./translations/uzanti-cevirmeni.pot

update_pos: translations/uzanti-cevirmeni.pot
	msgmerge -o translations/tr.po translations/tr.po translations/uzanti-cevirmeni.pot

generate_mos: translations/*.po
	msgfmt translations/tr.po -o translations/tr.mo
