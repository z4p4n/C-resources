CC=gcc
CFLAGS=-c -Wall -g
LDFLAGS=
SOURCES=x86ControlFlowGraph.c graph.c ../../ZpnFiles.c ../../ZpnError.c ../../ZpnRegex.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=x86CFG_builder

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	rm *.o ../../*.o

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -R *.o x86CFG_builder
