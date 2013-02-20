CC= g++
CFLAGS= -g  -DDEBUG 
LDFLAGS= -g -DDEBUG 
SOURCES=  dataplane.cc testHeavyHitters.cc taskHeavyHitters.cc taskSuperSpreaders.cc sketchCountMin.cc  sketchBitmap.cc os_mangler.cc os_galoisField.cc common.cc nf2util.c util.c

OBJECTS=  dataplane.o testHeavyHitters.o taskHeavyHitters.o taskSuperSpreaders.o sketchCountMin.o  sketchBitmap.o os_mangler.o os_galoisField.o common.o nf2util.o util.o

# OBJECTS=$(SOURCES:.cc=.o)

EXECUTABLE=run

all: $(SOURCES) $(EXECUTABLE)

$(SOURCES):
	$(CC)  $(CFLAGS)  -c $(SOURCES)


$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

# .cc.o:
# 	$(CC)  $(CFLAGS)  -c $<

clean:
	rm -f *.o
	rm -f $(EXECUTABLE)
