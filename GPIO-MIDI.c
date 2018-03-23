/*

	* compile: gcc -o GPIO-MIDI GPIO-MIDI.c -lasound -lpigpio -lrt -lwiringPi
	* dedicated gpio (4, 17, 27, 22, 5, 6, 13, 19, 26,
	* 				 	18, 23, 24, 25) - note gpio
	* 						(, 12, 16)	octave up, down
	* 						gpio 20 - chord mode switch plays multiple notes
	* 						gpio 21 - interupt from MCP23017 I2C IO extension IC


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <memory.h>
#include <alsa/asoundlib.h>
#include <alsa/seq.h>
#include <pigpio.h>

//library to interface with IO extension chip MCP23017
#include <wiringPi.h>
#include <mcp23017.h>
#include <mcp23x0817.h>
#include "wiringPiI2C.h"

snd_seq_t *seq;		// variable seq is a pointer to sdn-seq_t type

snd_seq_event_t ev;

int midi_channel;

struct midinote 
{
     int gpio;
     int notenum;
     int notevelocity;
     int polarity;		//gpio polarity (1 = normally open; 0 = normally closed)
};

struct midinote gpionote[17];		//Create array to store 17 gpio and 17 note 

struct mcp_device
{
	int gpioA;
	int gpioB;
	int prev_gpioA;
	int prev_gpioB;
	uint32_t tick_stamp;
	
};

struct mcp_device mymcp[20];		//create 20 of them to to debounce notes.

int notenum_start = 36;		//default start note is c for pedal

bool gpionotessactive = 0;	//detecting whether gpio inputs for notes are active or not

// set up MCP23017
int fd ;//global variable for i2c devices

bool mcp23017_1_INT =1;

int INTCAPB_Previous = 0xff;		//default state is off
int INTCAPA_Previous = 0xff;		//default state is off

int INTCAPB_Current = 0;
int INTCAPA_Current = 0;

int mask_in[8];		//input mask ( ex: mask_in[0] for pin 0). This is to mask input from mcp23017


void ProcessMIDIEvent(int gpio, int level, uint32_t tick)
{
	 printf("GPIO %d became %d at %d \n", gpio, level, tick);

	int i; 
	for(i=0; i<13; i++)
	{
		if ((gpio == gpionote[i].gpio) & level)						//button pressed
		{
			snd_seq_ev_set_noteon(&ev, midi_channel, gpionote[i].notenum, gpionote[i].notevelocity);
			printf(" notenum on: %d \n\n", gpionote[i].notenum);
			
			//If chord mode switch is on then play the octave + the 5th note
			if (gpioRead(20) & level)									//chord mode switch is on
			{
				snd_seq_ev_set_noteon(&ev, midi_channel, gpionote[i].notenum+12, gpionote[i].notevelocity);
				snd_seq_ev_set_noteon(&ev, midi_channel, gpionote[i].notenum+19, gpionote[i].notevelocity);
			}
			
			snd_seq_event_output(seq, &ev);
			snd_seq_drain_output(seq);
			
		}
		
		if ((gpio == gpionote[i].gpio) & !level)					//button released
		{
			snd_seq_ev_set_noteoff(&ev, midi_channel, gpionote[i].notenum, 64);
			printf(" notenum off: %d \n\n", gpionote[i].notenum);
			
			if (gpioRead(20) & !level)									//chord mode switch is off
			{
				snd_seq_ev_set_noteoff(&ev, midi_channel, gpionote[i].notenum+12, gpionote[i].notevelocity);
				snd_seq_ev_set_noteoff(&ev, midi_channel, gpionote[i].notenum+19, gpionote[i].notevelocity);
			}
			
			snd_seq_event_output(seq, &ev);
			snd_seq_drain_output(seq);
			
		}
	}
}

void UpdateOctave(int gpio, int level, uint32_t tick)
{
	// checking gpio 4, 17, 27, 22, 5, 6, 13, 19, 26, 18, 23, 24, and 25 to make sure they are off
	gpionotessactive = gpioRead(4)||gpioRead(17)||gpioRead(27)||gpioRead(22)||gpioRead(5)||gpioRead(6)
							||gpioRead(13)||gpioRead(19)||gpioRead(26)||gpioRead(18)||gpioRead(23)||
								gpioRead(24)||gpioRead(25);			 	
	
   //repopulate notenum for octave change
   int i;
   if (gpio==12 && (notenum_start < 84) && level && !gpionotessactive)			//Octave up button
   
   {
	   notenum_start = notenum_start+12;
	   for(i=0; i<13; i++)
		{
			gpionote[i].notenum = notenum_start+i;

		}
		printf(" notenum_start is: %d\n\n",notenum_start);
	}
   if (gpio==16 &&(notenum_start > 36) && level && !gpionotessactive)			//Octave down button
   
   {
	   
	   notenum_start = notenum_start-12;
	   for(i=0; i<13; i++)
		{
			gpionote[i].notenum = notenum_start+i;

		}
		printf(" notenum_start is: %d\n\n",notenum_start);
	}
}

int tempcount =0;
uint32_t prevTick;
int debounceTick;

int MAX_CHECKS = 10; 		//checks before a switch is debounced
int Debounced_StateB; 	//Debounced state of the switches
int State[10]; 	// Array that maintains bounce status
int Index = 0;				// Pointer into State


void InteruptAlert(int gpio, int level, uint32_t tick)
{
	
		
	State[Index] = wiringPiI2CReadReg8 (fd, MCP23x17_GPIOB);
	++Index;
	int i, j;

	j=0xff;

	for (i=0; i<MAX_CHECKS; i++)
	{ 
		//State[i] = wiringPiI2CReadReg8 (fd, MCP23x17_GPIOB);
		j = j & State[i];
	}

	Debounced_StateB = j;
	if(Index>=MAX_CHECKS) Index=0;
	debounceTick = gpioTick() - prevTick;	//debounceTick should be very small if value just changed
	
	if((INTCAPB_Previous != Debounced_StateB)) 			//only read mcp23017 if there are changes from previous value interupt
	{
		//printf("Debounce Tick: %d \n", debounceTick); 
		printf("tempcount %d \n", tempcount++);
		//printf("MCP23x17_GPIOB: 0x%02x \n\n", Debounced_StateB); //read gpio register to get value and to clear interupt
		INTCAPB_Previous = Debounced_StateB;
		
		if(!(mask_in[0] & Debounced_StateB))			//note on
		{
			printf("c on \n");
		}
		else if (mask_in[0] & Debounced_StateB)		//note off
		{
			printf("c off\n");
		}
	}
	if(!mcp23017_1_INT) {
		prevTick = gpioTick();				//whenever the value just changed. restart timer
	}
}



void UpdateI2C_Polling(void)
{
	

}

int main()
{
	
	printf("Starting main routine \n");
	
	
	midi_channel=0;					//set initial midi channel to be 0
       
    snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, 0);
    snd_seq_set_client_name(seq, "GPIO MIDI Client");
    


    int port = snd_seq_create_simple_port(seq, "GPIO MIDI VIRTUAL Port",
            SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ | SND_SEQ_PORT_CAP_WRITE,
            SND_SEQ_PORT_TYPE_HARDWARE);
            
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_direct(&ev);
    
    /* either */
    //snd_seq_ev_set_dest(&ev, 129, 0); /* send to port manually */
    /* or */
    snd_seq_ev_set_subs(&ev);        /* send to subscribers of source port */
    

    

   if (gpioInitialise() < 0)
   {
      fprintf(stderr, "pigpio initialisation failed\n");
      return 1;
   }
	//specifying gpio to use for input switch; specifying polarity
    gpionote[0].gpio=4; 	gpionote[0].polarity = 0;
	gpionote[1].gpio=17; 	gpionote[1].polarity = 1; 
	gpionote[2].gpio=27;	//I got bored here and don't want to program note polarity
	gpionote[3].gpio=22;
	gpionote[4].gpio=5;
	gpionote[5].gpio=6;
	gpionote[6].gpio=13;
	gpionote[7].gpio=19;
	gpionote[8].gpio=26;
	gpionote[9].gpio=18;
	gpionote[10].gpio=23;
	gpionote[11].gpio=24;
	gpionote[12].gpio=25;
	//gpionote[13].gpio=12;		//reserving these for octave up
	//gpionote[14].gpio=16;		//reserving this for octave down
	//gpionote[15].gpio=20;		//spare
	//gpionote[16].gpio=21;		//interupt from mcp23017 for changes in inputs from previous value
	
	//AllocateNotes();
	
	//setting up gpio and populating default midi note number,gpio, and velocity
   int i;
   for(i=0; i<13; i++)
	{
		gpionote[i].notenum = notenum_start+i;
		gpionote[i].notevelocity = 80;			//max is 127
		gpioSetMode(gpionote[i].gpio, PI_INPUT);	//Set as input.
		gpioSetPullUpDown(gpionote[i].gpio, PI_PUD_DOWN);// Set as pull-down. 
		gpioGlitchFilter(gpionote[i].gpio,4000); //set 4ms debounce for gpioSetAlertFunc
		gpioSetAlertFunc(gpionote[i].gpio, ProcessMIDIEvent);
		 //printf("     notes : %d \n", i);
         //printf(" gpio is: %d \n", gpionote[i].gpio);
         //printf(" notenum is: %d \n", gpionote[i].notenum);
         //printf(" notevelocity is: %d\n\n",gpionote[i].notevelocity);
	}
	

	//setup octave up octave down button
	gpioSetMode(12, PI_INPUT);	//Set as input.
	gpioSetMode(16, PI_INPUT);	//Set as input.
	gpioSetPullUpDown(12, PI_PUD_DOWN);// Set as pull-down. 
	gpioSetPullUpDown(16, PI_PUD_DOWN);// Set as pull-down. 
	gpioGlitchFilter(12,20000); //set debounce for gpioSetAlertFunc
	gpioGlitchFilter(16,20000); //set debounce for gpioSetAlertFunc
	gpioSetAlertFunc(12, UpdateOctave);			// octave up button
	gpioSetAlertFunc(16, UpdateOctave);			// octave down button

	// set up MCP23017
	
	if ((fd = wiringPiI2CSetup (0x20)) < 0)
	{
    printf("wiringPiFindNode failed\n\r");}
    
    /*
	IOCON Bit Descriptions
	bit 7 BANK: Controls how the registers are addressed
		1 = The registers associated with each port are separated into different banks
		0 = The registers are in the same bank (addresses are sequential)
	bit 6 MIRROR: INT Pins Mirror bit
		1 = The INT pins are internally connected
		0 = The INT pins are not connected. INTA is associated with PortA and INTB is associated with PortB
	bit 5 SEQOP: Sequential Operation mode bit.
		1 = Sequential operation disabled, address pointer does not increment.
		0 = Sequential operation enabled, address pointer increments.
	bit 4 DISSLW: Slew Rate control bit for SDA output.
		1 = Slew rate disabled.
		0 = Slew rate enabled.
	bit 3 HAEN: Hardware Address Enable bit (MCP23S17 only).
		Address pins are always enabled on MCP23017.
		1 = Enables the MCP23S17 address pins.
		0 = Disables the MCP23S17 address pins.
	bit 2 ODR: This bit configures the INT pin as an open-drain output.
		1 = Open-drain output (overrides the INTPOL bit).
		0 = Active driver output (INTPOL bit sets the polarity).
	bit 1 INTPOL: This bit sets the polarity of the INT output pin.
		1 = Active-high.
		0 = Active-low.
	bit 0 Unimplemented: Read as ‘0’
	*/
    
	wiringPiI2CWriteReg8 (fd, MCP23x17_IOCON, 0x40) ;		//MCP23017 io configuration
	wiringPiI2CWriteReg8 (fd, MCP23x17_IODIRA, 0xff) ;		//set all to be inputs
	wiringPiI2CWriteReg8 (fd, MCP23x17_IODIRB, 0xff) ;		//set all to be inputs
	wiringPiI2CWriteReg8 (fd, MCP23x17_INTCONA, 0x00) ;		//compare with previous value
	wiringPiI2CWriteReg8 (fd, MCP23x17_INTCONB, 0x00) ;		//compare with previous value
	//wiringPiI2CWriteReg8 (fd, MCP23x17_DEFVALA, 0xff) ;		//set default value to be 1 for all (not used)
	//wiringPiI2CWriteReg8 (fd, MCP23x17_DEFVALB, 0xff) ;		//set default value to be 1 for all (not used)
	wiringPiI2CWriteReg8 (fd, MCP23x17_GPPUA, 0xff) ;		//set inputs for pull up 
	wiringPiI2CWriteReg8 (fd, MCP23x17_GPPUB, 0xff) ;		//set inputs for pull up
	wiringPiI2CWriteReg8 (fd, MCP23x17_GPINTENA, 0xff) ; 	//enable interupt on all pins
	wiringPiI2CWriteReg8 (fd, MCP23x17_GPINTENB, 0xff) ; 	//enable interupt on all pins
	wiringPiI2CReadReg8 (fd, MCP23x17_INTCAPA ); 			// clear interrupt flag	
	wiringPiI2CReadReg8 (fd, MCP23x17_INTCAPB ); 			// clear interrupt flag

	//setup masking array to obtain individual inputs from mcp23017
	for (i=0; i<8; i++){
		mask_in[i] = 1<<i;
		//printf("mask_in[%d]=0x%02x\n",i,mask_in[i]);	
	}
	
	//setup gpio 21 for MCP23017 for extra inputs
	//This works fine for reed switches but mechanical switch still bounce a lot.
	gpioSetMode(21, PI_INPUT);	//Set gpio 21 to receive interrupt from MCP23017
	gpioSetPullUpDown(21, PI_PUD_UP);// Set as pull-down. 
	gpioGlitchFilter(21,2000); //set debounce for all mcp23017 inputs 
	gpioSetAlertFunc(21, InteruptAlert);			// Alerting that there is something change in mcp23017 to update inputs
	//  or approximately every 5msec run InteruptAlert
	gpioSetWatchdog(21, 5);
	

	
	// call UpdateI2C_Polling every 10 milliseconds, in case it get stuck
	//gpioSetTimerFunc(0, 10, UpdateI2C_Polling);


    
    printf("MCP23x17_GPINTENA: 0x%02x \n", MCP23x17_GPINTENA);
    
    while (1)				
    { 
		sleep(200);		
		
	}
	

            
 return 0;
}

