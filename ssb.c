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
* 
* ssb.c ... 
* - mix the selected range into baseband
* - create a small waterfall for this band
* - demodulate ssb
* - play audio to soundcard
* 
*/

#include "playSDRweb.h"
#include "fft.h"
#include "audio.h"
#include "hilbert90.h"
#include "wf_univ.h"
#include "downmixer.h"
#include "setqrg.h"
#include "fir_table_calc.h"
#include "playSDRweb.h"
#include "fifo.h"

short isamples[DEFAULT_SSB_RATE];
short qsamples[DEFAULT_SSB_RATE];
double usbsamples[DEFAULT_SSB_RATE];
short b16samples[AUDIO_RATE];

// gets the samples with SAMPLERATE_FIRST Samplerate
// we make further processing with SSB_RATE, so decimate by SSB_DECIMATE
void process_samples(short *xi, short *xq, int numSamples)
{
static int decimate = 0;
static int idx = 0;
static int audiodecimate = 0;
static int audioidx = 0;

    for(int i=0; i<numSamples; i++)
    {
        downmixer_process(&xi[i], &xq[i]);
        
        if(++decimate >= SSB_DECIMATE)
        {
            decimate = 0;
            
            // here we have a rate of SSB_RATE
            // usually we would need an anti aliasing filter for SSB_RATE here
            // but the Hilbert Filter BandPassm45deg and BandPass45deg
            
            isamples[idx] = (xi[i]);
            qsamples[idx] = (xq[i]);
            
            // LSB=+ or USB=-
            if(ssbmode == 0)
                usbsamples[idx] = BandPassm45deg(isamples[idx]) + BandPass45deg(qsamples[idx]);
            else
                usbsamples[idx] = BandPassm45deg(isamples[idx]) - BandPass45deg(qsamples[idx]);

            // Audio band low pass
            // optional: usbsamples[idx] = audio_lowPass(usbsamples[idx]);
            float vol = 1;
            if(hwtype == 1) vol = 20;
            usbsamples[idx] *= vol;
            
            // draw small WF
            if(idx >= SAMPLES_FOR_FFT_SMALL)
            {
                // we got enough samples for the requested fft resolution
                // make the FFT for the waterfall
                uFFT_InputData(FFTID_SMALL, isamples, qsamples, idx);
                uFFT_calc(FFTID_SMALL, 1, WF_WIDTH);

                // draw the waterfall picture:
                double *pfdata = fftd[FFTID_SMALL].fftData; // FFT result
                //for(int i=0; i<100; i++) printf("%.1f ",pfdata[i]);
                int width = WF_WIDTH;                   // the width of the bitmap
                int height = WF_HEIGHT_SMALL;           // the height of the bitmap, if 1 then the WebSocket mode is used
                int fleft = 0;                          // frequency offset of the left margin of the waterfall (usually 0)
                int fright = FFT_RESOLUTION_SMALL * width;  // frequency offset of the right margin of the waterfall (used for the titles)
                int frequency = TUNED_FREQUENCY;        // tuned base frequency of the SDR receiver (used for the titles)


                char filename[256] = {0};
                #ifdef WF_MODE_FILE
                    // jpg-filename of the bitmap, it is a good idea to write it to a RAM disk (NEVER write to an SD card !!!)
                    snprintf(filename,sizeof(filename)-1,"/tmp/wfline_small.jpg"); 
                #endif
                    
                #ifdef WF_MODE_WEBSOCKET
                    height = 1;
                #endif
                
                static int wfdelay = 0;
                // increasing this number makes the waterfall slower and 
                // needs less CPU time
                // for maximum speed: 1
                if(++wfdelay >= 1)  
                {
                    wfdelay = 0;
                    drawWF(WFID_SMALL,pfdata, width, width, height, fleft, fright, FFT_RESOLUTION_SMALL, frequency+foffset-(fright-fleft)/2, filename);
                }

                // the stream here has SSB_RATE which is required for a nice waterfall
                // for Audio we need less, 8k samples is good enough
                // lets decimate it to 8000 samples/s
                
                // here we have SAMPLES_FOR_FFT_SMALL samples in usbsamples
                // with a rate of SSB_RATE
                // copy into the audio buffer and decimate by AUDIO_DECIMATE
                for(int i=0; i<SAMPLES_FOR_FFT_SMALL; i++)
                {
                    if(++audiodecimate >= AUDIO_DECIMATE)
                    {
                        audiodecimate = 0;
                        b16samples[audioidx++] = usbsamples[i];
                        
                        // we collect audiocollect samples and then put it into the fifo
                        // audiocollect must be an even number !
                        
                        // direct playpack through soundcard:
                        // audiocollect should be a low value, but high enough
                        // so we do not get a broken pipe in audio playback.
                        
                        // playback via WebSocket:
                        // one transmission must be done every AUDIO_RATE/audiocollect
                        // so if audiocollect == AUDIO_RATE we send one packet every second
                        
                        int audiocollect = 8000; 
                        if(audioidx >= audiocollect)
                        {
                            audioidx = 0;
                            // *2 because we send short as uint8
                            // send to soundcard
                            //write_pipe(FIFO_AUDIO, (unsigned char *)b16samples, audiocollect*2);
                            
                            // send to websocket
                            write_pipe(FIFO_AUDIOWEBSOCKET, (unsigned char *)b16samples, audiocollect*2);
                        }
                    }
                }
                idx = 0;
            }
            idx++;
        }
    }
}
