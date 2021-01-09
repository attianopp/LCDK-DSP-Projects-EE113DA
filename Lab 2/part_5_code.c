#include "L138_LCDK_aic3106_init.h"
#include "evmomapl138_gpio.h"
#include <stdint.h>
#include <math.h>
#include <ti/dsplib/dsplib.h>


// Global Definitions and Variables
#define PI 3.14159265358979323
#define N 1024

int16_t flag = 0;


int16_t idx = 0;

/* Align the tables that we have to use */

// The DATA_ALIGN pragma aligns the symbol in C, or the next symbol declared in C++, to an alignment boundary.
// The alignment boundary is the maximum of the symbol's default alignment value or the value of the constant in bytes.
// The constant must be a power of 2. The maximum alignment is 32768.
// The DATA_ALIGN pragma cannot be used to reduce an object's natural alignment.

//The following code will locate mybyte at an even address.
//#pragma DATA_ALIGN(mybyte, 2)
//char mybyte;

//The following code will locate mybuffer at an address that is evenly divisible by 1024.
//#pragma DATA_ALIGN(mybuffer, 1024)
//char mybuffer[256];
#pragma DATA_ALIGN(x_in,8);
int16_t x_in[2*N];
int16_t x_in_new [N];

#pragma DATA_ALIGN(x_sp,8);
float   x_sp [2*N];
#pragma DATA_ALIGN(y_sp,8);
float   y_sp [2*N];
#pragma DATA_ALIGN(w_sp,8);
float   w_sp [2*N];

float magnitude [N];


// brev routine called by FFT routine
unsigned char brev[64] = {
		0x0, 0x20, 0x10, 0x30, 0x8, 0x28, 0x18, 0x38,
		0x4, 0x24, 0x14, 0x34, 0xc, 0x2c, 0x1c, 0x3c,
		0x2, 0x22, 0x12, 0x32, 0xa, 0x2a, 0x1a, 0x3a,
		0x6, 0x26, 0x16, 0x36, 0xe, 0x2e, 0x1e, 0x3e,
		0x1, 0x21, 0x11, 0x31, 0x9, 0x29, 0x19, 0x39,
		0x5, 0x25, 0x15, 0x35, 0xd, 0x2d, 0x1d, 0x3d,
		0x3, 0x23, 0x13, 0x33, 0xb, 0x2b, 0x1b, 0x3b,
		0x7, 0x27, 0x17, 0x37, 0xf, 0x2f, 0x1f, 0x3f
};

// The seperateRealImg function separates the real and imaginary data
// of the FFT output. This is needed so that the data can be plotted
// using the CCS graph feature
float y_real_sp [N];
float y_imag_sp [N];

separateRealImg () {
	int i, j;

	for (i = 0, j = 0; j < N; i+=2, j++) {
		y_real_sp[j] = y_sp[i];
		y_imag_sp[j] = y_sp[i + 1];
	}
}

float x_real_sp[N];
float x_imag_sp[N];



separateRealImgForX () {
	int i, j;

	for (i = 0, j = 0; j < N; i+=2, j++) {
		x_real_sp[j] = x_sp[i];
		x_imag_sp[j] = x_sp[i + 1];
	}
}

// Function for generating sequence of twiddle factors
void gen_twiddle_fft_sp (float *w, int n)
{
	int i, j, k;
	double x_t, y_t, theta1, theta2, theta3;

	for (j = 1, k = 0; j <= n >> 2; j = j << 2)
	{
		for (i = 0; i < n >> 2; i += j)
		{
			theta1 = 2 * PI * i / n;
			x_t = cos (theta1);
			y_t = sin (theta1);
			w[k] = (float) x_t;
			w[k + 1] = (float) y_t;

			theta2 = 4 * PI * i / n;
			x_t = cos (theta2);
			y_t = sin (theta2);
			w[k + 2] = (float) x_t;
			w[k + 3] = (float) y_t;

			theta3 = 6 * PI * i / n;
			x_t = cos (theta3);
			y_t = sin (theta3);
			w[k + 4] = (float) x_t;
			w[k + 5] = (float) y_t;
			k += 6;
		}
	}
}

interrupt void interrupt4(void) // interrupt service routine
{
	int16_t left_sample;

	// Input from ADC (Line IN)
	left_sample = input_left_sample();

	// Your code here
	if(idx<N){
		// Input is being read sample by sample real part in even indices, imaginary in odd.
		x_in[2*idx]=left_sample;
		x_in[2*idx+1]=(float)0.0;

		// Variable idx is global and its value is kept
		idx++;
	}

	//flag
	if (idx == 1024){
		flag = 1;
	}

	// Output to DAC (Line OUT)
	output_left_sample(left_sample);
	return;
}


int main(void)

{
	L138_initialise_intr(FS_48000_HZ,ADC_GAIN_0DB,DAC_ATTEN_0DB,LCDK_LINE_INPUT);

	// SAMPLE CODE: USE OF FFT ROUTINES

	// Copy input data to the array used by DSPLib functions
	while (1){
		if(flag == 1){

			int16_t n;
			for (n=0; n<N; n++)
			{
				x_sp[2*n]   = x_in[2*n];
				x_sp[2*n+1] = x_in[2*n+1];
				x_in_new[n]=  x_in[2*n];
			}



			//	int16_t i =0;
			//for (i =4; i<10; i=i+1){
			//printf ("Value of %dth is %d\n", i,x_in_new[i]);

			//}

			// Call twiddle function to generate twiddle factors needed for FFT and IFFT functions
			gen_twiddle_fft_sp(w_sp,N);

			// Call FFT routine
			DSPF_sp_fftSPxSP(N,x_sp,w_sp,y_sp,brev,4,0,N);

			// Call routine to separate the real and imaginary parts of data
			// Results saved to floats y_real_sp and y_imag_sp
			separateRealImg ();



			// Call the inverse FFT routine
			DSPF_sp_ifftSPxSP(N,y_sp,w_sp,x_sp,brev,4,0,N);

			separateRealImgForX();


//			int i = 0;
//			for (i = 0; i < N; i++){
//				magnitude [i]= sqrt(y_real_sp[i]*y_real_sp[i]+y_imag_sp[i]*y_imag_sp[i]);
//			}
			// END OF SAMPLE CODE

		}
	}

}
