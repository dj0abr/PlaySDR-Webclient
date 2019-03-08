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
* wf_univ.c ... drawing routines for waterfalls, using gdlib
* 
* usage:
* drawWF(int _id, double *fdata, int cnt, int wpix, int hpix, int _leftqrg, int _rightqrg, int res, int _tunedQRG, char *fn);
* 
* id ... a unique number (used to create a temporary file name, see wf_univ.h)
* fdata ... a double array containing the FFT result
* cnt ... number of FFT results (or less)
* wpix ... width of the waterfall bitmap in pixels (equal or less then cnt)
* hpix ... height of the waterfall bitmap in pixels
* _leftqrg ... frequency offset of the left margin of the bitmap (usually 0, the first FFT value)
* _rightqrg ... frequency offset of the right margin of the bitmap (usually: FFTRESOLUTION in Hz * width in pixels)
* res ... FFT resolution in Hz per FFT value (= Hz per pixel)
* _tunedQRG ... frequency where the SDR receivers is tuned (used for displaying the frequncy in the title)
* fn ... path and filename of the bitmap
* 
* the bitmap is created as a temporary BMP file in the /tmp folder.
* after drawing this is copied to fn
* 
*/

#include <stdio.h>
#include <string.h>
#include <gd.h>
#include "gdfontt.h"
#include "gdfonts.h"
#include "gdfontmb.h"
#include "gdfontl.h"
#include "gdfontg.h"
#include "wf_univ.h"
#include "fft.h"
#include "color.h"

void drawWFimage(gdImagePtr im, char *fn);
void drawFFTline(gdImagePtr dst);
void drawQrgTitles(gdImagePtr dst);

int dg_first[WFID_MAX];
int first = 1;
int id;
double *fftdata;    // data from fftcnt
int fftcnt;         // number of values from fftcnt
int pic_width;      // number of horizontal pixels of the resulting WF diagram
int pic_height;     // number of vertial pixels of the resulting WF diagram
                    // set pic_height=1 for one pixel line only
char filename[256]; // store the bitmap into this file
int leftqrg;        // qrg of the left margin of the bitmap
int rightqrg;       // qrg of the right margin of the bitmap
int resolution;     // resolution of the fft data in Hz per value
int toprow;         // start drawing here, keep lines above toprow clear for the title
int tunedQRG;    // base QRG for titles

void drawWF(int _id, double *fdata, int cnt, int wpix, int hpix, int _leftqrg, int _rightqrg, int res, int _tunedQRG, char *fn)
{
    id = _id;
    fftdata = fdata;
    fftcnt = cnt;
    pic_width = wpix;
    pic_height = hpix;
    strcpy(filename,fn);
    leftqrg = _leftqrg;
    rightqrg = _rightqrg;
    resolution = res;
    toprow = 20;    // lines above toprow are used for the titles, below toprow for the moving waterfall
    tunedQRG = _tunedQRG;
    
    if(first)
    {
        // init WFs
        for(int i=0; i<WFID_MAX; i++)
            dg_first[i] = 1;
        first = 0;
    }
    
    // scale the fft data to match antenna dBm
    scaleSamples(fftdata, fftcnt); 
    
    if(hpix > 1)
    {
        // if the requested height of the bitmap is > 1, draw a bitmap
        
        // filename of a temporary bitmap
        // make sure the /tmp directory is a RAM disk if you run the system on an SD card !
        char fn[512];
        sprintf(fn,"/tmp/w%d.bmp",id);
        
        // read the bitmap (if exists)
        gdImagePtr im = gdImageCreateFromFile(fn);
        if(im && dg_first[id] == 0)
        {
            drawWFimage(im, fn);
            // free ressources
            gdImageDestroy(im);
        }
        else
        {
            dg_first[id] = 0;
            // image file does not exist, create an empty image
            printf("Create new WF image: %s\n",fn);
            
            // create Image
            gdImagePtr im = gdImageCreate(pic_width,pic_height);      
            // black background
            gdImageColorAllocate(im, 0, 0, 0);  
            // draw frequency titles
            drawQrgTitles(im);
            // write to file
            gdImageFile(im, fn);
            // free ressources
            gdImageDestroy(im);
        }
           
        // the temporary bmp is needed because we need every pixel for copying
        // now the bmp image is ready convert it to jpg
        gdImagePtr convim = gdImageCreateFromFile(fn);
        if(convim)
        {
            gdImageFile(convim, filename);
            gdImageDestroy(convim);
        }
    }
    /*else
    {
        // special handling for height=1px, one WF line
        FILE *fw = fopen(filename,"w");
        if(fw)
        {
            // save the data in /tmp/wfline.pix in this format:
            // 1 byte: ID counter
            // 1 byte: waterfall ID
            // 4 byte: left margin frequency in Hz
            // 4 byte: right margin frequency in Hz
            // 4 byte: tuner frequency in Hz
            // 4 byte: resolution Hz per pixel
            // 4 byte: offset to tuner frequency in Hz
            // followed by the dBm data, 1 byte per pixel holding the dBm value
            unsigned char wfdata[10000];
            int idx = 0;
            static unsigned char idcnt = 0;
            
            wfdata[idx++] = idcnt++;
            wfdata[idx++] = id;
            
            wfdata[idx++] = leftqrg >> 24;
            wfdata[idx++] = leftqrg >> 16;
            wfdata[idx++] = leftqrg >> 8;
            wfdata[idx++] = leftqrg;
            
            wfdata[idx++] = rightqrg >> 24;
            wfdata[idx++] = rightqrg >> 16;
            wfdata[idx++] = rightqrg >> 8;
            wfdata[idx++] = rightqrg;
            
            int tqrg = tunedQRG;
            wfdata[idx++] = tqrg >> 24;
            wfdata[idx++] = tqrg >> 16;
            wfdata[idx++] = tqrg >> 8;
            wfdata[idx++] = tqrg;
            
            wfdata[idx++] = resolution >> 24;
            wfdata[idx++] = resolution >> 16;
            wfdata[idx++] = resolution >> 8;
            wfdata[idx++] = resolution;

            wfdata[idx++] = foffset >> 24;
            wfdata[idx++] = foffset >> 16;
            wfdata[idx++] = foffset >> 8;
            wfdata[idx++] = foffset;

            int left = leftqrg / resolution;
            int right = rightqrg / resolution;

            // calculate pixel colors
            calcColorParms(id,left,right,fdata); 
            
            // draw pixel per pixel
            for(int i=left; i<right; i++)
            {
                //printf("%d\n\r",getPixelColor(fdata[i]));
                wfdata[idx++] = getPixelColor(id,fdata[i]);
                if(idx >= sizeof(wfdata))
                    break;
            }
            
            fwrite(wfdata,1,idx,fw);
            
            fclose(fw);
        }
        else
        {
            printf("cannot open %s for writing\n",filename);
        }
    }*/
    
}

/*
 * draw the waterfall image is done in these steps:
 * - create a new empty destination image
 * - copy the title from the existing image to this new image (copy unmodified, as it is)
 * - copy the existing waterfall picture, one line to the bottom, which creates the moving picture
 * - add the new data in the top line
 */
void drawWFimage(gdImagePtr im, char *fn)
{
    // create destination image
    gdImagePtr dst = gdImageCreate(pic_width, pic_height);
    // allocate the colors
    allocatePalette(dst);
    
    // draw the existing image into the new destination image
    // Title: copy it as it is
    gdImageCopy(dst,im,0,0,0,0,pic_width,toprow); 
    // WF: shifted by 1 line (keep the top line empty)
    gdImageCopy(dst,im,0,toprow+1,0,toprow,pic_width,(pic_height-toprow)-1); 
    // and draw the top line with the new fft data
    drawFFTline(dst);
    // write to file
    gdImageFile(dst, fn);
    // free ressources
    gdImageDestroy(dst);
}

/*
 * usng the new FFT data, create a new waterfall line
 */
void drawFFTline(gdImagePtr dst)
{
    int left = leftqrg / resolution;
    int right = rightqrg / resolution;
    int width = right-left;
    
    // calculate pixel colors
    calcColorParms(id,left,right,fftdata); 
    
    // calculate position of vertical lines
    int fullwidth = rightqrg - leftqrg;
    char snum[50];
    sprintf(snum,"%d",fullwidth);
    for(int i=1; i<strlen(snum); i++) 
        snum[i] = '0';
    
    int numOfMarkers = 4;
    int markersteps = atoi(snum) / numOfMarkers;
    
    // draw pixel per pixel
    for(int i=left; i<right; i++)
    {
        // scale the frequency to pixel position
        int xs = ((i-left)*pic_width)/width;
        int xe = (((i+1)-left)*pic_width)/width;
        
        // at a 100kHz boundary make a thick white line
        if(!((i*resolution) % markersteps) && (i != left) && (i != right))
        {
            gdImageLine(dst, xs, toprow, xe, toprow, 253);
        }
        else
        {
            // and draw the pixel in the color corresponding to the level
            gdImageLine(dst, xs, toprow, xe, toprow, getPixelColor(id,fftdata[i]));
        }
    }
}

/*
 * draw the title
 */
void drawQrgTitles(gdImagePtr dst)
{
char s[50];
    
    allocatePalette(dst);
    
    int left = leftqrg / resolution;
    int right = rightqrg / resolution;
    int width = right-left;
    
    // calculate position of vertical lines
    int fullwidth = rightqrg - leftqrg;
    char snum[50];
    sprintf(snum,"%d",fullwidth);
    for(int i=1; i<strlen(snum); i++) 
        snum[i] = '0';
    
    int numOfMarkers = 4;
    int markersteps = atoi(snum) / numOfMarkers;
    
    for(int i=left; i<right; i++)
    {
        // insert title text
        if(!((i*resolution) % markersteps) && (i != left) && (i != right))
        {
            // scale the frequency to pixel position
            int xs = ((i-left)*pic_width)/width;
            
            sprintf(s,"%.6f",(double)((double)tunedQRG + i*(double)resolution)/1e6);
            gdImageString (dst,gdFontGetSmall(),xs-25,2,(unsigned char *)s,253);
        }
    }
}
