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
* playSDRweb.h ... configuration options for playSDRweb
*  
* 
*/

// sample rate of the SDRplay hardware
// see the possible values in the SDRplay driver
// high values will generate a high CPU load !
// its best to keep the default value of 2.4MS/s
#define SDR_SAMPLE_RATE 2400000 

// width of the big waterfall in Hz
// attention: the expression (SDR_SAMPLE_RATE / 2 / WF_RANGE_HZ) HAS to 
// be an integer number without decimal places !!!
// (the following alternatives are integer numbers if the sampling rate is 2.4MS/s)
//#define WF_RANGE_HZ        400000       // (first downsampled rate: 800000)
// #define WF_RANGE_HZ        300000       // (first downsampled rate: 600000)
// #define WF_RANGE_HZ        240000       // (first downsampled rate: 480000)
 #define WF_RANGE_HZ        200000       // (first downsampled rate: 400000) 
// #define WF_RANGE_HZ        150000       // (first downsampled rate: 300000)
// #define WF_RANGE_HZ        120000       // (first downsampled rate: 240000)
//#define WF_RANGE_HZ        100000       // (first downsampled rate: 200000)

// width of the big waterfall in pixels
#define WF_WIDTH    1000

// the height of the waterfall picture (ignored in WF_MODE_WEBSOCKET)
#define WF_HEIGHT   400     

// drawing mode (see waterfall.c), choose FILE or WEBSOCKET mode
#define WF_MODE_FILE     // draw a waterfall picture into a file
//#define WF_MODE_WEBSOCKET   // create one line using the actual data and send it to a browser

// tuned frequency, the base frequency where the SDR hardware is tuned
// this is similar to the left margin of the big waterfall picture
// and therefore should be the lowest frequency of interest (i.e. beginning of a band)
// for the Sat es'hail 2 this should be: 10489500000 Hz (= 10.4895 GHz)
#define TUNED_FREQUENCY     7000000     // 40 band



// ==========================================================================================
// calculations according to above values
// do NOT change !
// ==========================================================================================

#define SAMPLERATE_FIRST    (WF_RANGE_HZ * 2)                       // sample rate after the first down-sampling
#define DECIMATERATE        (SDR_SAMPLE_RATE / SAMPLERATE_FIRST)    // i.e 2.4M / 600k = 4 ... decimation factor
#define FFT_RESOLUTION      (WF_RANGE_HZ / WF_WIDTH)       // frequency step from one FFT value to the next
#define SAMPLES_FOR_FFT     (SAMPLERATE_FIRST / FFT_RESOLUTION)     // required samples for the FFT to generate to requested resolution

