#include "playSDRweb.h"
#include <fftw3.h>

void uFFT_init(int id, int capRate, int resolution);
void uFFT_exit(int id);
void uFFT_calc(int id, int mode, int wf_width);
void scaleSamples(double *samples, int numSamples);
void init_ffts();
void uFFT_InputData(int id, short *isamples, short *qsamples, int numSamples);

typedef struct {
    fftw_complex    *uFFT_din;      // FFT input data
    fftw_complex    *uFFT_dout;     // FFT output data
    fftw_plan       uFFT_plan;      // FFT plan
    double          fftData[SDR_SAMPLE_RATE]; // result, ready for waterfall
    int             sampPerPass;    // number of samples to be procesed
} FFT_DATA;

enum _FFT_IDs_ {
    FFTID_BIG = 0,
    FFTID_SMALL,
    FFTID_MAX
};

extern FFT_DATA fftd[FFTID_MAX];               // space for up to 10 FFTs
extern double fftData[SDR_SAMPLE_RATE];
