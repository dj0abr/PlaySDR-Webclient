#include "sdrplay.h"
#include <fftw3.h>

void uFFT_init(int id, int capRate, int resolution);
void uFFT_exit(int id);
void uFFT_calc(int id, short *isamples, short *qsamples, int numSamples, int mode);
void scaleSamples(double *samples, int numSamples);
void init_ffts();

typedef struct {
    fftw_complex    *uFFT_din;      // FFT input data
    fftw_complex    *uFFT_dout;     // FFT output data
    fftw_plan       uFFT_plan;      // FFT plan
    double          fftData[SDR_SAMPLE_RATE]; // result, ready for waterfall
    int             uFFT_rate;      // number of samples to be procesed
    int             uFFT_resolution;// Hz per FFT value
    int             uFFT_idx;       // internal counter
} FFT_DATA;

enum _FFT_IDs_ {
    FFTID_BIG = 0,
};

extern FFT_DATA fftd[10];               // space for up to 10 FFTs
extern double fftData[SDR_SAMPLE_RATE];
