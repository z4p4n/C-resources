CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=dump_iloveradio.c ../../ZpnError.c ../../ZpnHTTP.c ../../ZpnSocket.c ../../ZpnRegex.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=dump_iloveradio

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	rm *.o ../../*.o

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -R *.o dump_iloveradio advertising
