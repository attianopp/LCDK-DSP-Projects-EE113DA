# LCDK-DSP-Projects-EE113DA
Capstone electrical engineering undergrad course in digital signal processing design. Consisted of 4 labs and 2 projects all of which used an embedded system (LCDK from TI) for development and deployment of software specific to digital signal processing (using Code Composer Studio 5 as the IDE and C as the langauge). The purpose of the 4 labs was to become familiar using the LCDK as a tool for development and understanding its DSP limitations. The highlight of the 4 labs was turning the LCDK into a speech modulation tool that could slow down, speed up, and add an echo effect to any audio input to the LCDK through a connected microphone.

The purpose of the first project was to turn the LCDK into an object detector that could determine the shape of the any straight line black and white .bmp image passed into the LCDK through USB. The digital signal processed in this project was 2d black and white images.

The purpose of the second project was to turn the LCDK into an speech recognition tool that could determine if audio spoken into a mic connected to the LCDK was a monophthong vowel such as A, E, O, or U. This required building a single layer nueral network in matlab and exporting the weights into the c program for the LCDK.

Lab 1 objective: 

to become familiar with Computer Composer Studio software, the LCDK programmable board, and oscilloscope features. The lab required generating specific sine waves, being coded in the programming language C, and output them on the oscilloscope. An additional purpose of this lab is to review various digital signal processing concepts such as nyquist’s theorem and aliasing.

Lab 2 objective:

to become familiar with the FFT function in Code Composer Studio(CCS) and creating input from the function generator. The lab required reading input from the function generator to an array, then taking the FFT and IFFT of the array after performing computations in the frequency domain. In addition to taking the FFT of an oddsized array, a square wave, and a 2-Dimensional FFT.

Lab 3 objective:

to manipulate sound waveforms with the LCDK. In the first part of the lab the task is to create a delay between output pulses of the LCDK and function generator using a FIFO buffer. For the last part of the lab the task is manipulating a sound wave in several ways: applying 1/2 speed, 1/3 speed, 2x speed, and an echo effect. 


Lab 4 objective:

to compare filters from the LCDK to the digital filter in Matlab. In step 1, the function generator and excel are used to create bode plots of the LDCK frequency response. Step 2 was the creation of a digital filter in Matlab and comparison of it with a filter created in Code Composer studio.
