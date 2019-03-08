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
* sampleprocessing.c ... these functions process the raw 2.4MS/s data stream:
* 
* this software works with a 200kHz band for the first waterfall
* so we need data from 0..200kHz, which requires a minimum sample rate
* of 400k. Lets use 480k which is 1/5 of 2.4MS/s
* 
* - down scaling from 2.4M to 480k Samples/s
* - use an anti aliasing low pass filter for this down scaling
* - collect samples required for the FFT fft_resolution
* - then call the FFT process which also does the waterfall
* - give the 480k samples to the audio processing
* 
* ! init_hs_filters() must be called prior to using this function !
* 
*/
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "sampleprocessing.h"
#include "sdrplay.h"
#include "waterfall.h"
#include "fir_table_calc.h"

// the low pass filter is time critical !
// save some CPU time by inlining this code
#define FORCE_INLINE __attribute__((always_inline)) inline

FORCE_INLINE void hsi_fir_filter_increment(short sample);
FORCE_INLINE short hsi_fir_filter(short sample);
FORCE_INLINE void hsq_fir_filter_increment(short sample);
FORCE_INLINE short hsq_fir_filter(short sample);

// storage for the decimated 480k samples
short isamp[SAMPLERATE_480k];
short qsamp[SAMPLERATE_480k];

/*
 * this function is called by the SDR hardware's callback function
 * it gets samples with a rate of 2.4MS/s
 */
int cntr=0;
void sample_processing(short *xi, short *xq, int numSamples)
{
static int decimate = 0;
static int idx = 0;

    if(numSamples > samplesPerPacket)
    {
        printf("ignore :%d\n",numSamples);
        return;
    }

    /*
     * decimation is a very simple process. It just takes every DECIMATERATE sample
     * and ignores the rest
     * to remove aliasing the original 2.4M samples must go through a low pass
     * filter with a corner frequency of < (480/2)kHz
     */
    for(int i=0; i<numSamples; i++)
    {
        // take every DECIMATERATE sample
        if(++decimate >= DECIMATERATE)
        {
            decimate = 0;

            // feed the I and Q samples through the anti aliasing low pass filter
            // two separate filters for I and Q are required !
            isamp[idx] = hsi_fir_filter(xi[i]);
            qsamp[idx] = hsq_fir_filter(xq[i]);
            
            // here we have a rate of 480k Samples
            // wait until we have SAMPLES_FOR_FFT samples, then continue
            if(idx >= SAMPLES_FOR_FFT)
            {
                /*
                 * FFT: the resolution depends on the sample time
                 * i.e.: 
                 * if we have samples of 1 second, then the resolution is 1 Hz per FFT value
                 * if we have samples of 0.1 second, then the resolution is 10 Hz per FFT value
                 * so the resolution (Hz per FFT value) is 1/Sampletime
                 * 
                 * In this case: we want to show a spectrum of 200kHz and the
                 * waterfall has 1000 pixels. So we need a resolution of 200 Hz per pixel.
                 * We get this resultion if we record 2400 samples.
                 * 
                 * The samples are stored in isamp and qsamp,
                 * when SAMPLES_FOR_FFT samples are stored, we can do the fft and draw the waterfall 
                 * 
                 * we get here 2400 times per second (480k / 200)
                 * the waterfall process is started as often as possible
                 * on slow machines one process could go up to 100% CPU load
                 * to reduce this load increase wfdelay from 1 to any number as required, of course
                 * the waterfall will be slower
                 */
                static int wfdelay = 0;
                if(++wfdelay >= 1)
                {
                    wfdelay = 0;
                    draw_waterfall(isamp, qsamp, idx);
                }

                // decode and play the audio
                //process_samples(isamp, qsamp, idx);
                idx = 0;
            }
            idx++;
        }
        else
        {
            // the other samples are not used
            // BUT they MUST also go through the low pass filter
            // but we don't need the filter result, so we can use a simplified filter routine
            // without result calculation which saves a lot of CPU time
            hsi_fir_filter_increment(xi[i]);
            hsq_fir_filter_increment(xq[i]);
        }
    }
}


// =================================================================================
// FIR filters for 2M4 -> 480k decimation anti aliasing
// =================================================================================

#define FIR1_LEN    201

int hsi_wridx=0;                // write pointer to circular buffer
int hsq_wridx=0;
double hsi_buf[FIR1_LEN];       // circular buffer
double hsq_buf[FIR1_LEN];
double hs_ceoffs[FIR1_LEN];   // filter coefficients

void init_hs_filters()
{
    createLowPassFIRfilter(SDR_SAMPLE_RATE, 210000, hs_ceoffs, FIR1_LEN);
}

FORCE_INLINE void hsi_fir_filter_increment(short sample)
{
    hsi_buf[hsi_wridx++] = sample;
    hsi_wridx %= FIR1_LEN;
}

FORCE_INLINE short hsi_fir_filter(short sample)
{
    hsi_buf[hsi_wridx++] = sample;
    hsi_wridx %= FIR1_LEN;
    
    double y = 0;
    int idx = hsi_wridx;
    double *pcoeff = hs_ceoffs;
    for(int i = 0; i < FIR1_LEN; i++)
    {
        y += (*pcoeff++ * hsi_buf[idx++]);
        if(idx >= FIR1_LEN) idx=0;
    }

    return (short)y;
}

FORCE_INLINE void hsq_fir_filter_increment(short sample)
{
    hsq_buf[hsq_wridx++] = sample;
    hsq_wridx %= FIR1_LEN;
}

FORCE_INLINE short hsq_fir_filter(short sample)
{
    hsq_buf[hsq_wridx++] = sample;
    hsq_wridx %= FIR1_LEN;
    
    double y = 0;
    int idx = hsq_wridx;
    double *pcoeff = hs_ceoffs;
    for(int i = 0; i < FIR1_LEN; i++)
    {
        y += (*pcoeff++ * hsq_buf[idx++]);
        if(idx >= FIR1_LEN) idx=0;
    }

    return (short)y;
}
