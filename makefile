PROG = main
CC = gcc
CFLAGS = -Wall -std=c99
OBJS = main.o

.PHONY: all debug clean purge

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

$(PROG): % : $(OBJS) %.o
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

clean:
	@rm -f *~ *.bak *.tmp

purge: clean
	@rm -f  $(PROG) $(PROG_AUX) *.o $(OBJS) core a.out
	@rm -f *.png marker.out *.log