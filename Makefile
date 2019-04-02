CFLAGS?=-O3 -Wall -I./websocket
LDLIBS+= -lpthread -lm -lmirsdrapi-rsp -lfftw3 -lsndfile -lasound -lgd -lz -ljpeg -lfreetype -lrtlsdr
CC?=gcc
PROGNAME=playSDRweb
OBJ=playSDRweb.o sdrplay.o sampleprocessing.o fir_table_calc.o waterfall.o fft.o wf_univ.o color.o websocket/websocketserver.o websocket/ws_callbacks.o websocket/base64.o websocket/sha1.o websocket/ws.o websocket/handshake.o ssb.o hilbert90.o downmixer.o audio.o setqrg.o rtlsdr.o timing.o fifo.o

all: playSDRweb

websocket/%.o: websocket/%c
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

playSDRweb: $(OBJ)
	$(CC) -g -o $@ $^ $(LDFLAGS) $(LDLIBS)

clean:
	rm -f *.o websocket/*.o playSDRweb uFFT_wisdom*
