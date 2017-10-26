# change application name here (executable output name)

CC := gcc 
PROGRAM = raw2fits

CFLAGS := -Wall -g -ggdb -pipe

GTKLIB=`pkg-config --cflags --libs gtk+-3.0`

LDFLAG := $(GTKLIB) -L/usr/lib -lraw -lm -lcfitsio -export-dynamic -pthread

SRC := src/main.c src/converter.c src/list.c src/file_utils.c src/thread_pool.c src/raw2fits.c

all: $(PROGRAM)

$(PROGRAM): $(OBJECTS)
	$(CC) $(CFLAGS) $(SRC) $(LDFLAG) -o $(PROGRAM)

clean:
	rm -fr $(PROGRAM) $(PROGRAM).o

