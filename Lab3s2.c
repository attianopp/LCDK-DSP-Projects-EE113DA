#include "L138_LCDK_aic3106_init.h"
#include "evmomapl138_gpio.h"
#include "L138_LCDK_switch_led.h"
#include <stdint.h>
#include <math.h>
#include <stdio.h>

// Global Definitions and Variables
int fifo[24];
int i=0;
int total=0;
int all_light_counter=0;
int testicle_count=0;
int16_t input_buffer[8000];
int buffer_index=0;
int temp_max=0;
int state_6_counter=0;
int state_7_count=0;
int state_8_count=0;
int16_t output;
int counttt=0;
void find_max_in_8000(int* p){
	int j=0;
	for(j=0;j<8000;j++){
		if(abs(p[j])>temp_max){
			temp_max=abs(p[j]);
		}
	}
}


interrupt void interrupt4(void) // interrupt service routine
{


	total = LCDK_SWITCH_state(5)+LCDK_SWITCH_state(6)+LCDK_SWITCH_state(7)+LCDK_SWITCH_state(8);

	//if(i==24){
	//	i=0;
	//}
	int16_t left_sample;
	left_sample = input_left_sample();
	if(total==0){
		testicle_count=0;
		buffer_index=0;
		LCDK_LED_off(4);
		LCDK_LED_off(6);
		LCDK_LED_off(5);
		LCDK_LED_off(7);
		output_left_sample(0);
	}
	else if(total>1){
		testicle_count=0;
		buffer_index=0;
		all_light_counter++;
		//error more than two switches turned
		//flash every half second
		if(all_light_counter<4000 || all_light_counter == 4000){
			LCDK_LED_on(4);
			LCDK_LED_on(5);
			LCDK_LED_on(6);
			LCDK_LED_on(7);
		}
		else if(all_light_counter>4000){
			if(all_light_counter>8000){
				all_light_counter=0;
			}
			LCDK_LED_off(4);
			LCDK_LED_off(5);
			LCDK_LED_off(6);
			LCDK_LED_off(7);
		}
		output_left_sample(0);
	}
	else if (total==1){
		if(LCDK_SWITCH_state(5)){
			testicle_count++;
			if(testicle_count<8000){
				LCDK_LED_off(5);
				LCDK_LED_off(6);
				LCDK_LED_off(7);
				LCDK_LED_on(4);
			}
			else if(testicle_count > 8000 && testicle_count<16000){
				LCDK_LED_off(4);
				LCDK_LED_off(6);
				LCDK_LED_off(7);
				LCDK_LED_on(5);
			}
			else if(testicle_count > 16000 && testicle_count<24000){
				LCDK_LED_off(5);
				LCDK_LED_off(4);
				LCDK_LED_off(7);
				LCDK_LED_on(6);

			}
			else if(testicle_count > 24000 && testicle_count<32001){
				LCDK_LED_off(4);
				LCDK_LED_off(6);
				LCDK_LED_off(5);
				LCDK_LED_on(7);
				input_buffer[buffer_index]=left_sample;
				buffer_index++;
			}
			else if(testicle_count > 32000){
				LCDK_LED_off(7);
				LCDK_LED_off(6);
				LCDK_LED_off(5);
				LCDK_LED_on(4);

			}
			output_left_sample(0);

		}
		else if(LCDK_SWITCH_state(6)){
			LCDK_LED_off(4);
			LCDK_LED_off(7);
			LCDK_LED_off(6);
			LCDK_LED_on(5);
			testicle_count=0;
			buffer_index=0;
			state_6_counter++;
			//560 samples between first and second output
			if(state_6_counter<560){
				output=input_buffer[state_6_counter];

			}
			else{
				//find_max_in_8000(input_buffer);
				output=(input_buffer[state_6_counter]+(2.0/3.0)*input_buffer[state_6_counter-560]);
			}
			if(state_6_counter==7999){
				state_6_counter=0;
			}
			output_left_sample(output);
		}
		else if(LCDK_SWITCH_state(7)){
			LCDK_LED_off(4);
			LCDK_LED_off(7);
			LCDK_LED_off(5);
			LCDK_LED_on(6);
			testicle_count=0;
			buffer_index=0;
			state_7_count++;

			if(state_7_count==3999){
				state_7_count=0;
			}
			output_left_sample(input_buffer[state_7_count*2]);

		}
		else if(LCDK_SWITCH_state(8)){
			LCDK_LED_off(4);
			LCDK_LED_off(6);
			LCDK_LED_off(5);
			LCDK_LED_on(7);
			testicle_count=0;
			buffer_index=0;
			state_8_count++;
			state_8_count=state_8_count%24000;
			if(state_8_count%3==0){
				counttt++;
			}
			counttt=counttt%8000;

			int difference = abs(input_buffer[counttt]-input_buffer[counttt+1]);

			if(state_8_count%3==0){
				output=input_buffer[counttt];
			}
			else if(state_8_count%3==1){
				output=input_buffer[counttt]+(1.0/3.0)*(float)(input_buffer[counttt+1]-input_buffer[counttt]);
			}
			else if (state_8_count%3==2){
				output=input_buffer[counttt]+(2.0/3.0)*(float)(input_buffer[counttt+1]-input_buffer[counttt]);
			}

			output_left_sample(output);
		}
	}

	// Input from ADC (Line IN)
	//left_sample = input_left_sample();
	//output_left_sample(fifo[i]);
	//fifo[i]=left_sample;
	//i++;

	// Output to DAC (Line OUT)

	return;
}


int main(void)
{
	int j=0;
	for(j=0;j<24;j++){
		fifo[j]=0;
	}
	L138_initialise_intr(FS_8000_HZ,ADC_GAIN_24DB,DAC_ATTEN_0DB,LCDK_MIC_INPUT);
	LCDK_GPIO_init();
	LCDK_SWITCH_init();
	LCDK_LED_init();

	// SAMPLE CODE: USE OF FFT ROUTINES
	while(1);
}


