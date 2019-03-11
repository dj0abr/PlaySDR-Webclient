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
* playSDRweb.c ... file containing main() and calling all other functions
* 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "sdrplay.h"
#include "sampleprocessing.h"
#include "fft.h"
#include "audio.h"
#include "downmixer.h"
#include "ssb.h"
#include "wf_univ.h"
#include "websocketserver.h"

void sighandler(int signum)
{
	printf("signal %d, exit program\n",signum);
    remove_SDRplay();
    exit(0);
}

void sighandler_mem(int signum)
{
	printf("memory error, signal %d, exit program\n",signum);
    remove_SDRplay();
    exit(0);
}

int main()
{
    // make signal handler, mainly use if the user presses Ctrl-C
    struct sigaction sigact;
    sigact.sa_handler = sighandler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGQUIT, &sigact, NULL);
	sigaction(SIGPIPE, &sigact, NULL);
    
    struct sigaction sigact_mem;
    sigact_mem.sa_handler = sighandler_mem;
	sigemptyset(&sigact_mem.sa_mask);
	sigact_mem.sa_flags = 0;
    sigaction(SIGSEGV, &sigact_mem, NULL);
    
    printf("\nplaySDRweb parameters:\n\n");
    printf("SDR base QRG:    %d Hz\n",TUNED_FREQUENCY);
    printf("SDR sample rate: %d S/s\n",SDR_SAMPLE_RATE);
    printf("WF width:        %d Hz\n",WF_RANGE_HZ);
    printf("WF width:        %d pixel\n",WF_WIDTH);
    printf("1st downsampling:%d S/s\n",SAMPLERATE_FIRST);
    printf("1st decim. rate: %d\n",DECIMATERATE);
    printf("1st FFT resol.:  %d Hz\n",FFT_RESOLUTION);
    printf("1st FTT smp/pass:%d\n",SAMPLES_FOR_FFT);
    printf("SSB audio rate  :%d\n",SSB_RATE);
    printf("SSB audio decim :%d\n",SSB_DECIMATE);
    printf("2nd FFT resol.  :%d\n",FFT_RESOLUTION_SMALL);
    printf("2nd FTT smp/pass:%d\n",SAMPLES_FOR_FFT_SMALL);
    
    // init anti aliasing filters
    init_hs_filters();
    
    // init the FFT for the big waterfall
    init_ffts();
    
    // init downmixer
    downmixer_init(SAMPLERATE_FIRST);
    
    // init waterfall drawing
    init_wf_univ();
    
    // init the Websocket Server
    ws_init();
    
    // init audio output
    init_soundcard("pulse", SSB_RATE);
    
    printf("\nInitialisation complete, system running ... stop with Ctrl-C\n");
    
    // init SDRplay hardware
    // this MUST be the LAST initialisation because
    // the callback starts immediately after init_SDRplay
    init_SDRplay();

    // infinite loop, 
    // stop program with Ctrl-C
    while(1)
    {
        usleep(10000);
    }
    
    return 0;
}
