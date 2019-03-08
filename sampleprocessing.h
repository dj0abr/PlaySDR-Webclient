#define SAMPLERATE_480k    480000
#define DECIMATERATE        (SDR_SAMPLE_RATE / SAMPLERATE_480k) // 2.4M / 480k = 5 ... decimation factor
#define FFT_RESOLUTION  200                                 // 200 Hz per FFT value (=WF pixel) and a WF width of 
                                                            // 1000 pixel gives a range of 200kHz for the first waterfall
#define SAMPLES_FOR_FFT (SAMPLERATE_480k / FFT_RESOLUTION)  // see comment below

void sample_processing(short *xi, short *xq, int numSamples);
void init_hs_filters();
