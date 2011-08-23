all: nollecfg mainwin.xml

nollecfg: maingui.c
	gcc -Wall -g -o $@ $^ `pkg-config --cflags --libs gtk+-2.0` -export-dynamic

mainwin.xml: mainwin.glade
	gtk-builder-convert $^ $@

clean:
	rm -f nollecfg mainwin.xml