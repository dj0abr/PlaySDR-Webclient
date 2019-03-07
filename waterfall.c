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
#include <pthread.h>
#include "fft.h"

/*
 * do the FFT and the drawing in a separate process
 * to keep the SDR hardware's callback function as fast as possible
 */
pthread_t wf_pid = 0;

typedef struct {
    short *pi;
    short *pq;
    int cnt;
} WFDATA;

void *wfproc(void *pdata);

void draw_waterfall(short *pi, short * pq, int cnt)
{
    if(wf_pid == 0)
    {
        // if the previous drawing is not finished, ignore the data
        // this makes the waterfall slower, but it is still ok
        // and will also work with slower CPUs
        return;
    }
    
    WFDATA wfdata;
    wfdata.pi = pi;
    wfdata.pq = pq;
    wfdata.cnt = cnt;
    
    // TODO: there is a risk that pi and pq get overwritten by the next samples
    // if the CPU is too slow. No problem for now, but if it happens we have to make
    // a copy of pi and pq here. For now we avoid copying the data, but in case ...
    
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
    int cnt = uFFT_calc(0, pwf->pi, pwf->pq, pwf->cnt, 7000000, 0);
    if(cnt > 0)
    {
        /*// fft data available in uFFT_dout, length: cnt
        int fleft = 0;
        int fright = 200000;
        int res = (fright-fleft)/1000;
        
        // draw waterfall (one line only = send one line via WebSocket)
        wf_drawWF(0,fftd[0].fftData, cnt, (fright-fleft)/res, 1, fleft,fright,res,frequency,"/tmp/wfline.pix");*/
    }
    
    wf_pid = 0;
    pthread_exit(NULL); // self terminate this thread
}
