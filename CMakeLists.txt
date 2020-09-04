#
# CMakeLists.txt
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

CMAKE_MINIMUM_REQUIRED (VERSION 3.2)

PROJECT (raw2fits C)

INCLUDE (CheckFunctionExists)
FIND_PACKAGE (Threads)

# Use the package PkgConfig to detect GTK+ headers/library files
FIND_PACKAGE(PkgConfig REQUIRED)
PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)

# Setup CMake to use GTK+, tell the compiler where to look for headers
# and to the linker where to look for libraries
INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})

# Add other flags to the compiler
ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})

INCLUDE_DIRECTORIES (./include)

SET (SOURCES src/converter.c src/list.c src/file_utils.c src/thread_pool.c 
			src/raw2fits.c src/coords_calc.c src/main.c)

ADD_EXECUTABLE (raw2fits ${SOURCES})

TARGET_LINK_LIBRARIES (raw2fits ${GTK3_LIBRARIES})
TARGET_LINK_LIBRARIES (raw2fits m)
TARGET_LINK_LIBRARIES (raw2fits raw)
TARGET_LINK_LIBRARIES (raw2fits cfitsio)
TARGET_LINK_LIBRARIES (raw2fits ${CMAKE_THREAD_LIBS_INIT})

ADD_CUSTOM_COMMAND (TARGET raw2fits PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
					${CMAKE_SOURCE_DIR}/glade $<TARGET_FILE_DIR:raw2fits>/glade)

INSTALL (TARGETS raw2fits RUNTIME DESTINATION bin)


