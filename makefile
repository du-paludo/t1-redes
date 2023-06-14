FLAGS = -Wall -std=c99
OBJS = ConexaoRawSocket.o packet.o

# .PHONY: all debug clean purge

# %.o: %.c %.h
# 	$(CC) $(CFLAGS) -c $< -o $@

# $(PROG): % : $(OBJS) %.o
# 	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

all: client server

client: client.o $(OBJS)
	gcc -o client client.o $(OBJS) $(FLAGS)

server: server.o $(OBJS)
	gcc -o server server.o $(OBJS) $(FLAGS)

ConexaoRawSocket.o: ConexaoRawSocket.c
	gcc -c ConexaoRawSocket.c $(FLAGS)

packet.o: packet.c
	gcc -c packet.c $(FLAGS)

client.o: client.c
	gcc -c client.c $(FLAGS)

server.o: server.c
	gcc -c server.c $(FLAGS)

clean:
	@rm -f *~ *.bak *.tmp

purge: clean
	@rm -f  $(PROG) $(PROG_AUX) *.o $(OBJS) core a.out
	@rm -f *.png marker.out *.log


