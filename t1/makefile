# Programa
SERVER		= server
SERVER_OBJS = server.o packet.o socket.o video.o
CLIENT		= client
CLIENT_OBJS = client.o packet.o socket.o video.o

# Compilador
CC		= gcc

# Flags
CFLAGS 	= -Wall
LFLAGS = 
DEBUGFLAGS = -g -DDEBUG
LOOPBACK = -DLOOPBACK

# Lista de arquivos para distribuição
DISTFILES = *.c *.h LEIAME* Makefile
DISTDIR = `basename ${PWD}`

.PHONY: all clean purge dist debug

all: server client

debug: CFLAGS += $(DEBUGFLAGS)
debug: server client

loopback: CFLAGS += $(LOOPBACK)
loopback: server client

server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

server.o: server.c
	@echo "Gerando server"
	$(CC) -c $(CFLAGS) -o $@ $<

client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

client.o: client.c
	@echo "Gerando client"
	$(CC) -c $(CFLAGS) -o $@ $<

packet.o: packet.h packet.c
	$(CC) $(CFLAGS) -o $@ -c packet.c

socket.o: socket.h socket.c
	$(CC) $(CFLAGS) -o $@ -c socket.c

video.o: video.h video.c
	$(CC) $(CFLAGS) -o $@ -c video.c

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
