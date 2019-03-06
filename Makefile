CFLAGS?=-O3 -Wall
LDLIBS+= -lpthread -lm -lmirsdrapi-rsp -lfftw3 -lsndfile -lasound -lgd -lz -ljpeg -lfreetype
CC?=gcc
PROGNAME=playSDRweb
OBJ=soundcard.o playSDRweb.o sdrplay.o

all: playSDRweb

%.o: %.c
	$(CC) $(CFLAGS) -c $<

playSDRweb: $(OBJ)
	$(CC) -g -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f *.o playSDRweb
