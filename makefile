#Makefile do EP2 - Crivo de Eratostenes 
#Douglas Rodrigue de Almeida
#
#

SERIALNAME=serial
PARALELNAME=paralel
CC=gcc

CFLAGS=-fopenmp -std=c99 -c -O3 -Wall -Wextra -Wpedantic $(DEFS) -Iinclude/
LIBS=-lm -lpthread -fopenmp
DBGFLAGS=-ggdb -fno-inline -fno-omit-frame-pointer
BINDIR=bin
OBJDIR=obj
SOBJFILES=utils.o rselect.o serial.o main.o
POBJFILES=paralel.o main.o
SOBJECTS=$(addprefix $(OBJDIR)/, $(SOBJFILES))
POBJECTS=$(addprefix $(OBJDIR)/, $(POBJFILES))
SOURCEDIR=src
ARGS=25 3 all 2
ARGS1=30 4 all 3

$(OBJDIR)/%.o: $(SOURCEDIR)/%.c
	@echo 
	@echo Compilando $<...
	$(CC) $(DBGFLAGS) $(CFLAGS) $< -o $@

$(PARALELNAME): $(POBJECTS) 
	@echo 
	@echo Gerando executavel...
	$(CC) $(PRLFLAGS) $(LDFLAGS) -o $(BINDIR)/$@ $^ $(LIBS)
	
$(SERIALNAME): $(SOBJECTS) 
	@echo 
	@echo Gerando executavel...
	$(CC) -o $(BINDIR)/$@ $^ $(LIBS)

.PHONY: all build clean debug memcheck run serial paralel

all:
	$(SERIALNAME)

build:
	@echo 
	@echo Gerando arquivo compactado...
	tar zcvf ../$(MATRICULA).tar.gz doc/documentacao.pdf src/* include/* --exclude-vcs

clean:
	@echo 
	@echo Excluindo executavel...
	rm -f $(BINDIR)/$(PARALELNAME)
	rm -f $(BINDIR)/$(SERIALNAME)
	@echo Excluindo objetos...
	rm -f $(POBJECTS)
	rm -f $(SOBJECTS)
	
debug:
	gdb --args ./$(BINDIR)/$(SERIALNAME) $(ARGS)

memcheck:
	valgrind -v --leak-check=yes --track-origins=yes $(BINDIR)/$(PROJECTNAME) $(ARGS)

run-paralel:
	./$(BINDIR)/$(PARALELNAME) $(ARGS)

run-serial:
	./$(BINDIR)/$(SERIALNAME) $(ARGS)

show-paralel:
	./$(BINDIR)/$(PARALELNAME) $(ARGS1)

show-serial:
	./$(BINDIR)/$(SERIALNAME) $(ARGS1)

