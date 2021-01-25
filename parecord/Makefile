CFLAGS = -Wall -g
PULSEFLAGS = -lpulse -lpulse-simple

all:
	@gcc $(CFLAGS) $(PULSEFLAGS) parecord.c -o parecord

clean:
	@rm -f parecord