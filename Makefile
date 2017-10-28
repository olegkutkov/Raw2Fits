

CC := gcc 
PROGRAM = raw2fits

CFLAGS := -Wall -g -ggdb -pipe -I./include

GTKLIB=`pkg-config --cflags --libs gtk+-3.0`

LIBRAW := -L/usr/lib -lraw

LDFLAG := $(GTKLIB) $(LIBRAW) -lm -lcfitsio -export-dynamic -pthread

SRC := src/main.c src/converter.c src/list.c src/file_utils.c src/thread_pool.c src/raw2fits.c

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAG) -o $(PROGRAM)

install:
	mkdir -p /usr/share/raw2fits/
	cp -f glade/ui.glade /usr/share/raw2fits/
	cp -f desktop/raw2fits.desktop /usr/share/applications/
	cp -f glade/raw2fits_48x48.png /usr/share/raw2fits/
	cp -f glade/raw2fits_128x128.png /usr/share/raw2fits/
	cp -f glade/raw2fits_48x48.png /usr/share/pixmaps/raw2fits.png
	cp -f raw2fits /usr/bin

uninstall:
	rm -f /usr/bin/raw2fits
	rm -f /usr/share/pixmaps/raw2fits.png
	rm -f /usr/share/applications/raw2fits.desktop
	rm -f /usr/share/raw2fits/raw2fits_48x48.png
	rm -f /usr/share/raw2fits/raw2fits_128x128.png
	rm -f /usr/share/raw2fits/ui.glade
	rm -fr /usr/share/raw2fits/

clean:
	rm -fr $(PROGRAM) $(PROGRAM).o

