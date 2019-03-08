CFLAGS?=-O3 -Wall
LDLIBS+= -lpthread -lm -lmirsdrapi-rsp -lfftw3 -lsndfile -lasound -lgd -lz -ljpeg -lfreetype
CC?=gcc
PROGNAME=playSDRweb
OBJ=playSDRweb.o soundcard.o sdrplay.o sampleprocessing.o fir_table_calc.o waterfall.o fft.o wf_univ.o color.o

all: playSDRweb

%.o: %.c
	$(CC) $(CFLAGS) -c $<

playSDRweb: $(OBJ)
	$(CC) -g -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f *.o playSDRweb
