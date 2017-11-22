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

PREFIX ?= /usr/
BIN_PREFIX ?= $(PREFIX)/bin
SHARE_PREFIX ?= $(PREFIX)/share

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
	mkdir -p $(SHARE_PREFIX)/raw2fits/
	mkdir -p $(SHARE_PREFIX)/applications/
	mkdir -p $(SHARE_PREFIX)/pixmaps/
	cp -f glade/ui.glade $(SHARE_PREFIX)/raw2fits/
	cp -f desktop/raw2fits.desktop $(SHARE_PREFIX)/applications/
	cp -f glade/raw2fits_48x48.png $(SHARE_PREFIX)/raw2fits/
	cp -f glade/raw2fits_128x128.png $(SHARE_PREFIX)/raw2fits/
	cp -f glade/raw2fits_48x48.png $(SHARE_PREFIX)/pixmaps/raw2fits.png
	cp -f raw2fits $(BIN_PREFIX)

uninstall:
	rm -f $(BIN_PREFIX)/raw2fits
	rm -f $(SHARE_PREFIX)/pixmaps/raw2fits.png
	rm -f $(SHARE_PREFIX)/applications/raw2fits.desktop
	rm -f $(SHARE_PREFIX)/raw2fits/raw2fits_48x48.png
	rm -f $(SHARE_PREFIX)/raw2fits/raw2fits_128x128.png
	rm -f $(SHARE_PREFIX)/raw2fits/ui.glade
	rm -fr $(SHARE_PREFIX)/raw2fits/

install-cli:
	cp -f raw2fits-cli $(BIN_PREFIX)

uninstall-cli:
	rm -f $(BIN_PREFIX)/raw2fits-cli

clean:
	rm -f $(PROGRAM) $(PROGRAM_CLI) $(OBJ_COMMON) $(OBJ_GUI) $(OBJ_CLI)

