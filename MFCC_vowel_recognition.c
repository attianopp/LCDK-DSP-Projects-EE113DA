#include "L138_LCDK_aic3106_init.h"
#include "evmomapl138_gpio.h"
#include <stdint.h>
#include <math.h>
#include <ti/dsplib/dsplib.h>
#include "L138_LCDK_switch_led.h"

//Define global constant
#define PI 3.14159265358979323
//sample size
#define N 1024
//hidden layer size
#define P 5
//output layer size
#define Q 4

//Declare the variables

//Counter for input data
int idx = 0;
int n=0;
int mfccRow = 0;

//Flag
int recorded=0;
int g=0;
int ignore100=0;

//Store the filter bank 
float bank[26][513];
//Store the regular frequency 
float freq[28];
//Store the value for lamdba 
float lamda[28];
//Store the value after filter bank
float filtered[26];
//Store the value for output energy
float outputLog[26];
//Store the data after hamming window
float w[N];
//Store the magnitude square 
float fftMagSq[N/2+1];
//MFCC 2D array set
float MFCC_set[40][13];


//Neural network variables 
float Y[13]={0};
float hidden_layer[5]={0};
float output_layer[4]={0};
float o[5]={0};
//Neural network constant

//% Input 1
float x1_step1_xoffset[13] = {-10.09512,-8.043056,4.454937,-10.92074,-4.696647,-3.079764,-6.563395,-3.560701,-4.342183,-7.292183,-1.288938,-5.610725,-5.694504};
float x1_step1_gain[13] = {0.0652030749770159,0.121389843299672,0.131980332554803,0.155860519174507,0.195000613276929,0.218908219335966,0.184246025191406,0.158379335931282,0.170893095004684,0.190673201291506,0.216940774951497,0.241506080217452,0.228368871906872};
float x1_step1_ymin = -1;

//% Layer 1
float b1[5] = {-0.83593533392167108,0.059356427997956478,-0.31821072025888769,-0.54502722018977667,0.54102554090705057};
float IW1_1[5][13]= {{-1.0201400773811309,1.428682089871627,0.3418291868674066,-1.0586395500245995,0.22396749214701114, 0.94636128155801458, -0.7280244709185254, 0.88825350142363946, 1.6040704484134143, -1.7703268637810083, 0.63250479687836503 ,0.065106823455411453, -1.2736460529153157},{-1.5404921284333803 ,0.092448650875276844, 0.68662123869836988, 1.3428016623810108, 2.0860827637500381, 0.23686826473989903, -1.3983566060516406, -0.94960288848145258, 0.21175145343842569, 1.112853988713147, 0.011481245472264732, -1.9177937024553573, 0.58589071485859379},{-1.6990921562466814, -0.59119339359284528 ,0.77867397565089247, 0.86009956220268713, 1.21493602931561, 0.30032136604941506 ,-0.36632892357836033 ,0.50774294555375965 ,-0.55112362227757405 ,1.4160503480809976 ,-0.59191413250796254, -1.3779960801661209, 0.45993177534163138},{0.64454556474241809, -1.4782760039085576, -1.2218912335847931 ,-0.64547317387737746, 0.84989887958031618 ,-1.1959777078465925 ,-0.4199967963706675, -0.37252996283482592 ,-0.62383947233835935, 0.053345039803338573, -0.70332093344847291, 0.20741916084354431, -0.38789293138868364},{1.4633817259851218 ,-0.38874389016232291 ,-1.2082952137304579, 0.50202163557681867, 0.14963494180124931 ,-0.45333223195312267, 0.095920810315253097 -0.52875226472134929 ,0.4024321484136249 ,-0.023882403065374661 ,-0.19940225732928871, 0.89069221135728338 ,-0.68381320048495309}};

//% Layer 2
float b2[4] = {-0.6494655392390043,0.028328336845603291,0.83923987969660574,0.88473182999309175};
float LW2_1[4][5] = {{-2.3678584969498213 ,-2.6482587800621671, -3.3188961162336752, 3.8394522380232359, 0.30825002157608311},{0.18353287596039591 ,3.2034106304943655, 3.7564606503117859, 4.5157244925864788, 1.8630656674071955},{-0.36599360150603411, 2.8546582109665315 ,2.2490439740847554, -4.1155097328931474, -2.9246434263239656},{3.7167914319733897, -2.9571330904027358, -1.2959815608316885 ,-3.2601405857144283, 0.34224987524149758}};



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

#pragma DATA_ALIGN(x_sp,8);
float   x_sp [2*N];
#pragma DATA_ALIGN(y_sp,8);
float   y_sp [2*N];
#pragma DATA_ALIGN(w_sp,8);
float   w_sp [2*N];

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
//Reset flags 
void resetFlags()
{
	if (recorded)
		recorded = 0;
	if (ignore100)
		ignore100 = 0;
}

//Apply hamming window and return in the array 
void hammingFunc (float *w, int n)
{
	int i;
	for (i=0;i<N;i++)
	{
		w[i]= x_in[2*i]*(0.54-0.46*cos((2*PI*i)/(n-1)));
	}
}

//Calculate the FFT magnitude square
void fftMag(float *output, float *input)
{
	int i;
	for(i=0;i<N;i++)
	{
		output[i]=(input[2*i]*input[2*i]+input[2*i+1]*input[2*i+1]);
	}
}

//Convert regular frequency to Mel frequency
float melFrequency(float a)
{
	return 2595*log10f(1+a/700);
}

//Convert Mel frequency to frequency 
float frequency(float a)
{
	return (powf(10,a/2595)-1)*700;
}

//Create the filter bank and store in the bank array
void filterBank()
{

	float melFirst, melLast,interval;

	melFirst=melFrequency(250);
	melLast=melFrequency(8000);
	interval = (melLast-melFirst)/27.0;

	int i;
	for ( i=0;i<28;i++)
	{
		freq[i]=frequency(melFirst+i*interval);
	}

	for(i=0;i<28;i++)
	{
		lamda[i]=513*freq[i]/8000;
	}
	int j;

	for(j=0;j<26;j++)
	{
		for(i=0;i<513;i++)
		{
			if(i+1<lamda[j])
				bank[j][i]=0;
			else if(i+1<(int16_t)lamda[j+1]){
				bank[j][i]=((i+1)-lamda[j])/(lamda[j+1]-lamda[j]);
				if(bank[j][i]<0.0)
					bank[j][i]=bank[j][i]*-1.0;
			}
			else if(i+1<=(int16_t)lamda[j+2])
			{
				bank[j][i]=(lamda[j+2]-(i+1))/(lamda[j+2]-lamda[j+1]);
				if(bank[j][i]<0.0)
					bank[j][i]=bank[j][i]*-1.0;
			}
			else
				bank[j][i]=0;

		}
	}

}

//Interrupt function
interrupt void interrupt4(void) // interrupt service routine
{
	int16_t left_sample;

	// Input from ADC (MIC)
	left_sample = input_sample();
	//Reset flags
	if (LCDK_SWITCH_state(5) == 0 && recording_done == 1)
		resetFlags();
	// Reset flags when switch 6 is turned off

	// Your code here
	if(LCDK_SWITCH_state(5)==1)
	{
		//Ignore first 100 samples
		if (ignore100 < 100)
		{
			ignore100++;
			output_sample(0);
		}
		
		if(idx<N){
			// Input is being read sample by sample real part in even indices, imaginary in odd.
			x_in[2*idx]=left_sample;
			x_in[2*idx+1]=(float)0.0;

			// Variable idx is global and its value is kept
			idx++;
			
			if(idx==N)
			recorded++;
			
			output_sample(0);
		}
	}

	//  Output to DAC (Line OUT)
	output_sample(0);
	return;
}

int main(void)
{
	//initialize LCDK
	L138_initialise_intr(FS_16000_HZ,ADC_GAIN_0DB,DAC_ATTEN_0DB,LCDK_MIC_INPUT);
	LCDK_GPIO_init();
	LCDK_SWITCH_init();
	LCDK_LED_init();

	//Create the filter bank
	filterBank();

	while (1){
		if(recorded ==1)
		{
			//Apply hamming window
			hammingFunc(w,N);

			// Copy input data to the array used by DSPLib functions
			for (n=0; n<N; n++)
			{
				x_sp[2*n]   = w[n];
				x_sp[2*n+1] = 0.0;
			}

			// Call twiddle function to generate twiddle factors needed for FFT and IFFT functions
			gen_twiddle_fft_sp(w_sp,N);

			// Call FFT routine
			DSPF_sp_fftSPxSP(N,x_sp,w_sp,y_sp,brev,4,0,N);

			//Calculate Magnitude Square of fft
			fftMag(fftMagSq, y_sp);
	
			filtered[0]=0;
			int j,i;
			//Calculate and store data after filtered bank
			for(i=0;i<26;i++)
			{
				for(j=0;j<513;j++)
				{
					filtered[i]+=bank[i][j]*fftMagSq[j];
				}
			}
			//Calculate and store output power
			for(j=0;j<26;j++)
			{
				outputLog[j]=log10f(filtered[j]);
			}
			//Calculate and store MFCC
			MFCC[mfccRow][0]=0;
			for(j=0;j<13;j++)
			{
				for(i=0;i<26;i++){
					MFCC[mfccRow][j]+=filtered[i]*cos(j*(i-1/2)*PI/26);
				}
			}
			mfccRow++;
		}
		
		//Implementing the NN with the data from MATLAB
		int z,p,q;
		for (z = 0; z < Z; z++) {
						Y[z] = MFCC[z] - x1_step1_xoffset[z];
						Y[z] *= x1_step1_gain[z];
						Y[z] += x1_step1_ymin;
					};

					// Compute weights for hidden layer neurons
					for (p = 0; p < P; p++) {
						for(z = 0; z < Z; z++)
							temp += IW1_1[p][z] * Y[z];
						temp += b1[p];
						hidden_layer[p] = (2.0 / (1.0 + exp(-2.0 * temp))) - 1.0;
						temp = 0.0;
					}

					// Compute weights for output layer neurons
					for (q = 0; q < Q; q++) {
						for(p = 0; p < P; p++)
							temp += LW2_1[q][p] * hidden_layer[p];
						o[q] = temp + b2[q];

						temp = 0.0;
					}

					for(q = 0; q < Q; q++)
						temp += exp(o[q]);

					for(q = 0; q < Q; q++)
						output_layer[q] = exp(o[q]) / temp;

					temp = 0.0;
	}
	};
}
