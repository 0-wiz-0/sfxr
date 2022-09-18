PREFIX?=/usr
CFLAGS=-ggdb
CXXFLAGS+=$(CFLAGS) `sdl2-config --cflags` `pkg-config gtk4 --cflags` -DPREFIX=\"$(PREFIX)\"
LDFLAGS+=`sdl2-config --libs` `pkg-config gtk4 --libs` -lm

sfxr: main.cpp tools.h sdlkit.h
	$(CXX) main.cpp $(CXXFLAGS) $(LDFLAGS) -o $@

install: sfxr
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/share/sfxr
	mkdir -p $(DESTDIR)$(PREFIX)/share/applications
	mkdir -p $(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps
	install -m 755 sfxr $(DESTDIR)$(PREFIX)/bin
	install -m 644 -p *.tga *.bmp $(DESTDIR)$(PREFIX)/share/sfxr
	install -p -m 644 sfxr.png \
		$(DESTDIR)$(PREFIX)/share/icons/hicolor/48x48/apps
	desktop-file-install --vendor "" \
		--dir $(DESTDIR)$(PREFIX)/share/applications sfxr.desktop

clean:
	rm -f sfxr
