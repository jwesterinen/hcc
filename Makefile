TARGET = hcc

PREFIX ?= /usr/local
INST_BIN_DIR = $(PREFIX)/bin
INST_LIB_DIR = $(PREFIX)/lib

#DEFINES = -DYYDEBUG -DTRACE

CFLAGS = -O0 -g -Wall -c
LIBS = -ll

HEADERS = error.h symtab.h message.h gen.h gen_hack.h
OBJECTS = main.o error.o symtab.o message.o gen.o gen_hack.o lex.yy.o y.tab.o

all: $(TARGET)

$(TARGET): lex.yy.c y.tab.c y.tab.h $(OBJECTS)
	cc $(OBJECTS) $(LIBS) -o $@

$(OBJ)/%.o: $(SRC)/%.cpp $(HEADERS)
	cc $(DEFINES) $(CFLAGS) -o $@ $<

lex.yy.c: $(TARGET).l y.tab.h
	lex $<

y.tab.c y.tab.h: $(TARGET).y
	yacc -d $<

clean:
	rm -f $(TARGET) *.o lex.yy.c y.tab.c y.tab.h

install:
	/usr/bin/install -m 755 $(TARGET) $(INST_BIN_DIR)

uninstall:
	rm -f $(INST_BIN_DIR)/$(TARGET)

.PHONY: clean install uninstall
	
