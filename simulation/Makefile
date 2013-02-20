CC= g++
CFLAGS= -g  -DDEBUG 
LDFLAGS= -g -DDEBUG 
SOURCES=  dataPlane.cc testHeavyHitters.cc taskHeavyHitters.cc taskSuperspreaders.cc sketchCountMin.cc  sketchBitmap.cc os_mangler.cc os_galoisField.cc common.cc 

OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=run

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cc.o:
	$(CC)  $(CFLAGS)  -c $<

clean:
	rm -f *.o
	rm -f $(EXECUTABLE)
