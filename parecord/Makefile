CFLAGS = -Wall -g
PULSEFLAGS = -lpulse -lpulse-simple
SOURCES = parecord.c audio_types.c device.c
OBJECTS = ${SOURCES:.c=.o}
EXECUTABLE = parecord

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@gcc $(CFLAGS) $(PULSEFLAGS) -o $(EXECUTABLE) $(OBJECTS)

$(OBJECTS):
	@gcc -c $(CFLAGS) $(PULSEFLAGS) $(SOURCES)

clean:
	@rm -f $(OBJECTS) $(EXECUTABLE)