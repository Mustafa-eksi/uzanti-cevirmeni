prefix = /usr
SHARE_DIR=$(DESTDIR)$(prefix)/share/uzanti-cevirmeni
LOCALE_DIR=$(DESTDIR)$(prefix)/share/locale

main: src/*
	cargo build --release
install:
	install -D -t $(DESTDIR)$(prefix)/bin ./target/release/uzanti-cevirmeni 
	install -D -t $(SHARE_DIR) ui.ui
	install -d $(DESTDIR)$(prefix)/share/applications/
	install -D -t $(DESTDIR)$(prefix)/share/applications/me.mustafaeksi.uzanti-cevirmeni.desktop Uzanti-Cevirmeni.desktop
	# TODO: Automate translations
	install -d $(LOCALE_DIR)/tr/LC_MESSAGES
	install translations/tr.mo $(LOCALE_DIR)/tr/LC_MESSAGES/uzanti-cevirmeni.mo
