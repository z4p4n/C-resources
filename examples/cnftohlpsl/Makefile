CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=cnfToHlpsl.c ../../ZpnError.c ../../ZpnRegex.c ../../ZpnString.c ../../ZpnFiles.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=cnf_to_hlpsl

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	rm *.o ../../*.o

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -R *.o cnf_to_hlpsl
