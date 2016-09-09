CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=test_tor.c ../../ZpnError.c ../../ZpnHTTP.c ../../ZpnSocket.c ../../ZpnRegex.c ../../ZpnString.c ../../ZpnTor.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=torrify

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	rm *.o ../../*.o

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -R *.o $(EXECUTABLE)
