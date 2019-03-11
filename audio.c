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
* audio.c ... play the demodulated samples to the sound card
* 
*/

#include <pthread.h>
#include <alsa/asoundlib.h>
#include <sndfile.h>
#include <time.h>
#include <sys/time.h>

snd_pcm_t *playback_handle=NULL;

void play_samples(double *samp, int len)
{   
static int pbfirst = 1;
int err;

	// the USB demodulated samples are now in samp
	// convert to 16 bit and double for stereo output
    short samples[len * 2];
    for (int i = 0; i < len; i++)
    {
        samples[i*2] = (short)(samp[i]);
        samples[i*2+1] = (short)(samp[i]);
    }
    
	if ((err = snd_pcm_writei(playback_handle, samples, len)) != len) {
		printf("write to audio interface failed (%s)\n", snd_strerror(err));
		exit(0);
	}
 
	// write the first frame many times to fill the buffer and aviod underrun
	if (pbfirst == 1)
	{
		pbfirst = 0;
        // this may introduce a delay, so keep this as short as possible
        // if its too short, an sound underrun will occur
        for(int i=0; i<10; i++)
        {
            if ((err = snd_pcm_writei(playback_handle, samples, len)) != len) {
                printf("1:write to audio interface failed (%s)\n", snd_strerror(err));
                return;
            }
        }
	}
}

// initialize the sound device for playback
// usually I use "pulse" as sndcard because then
// we can use pavucontrol to control and route the audio
void init_soundcard(char *sndcard, unsigned int rate)
{
	int err;
	int channels = 2;
	snd_pcm_hw_params_t *hw_params;
    
    printf("open %s\n\r",sndcard);

    if ((err = snd_pcm_open(&playback_handle, sndcard, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        printf("please disable playback. cannot open audio device %s (%s)\n", sndcard, snd_strerror(err));
        
    }

    else if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
        printf("please disable playback.cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
        
    }

    else if ((err = snd_pcm_hw_params_any(playback_handle, hw_params)) < 0) {
        printf("please disable playback. cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
        
    }

    else if ((err = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        printf("please disable playback.cannot set access type (%s)\n", snd_strerror(err));
        
    }

    else if ((err = snd_pcm_hw_params_set_format(playback_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
        printf("please disable playback.cannot set sample format (%s)\n", snd_strerror(err));
        
    }

    else if ((err = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &rate, 0)) < 0) {
        printf("please disable playback. cannot set sample rate (%s)\n", snd_strerror(err));
        
    }

    else if ((err = snd_pcm_hw_params_set_channels(playback_handle, hw_params, channels)) < 0) {
        printf("please disable playback. cannot set channel count (%s)\n", snd_strerror(err));
        
    }

    else if ((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0) {
        printf("please disable playback. cannot set parameters (%s)\n", snd_strerror(err));
        
    }
    else
    {
        snd_pcm_hw_params_free(hw_params);

        if ((err = snd_pcm_prepare(playback_handle)) < 0) {
            printf("please disable playback. cannot prepare audio interface for use (%s)\n", snd_strerror(err));
            
        }
    }
}
