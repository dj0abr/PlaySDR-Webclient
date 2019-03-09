/*
* Web based SDR Client for SDRplay
* =============================================================
* Author: DJ0ABR
*
*   (c) DJ0ABR
*   www.dj0abr.de
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software
*   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
* 
* fft.c ... FFT preprocessing for an SDR waterfall and spectrum display
* 
* Usage:
* up to 10 different FFTs can be defined
* 1) call uFFT_init at program start to define the FFT
* 2) use uFFT_calc to do the FFT job
*   optional: use scaleSamples to scale the FFT output with the antenna input
* 3) call uFFT_exit to free the ressources at program end
* 
* Functions:
* void uFFT_init(int capRate, int resolution) ... Initialize the fft
* void uFFT_exit() ... free fft ressources
* void uFFT_calc(short *samples, int numSamples, int channels, int base_frequency) ...
*                      convert samples into fft data
* void scaleSamples(double *samples, int numSamples) ...
*                      scale fft output to match antenna dBm
* 
* uses the fftw's r2r_1d format, which is double-in, double-out, 1-dimensional
* 
*/

#include <fftw3.h>
#include <math.h>
#include <string.h>
#include "fft.h"
#include "sampleprocessing.h"

// init all FFTs used in this software
void init_ffts()
{
    uFFT_init(FFTID_BIG, SAMPLERATE_FIRST, FFT_RESOLUTION);
}

// this structure can store the definitions of up to 10 FFTs
FFT_DATA fftd[10];   // max id is 10-1

/*
 * init the FFT
 * must be called once before the first fft calculation (i.e. at program start)
 * id ... ID of this FFT, any number between 0 and 9
 * capRate ... capture rate of the sample source (soundcard or SDR)
 * resolution ... resolution of the resulting FFT data in Hz
 * */
void uFFT_init(int id, int capRate, int resolution)
{
char fn[256];

    sprintf(fn,"uFFT_wisdom_%d_%d:%d",capRate,resolution,id);
    
    fftd[id].uFFT_idx = 0;

    fftd[id].uFFT_resolution = resolution;
    fftd[id].uFFT_rate = capRate / resolution;
    
	if (fftw_import_wisdom_from_filename(fn) == 0)
    {
		//printf("creating fft wisdom file");
    }
    
    // allocate memory for the in abd oput data
    fftd[id].uFFT_din = (fftw_complex *) fftw_malloc(sizeof(fftw_complex ) * fftd[id].uFFT_rate);
    fftd[id].uFFT_dout = (fftw_complex *) fftw_malloc(sizeof(fftw_complex ) * fftd[id].uFFT_rate);
    
    // make the FFT plan
    fftd[id].uFFT_plan = fftw_plan_dft_1d(fftd[id].uFFT_rate, fftd[id].uFFT_din, fftd[id].uFFT_dout, FFTW_FORWARD, FFTW_MEASURE);
    
    // store the plan for the next time, so next start will be much faster
    if (fftw_export_wisdom_to_filename(fn) == 0)
    {
		//printf("export fft wisdom error");
    }
}

/*
 * exit the FFT
 * must be called on program exit to free ressouces
 * */
void uFFT_exit(int id)
{
    if(fftd[id].uFFT_plan) fftw_destroy_plan(fftd[id].uFFT_plan);
	if(fftd[id].uFFT_din) fftw_free(fftd[id].uFFT_din);
	if(fftd[id].uFFT_dout) fftw_free(fftd[id].uFFT_dout);
}

/*
 * FFT calculation
 * id ... ID of this FFT, any number between 0 and 9, the same as used for uFFT_init
 * samples: array of 16 bit samples (i and q channel)
 * numSamples: number of samples
 * channels: 1 or 2, if 2 (stereo) we just use the first channel (samples are alternating between Ch1 and Ch2)
 * base_frequency: the frequency in Hz (tuned frequency, lowest frequency)
 * mode: 0 ... positive spectrum 0..samplerate/2
 *       1 ... both sides 0..samplerate/2 and -samplerate/2..0
 * 
 * */
extern char errtxt[1000000];

void uFFT_calc(int id, short *isamples, short *qsamples, int numSamples, int mode, int wf_width)
{
    // go through all delivered samples
    for (int i = 0; i < numSamples; i++)
    {
        // copy the samples into the FFT input array
        fftd[id].uFFT_din[fftd[id].uFFT_idx][0] = isamples[i];
        fftd[id].uFFT_din[fftd[id].uFFT_idx][1] = qsamples[i];        
        
        // wait until we have enough samples
        if(++fftd[id].uFFT_idx >= fftd[id].uFFT_rate)
        {
            fftd[id].uFFT_idx = 0;
            
            // the FFT input array is filled, we can do the fft
            fftw_execute(fftd[id].uFFT_plan);
            
            // the FFT generates:
            // 0..rate/2 :     Spectrum from 0 to samplerate/2
            // rate/2..rate:   Spectrum from -samplerate/2 to 0 in reverse order
            
            // number of FFT result values
            int ret = fftd[id].uFFT_rate / 2;
            
            // calculate the absolute value of the fft result
            double real,imag;
            if(mode == 0)
            {
                for (int i = 0; i < ret; i++)
                {
                    // calculate absolute value
                    real = fftd[id].uFFT_dout[i][0];
                    imag = fftd[id].uFFT_dout[i][1];
                    fftd[id].fftData[i] = sqrt((real * real) + (imag * imag));
                }
            }
            else
            {
                int dstidx = 0;
                // we need width pixels for the waterfall
                int pixels = wf_width;

                for (int i = (fftd[id].uFFT_rate - pixels/2); i < fftd[id].uFFT_rate; i++)
                {
                    // calculate absolute value
                    real = fftd[id].uFFT_dout[i][0];
                    imag = fftd[id].uFFT_dout[i][1];
                    fftd[id].fftData[dstidx++] = sqrt((real * real) + (imag * imag));
                }

                for (int i = 0; i < (pixels/2); i++)
                {
                    // calculate absolute value
                    real = fftd[id].uFFT_dout[i][0];
                    imag = fftd[id].uFFT_dout[i][1];
                    fftd[id].fftData[dstidx++] = sqrt((real * real) + (imag * imag));
                }
            }
        }
    }
}

/*
 * scale FTT values
 * 
 * a change of 6 dBm at the antenna input must result in *2 or /2 of
 * the spectrum needle.
 * To match the FFT output with this requirement we must
 * use a log(base 1,122). I don't know why this works, but I have
 * carefully tested and simulated this formula.
 * 
 * Overwrites "samples" !
 * */

// calibration value to match the spectrum display to the antenna input
double refminus80dBm = 0;

void scaleSamples(double *samples, int numSamples)
{
    double log1122 = log(1.122);
    double maxval = (double)(log(32768.0*(double)numSamples) / log1122);
    
    for(int i=0; i<numSamples; i++)
    {
        double dval = samples[i];
        
        // a log of 0 is not possible, so make it to 1
        if (dval < 1) dval = 1;    
        
        // this formula is a log to the base of 1.122
        dval = (double)(log(dval) / log1122);
        
        /*
         * lowest value:
         *  the lowest FFT output value may be 0 (corrected to 1 above), 
         *  the result of the log is 1
         * highers value:
         *  the max. value of the FFT output can be 32768*(uFFT_rate/2)
         * 
         * Example: uFFT_rate = 48k, maxVal=786.432.000
         * the above log results in: 178
         * therefore: the maximum dynamic range in this case is 178 dBm
         * */
        
        // turn into a negative dBm value
        dval = dval - maxval;
        
        // at this point "dval" matches the dBm value at the antenna input
        // but we need to add a correction value which has to be calibrated
        // at -80dBm. 
        // !!! all this works ONLY if the receiver AGC is switched OFF !!!
        dval += refminus80dBm;

        samples[i] = dval;
    }
}
