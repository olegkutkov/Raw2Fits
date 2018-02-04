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

LIBS := gtk+-3.0 \
        libraw \
        libconfig \
        cfitsio

CFLAGS += -Wall -pipe -I./include $(DEBUG)
CFLAGS += $(shell pkg-config --cflags $(LIBS))

LDFLAGS += $(shell pkg-config --libs $(LIBS)) -lm -lpthread -export-dynamic

OBJ_COMMON := converter.o \
              list.o \
              file_utils.o \
              thread_pool.o \
              raw2fits.o \
              coords_calc.o

OBJ_GUI := main.o

OBJ_CLI := main_cli.o \
           config_loader.o

all: $(PROGRAM)

$(PROGRAM): $(OBJ_COMMON) $(OBJ_GUI)

cli: $(PROGRAM_CLI)

$(PROGRAM_CLI): $(OBJ_COMMON) $(OBJ_CLI)

install:
	$(INSTALL_DATA) -D desktop/raw2fits.desktop $(DESTDIR)$(datadir)/applications/raw2fits.desktop
	$(INSTALL_DATA) -D glade/raw2fits_128x128.png $(DESTDIR)$(datadir)/raw2fits/aw2fits_128x128.png
	$(INSTALL_DATA) -D glade/raw2fits_48x48.png $(DESTDIR)$(datadir)/raw2fits/raw2fits_48x48.png
	$(INSTALL_DATA) -D glade/ui.glade $(DESTDIR)$(datadir)/raw2fits/ui.glade
	$(INSTALL_DATA) -D glade/raw2fits_48x48.png $(DESTDIR)$(datadir)/pixmaps/raw2fits.png
	$(INSTALL_PROGRAM) -D raw2fits $(DESTDIR)$(bindir)

uninstall:
	rm -f $(DESTDIR)$(bindir)/raw2fits
	rm -f $(DESTDIR)$(datadir)/pixmaps/raw2fits.png
	rm -f $(DESTDIR)$(datadir)/applications/raw2fits.desktop
	rm -f $(DESTDIR)$(datadir)/raw2fits/raw2fits_48x48.png
	rm -f $(DESTDIR)$(datadir)/raw2fits/raw2fits_128x128.png
	rm -f $(DESTDIR)$(datadir)/raw2fits/ui.glade
	rm -fr $(DESTDIR)$(datadir)/raw2fits/

install-cli:
	cp -f raw2fits-cli $(DESTDIR)$(bindir)

uninstall-cli:
	rm -f $(DESTDIR)$(bindir)/raw2fits-cli

clean:
	rm -f $(PROGRAM) $(PROGRAM_CLI) $(OBJ_COMMON) $(OBJ_GUI) $(OBJ_CLI)

