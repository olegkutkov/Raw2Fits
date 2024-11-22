#
# Makefile
#
#   Copyright 2017  Oleg Kutkov <contact@olegkutkov.me>
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

PROGRAM = raw2fits
PROGRAM_CLI = raw2fits-cli

prefix ?= /usr
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)/bin
datarootdir ?= $(prefix)/share
datadir ?= $(datarootdir)

INSTALL = install
INSTALL_DATA = ${INSTALL} -m 644
INSTALL_PROGRAM = $(INSTALL)

DEBUG := -g -ggdb

VPATH := src/

LIBS_COMMON := libraw \
			cfitsio

LIBS_GUI := gtk+-3.0 \
		$(LIBS_COMMON)

LIBS_CLI := libconfig \
		$(LIBS_COMMON)

CFLAGS += -Wall -pipe -I./include $(DEBUG)
CFLAGS_GUI := $(shell pkg-config --cflags $(LIBS_GUI)) $(CFLAGS)
CFLAGS_CLI := $(shell pkg-config --cflags $(LIBS_CLI)) $(CFLAGS)

LDFLAGS_COMMON += -lm -lpthread -export-dynamic
LDFLAGS_GUI += $(shell pkg-config --libs $(LIBS_GUI)) $(LDFLAGS_COMMON)
LDFLAGS_CLI += $(shell pkg-config --libs $(LIBS_CLI)) $(LDFLAGS_COMMON)

SRC_COMMON := src/converter.c src/list.c src/file_utils.c \
				src/thread_pool.c src/raw2fits.c src/coords_calc.c

SRC_UI := src/main.c
SRC_CLI := src/main_cli.c src/config_loader.c


all:
	$(CC) $(CFLAGS_GUI) $(SRC_COMMON) $(SRC_UI) $(LDFLAGS_GUI) -o $(PROGRAM)

cli:
	$(CC) $(CFLAGS_CLI) $(SRC_COMMON) $(SRC_CLI) $(LDFLAGS_CLI) -o $(PROGRAM_CLI)

install:
	$(INSTALL_DATA) -D desktop/raw2fits.desktop $(DESTDIR)$(datadir)/applications/raw2fits.desktop
	$(INSTALL_DATA) -D glade/raw2fits_128x128.png $(DESTDIR)$(datadir)/raw2fits/aw2fits_128x128.png
	$(INSTALL_DATA) -D glade/raw2fits_48x48.png $(DESTDIR)$(datadir)/raw2fits/raw2fits_48x48.png
	$(INSTALL_DATA) -D glade/ui.glade $(DESTDIR)$(datadir)/raw2fits/ui.glade
	$(INSTALL_DATA) -D glade/raw2fits_48x48.png $(DESTDIR)$(datadir)/pixmaps/raw2fits.png
	$(INSTALL_PROGRAM) -D raw2fits $(DESTDIR)$(bindir)/raw2fits

uninstall:
	rm -f $(DESTDIR)$(bindir)/raw2fits
	rm -f $(DESTDIR)$(datadir)/pixmaps/raw2fits.png
	rm -f $(DESTDIR)$(datadir)/applications/raw2fits.desktop
	rm -f $(DESTDIR)$(datadir)/raw2fits/raw2fits_48x48.png
	rm -f $(DESTDIR)$(datadir)/raw2fits/raw2fits_128x128.png
	rm -f $(DESTDIR)$(datadir)/raw2fits/ui.glade
	rm -fr $(DESTDIR)$(datadir)/raw2fits/

install-cli:
	cp -f raw2fits-cli $(DESTDIR)$(bindir)/raw2fits-cli

uninstall-cli:
	rm -f $(DESTDIR)$(bindir)/raw2fits-cli

clean:
	rm -f $(PROGRAM) $(PROGRAM_CLI) $(OBJ_COMMON) $(OBJ_GUI) $(OBJ_CLI)

