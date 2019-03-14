
// to compile: gcc testrtl.c -o testrtl -lrtlsdr -lfftw3 -lm


#include <complex.h>
#include <fftw3.h>
#include <rtl-sdr.h>
#include <stdlib.h>
#include <math.h>



int main(void){

int retval;
int devices;
static rtlsdr_dev_t *dev;
int samples_read;
int n;
uint8_t *buf;	//unsigned 8bit int - I didn't know what it was!, the _t must be 'type'
char * fftresult;



devices = rtlsdr_get_device_count();

for(n=0;n<devices;n++){
	printf("Device %d: %s\n\n",devices,rtlsdr_get_device_name(n));
}

if (devices>0){
	retval = rtlsdr_open(&dev, n-1);	//open last device, first if only one connected!
	printf("Open= %d\n",retval);
}else{
	printf("No Devices found...!");
	exit(0);
}

buf = malloc(10000 * sizeof(uint8_t));



//configure rtlsdr settings
retval = rtlsdr_set_sample_rate(dev, 1024000);
printf("Sample rate set= %d\n",retval);
retval = rtlsdr_set_center_freq(dev, 145000000);
printf("freqset= %d\n",retval);
retval = rtlsdr_set_tuner_gain_mode(dev, 1);
printf("Gain mode set= %d\n",retval);
retval = rtlsdr_set_tuner_gain(dev, 390);
printf("Set gain= %d\n",retval);
retval = rtlsdr_reset_buffer(dev);


//Grab some samples
while(1)
{
    retval = rtlsdr_read_sync(dev, buf, 2048, &samples_read);
    printf("%d: Samples read: %d\n",retval,samples_read);
}

rtlsdr_close(0);


//Configure FFTW to convert the samples in time domain to frequency domain

int NUM_POINTS = 512;
fftw_plan my_plan;
fftw_complex *in, *out;
in = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*samples_read);
out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*samples_read);
my_plan = fftw_plan_dft_1d(NUM_POINTS, in, out, FFTW_FORWARD, FFTW_ESTIMATE);



//convert buffer from IQ to complex ready for FFTW, seems that rtlsdr outputs IQ data as IQIQIQIQIQIQ so ......
int i;
n=0;
for (i=0;i<(samples_read-2);i+=2){
	__real__  in[i] = buf[n]-127;	//sample is 127 for zero signal,so 127 +/-127
	__imag__ in[i] = buf[n+1]-127;
	//printf("%f + %f\n", __real__  in[i], __imag__ in[i]);
	n++;
}

//Convert the comples samples to complex frequency domain
fftw_execute(my_plan);




//compute magnitude from complex = sqrt(real^2 + imaginary^2)
//print the 512 bin spectrum as numbers
int mr,mi;
float m;

for (i=0;i<samples_read;i++){
	mr = __real__ out[i] * __real__ out[i];
	mi = __imag__ out[i] * __imag__ out[i];

	m = sqrt(mr+mi);
	//printf("Bin %d = %f\n",i,m);
}


exit(0);



}
