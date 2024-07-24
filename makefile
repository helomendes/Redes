# Programa
SERVER		= server
SERVER_OBJS = server.o
CLIENT		= client
CLIENT_OBJS = client.o

# Compilador
CC		= gcc

# Flags
CFLAGS 	= -Wall
LFLAGS = 
DEBUGFLAGS = -g -DDEBUG

# Lista de arquivos para distribuição
DISTFILES = *.c *.h LEIAME* Makefile
DISTDIR = `basename ${PWD}`

.PHONY: all clean purge dist debug

all: server

debug: CFLAGS += $(DEBUGFLAGS)
debug: $(PROG)

server: server.o
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

server.o: server.c
	@echo "Gerando server"
	$(CC) -c $(CFLAGS) -o $@ $<

client: client.o
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

client.o: client.c
	@echo "Gerando client"
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	@echo "Limpando sujeira ..."
	@rm -f *~ *.bak

purge:  clean
	@echo "Limpando tudo ..."
	@rm -f $(SERVER) $(CLIENT) $(SERVER_OBJS) $(CLIENT_OBJS) core a.out $(DISTDIR) $(DISTDIR).tar

dist: purge
	@echo "Gerando arquivo de distribuição ($(DISTDIR).tar) ..."
	@ln -s . $(DISTDIR)
	@tar -cvf $(DISTDIR).tar $(addprefix ./$(DISTDIR)/, $(DISTFILES))
	@rm -f $(DISTDIR)
