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
* waterfall.c ... convert samples to FFT values and draw waterfall
* 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>
#include <errno.h>
#include "fft.h"
#include "waterfall.h"
#include "wf_univ.h"
#include "sampleprocessing.h"
#include "playSDRweb.h"

/*
 * do the FFT and the drawing in a separate process
 * to keep the SDR hardware's callback function as fast as possible
 */
pthread_t wf_pid = 0;

typedef struct {
    short pi[SAMPLES_FOR_FFT];
    short pq[SAMPLES_FOR_FFT];
    int cnt;
} WFDATA;

void *wfproc(void *pdata);

void draw_waterfall(short *pi, short * pq, int cnt)
{
    if(wf_pid != 0)
    {
        // if the previous drawing is not finished, ignore the data
        // this makes the waterfall slower, but it is still ok
        // and will also work with slower CPUs
        //printf("ueberholt\n");
        return;
    }
    
    WFDATA wfdata;
    if(cnt > SAMPLES_FOR_FFT)
    {
        printf("sizeof pi and pq too small ! is %d, should be > %d\n",SAMPLES_FOR_FFT, cnt);
    }
    memcpy(wfdata.pi,pi,sizeof(short)*cnt);
    memcpy(wfdata.pq,pq,sizeof(short)*cnt);
    wfdata.cnt = cnt;
    
    int ret = pthread_create(&wf_pid,NULL,wfproc, &wfdata);
    if(ret)
    {
        printf("wf_drawline: proc NOT started\n\r");
    }
}

void *wfproc(void *pdata)
{
    // this thread must terminate itself because
    // the parent does not want to wait
    pthread_detach(pthread_self()); 
    
    // get the parameters
    WFDATA *pwf = (WFDATA *)pdata;
    
    // do the FFT
    // mode=0 ... the FTT result range is: base-frequency .. base-frequency + width*FFT_RESOLUTION (see sampleprocessing.c)
    uFFT_calc(FFTID_BIG, pwf->pi, pwf->pq, pwf->cnt, 0, 0);

    /* with this setting the FFT returns SAMPLES_FOR_FFT/2 values
    * the first value starts by 0 (=tuning frequency of the SDR receiver)
    * each value is +FFT_RESOLUTION Hz
    * we use the first WF_WIDTH values for the waterfall which is a range of WF_RANGE_HZ
    */
    
    // draw the waterfall picture:
    double *pfdata = fftd[FFTID_BIG].fftData; // FFT result
    int width = WF_WIDTH;                   // the width of the bitmap
    int height = WF_HEIGHT;                 // the height of the bitmap, if 1 then the WebSocket mode is used
    int fleft = 0;                          // frequency offset of the left margin of the waterfall (usually 0)
    int fright = FFT_RESOLUTION * width;    // frequency offset of the right margin of the waterfall (used for the titles)
    int frequency = TUNED_FREQUENCY;        // tuned base frequency of the SDR receiver (used for the titles)


    char filename[256] = {0};
    #ifdef WF_MODE_FILE
        // jpg-filename of the bitmap, it is a good idea to write it to a RAM disk (NEVER write to an SD card !!!)
        snprintf(filename,sizeof(filename)-1,"/tmp/wfline.jpg"); 
    #endif
        
    #ifdef WF_MODE_WEBSOCKET
        height = 1;
    #endif
    
    drawWF(WFID_BIG,pfdata, width, width, height, fleft, fright, FFT_RESOLUTION, frequency, filename);

    
    wf_pid = 0;
    pthread_exit(NULL); // self terminate this thread
}
