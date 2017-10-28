#
# Makefile
#
#   Copyright 2017  Oleg Kutkov <elenbert@gmail.com>
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.  
#

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

