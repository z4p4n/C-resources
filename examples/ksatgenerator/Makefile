CC=gcc
CFLAGS=-c -Wall
LDFLAGS=-lm
SOURCES=ksat.c ../../ZpnError.c ../../ZpnHTTP.c ../../ZpnSocket.c ../../ZpnRegex.c ../../ZpnString.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=ksat

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	rm *.o ../../*.o

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -R *.o ksat gen
