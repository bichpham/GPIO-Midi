/* testing

	* compile: gcc -o GPIO-MIDI GPIO-MIDI.c -lasound -lpigpio -lrt -lwiringPi
	* dedicated gpio (4, 17, 27, 22, 5, 6, 13, 19, 26,
	* 				 	18, 23, 24) - note gpio
	*								25 - cymbal channel 10
	*								7 - channel 11 - timpany roll
	*								8 - update velocity true = max velocity, false = med velocity
	* 						(, 12, 16)	octave up, down
	* 						gpio 20 - chord mode switch plays multiple notes
	* 						gpio 21 - interupt from MCP23017 I2C IO extension IC
	*						gpio 9, 10, 11 - not assigned (spi0 inputs)
	*						

	* Program channel conversion
	*	c code channel 0 = iSymphonic channel 1
	*to do
	*	1. add switch to maximize volume normal/max
	*	2. add add button to play timpany roll (done) (wire gpio 7 up)
	*	3. add button to play secondary channel

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

int main_midi_channel;

struct midinote 
{
     int gpio;
     int notenum;
	 int notechannel;
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

bool chord_mode_memory = 0; 	//variable to keep track of chord_mode to prevent stuck notes


void ProcessMIDIEvent(int gpio, int level, uint32_t tick)
{
	 printf("GPIO %d became %d at %d \n", gpio, level, tick);

	int i; 
	for(i=0; i<14; i++)
	{
		if ((gpio == gpionote[i].gpio) & level)						//button pressed
		{
			snd_seq_ev_set_noteon(&ev, gpionote[i].notechannel , gpionote[i].notenum, gpionote[i].notevelocity);
			printf(" notenum on: %d \n\n", gpionote[i].notenum);
			
			snd_seq_event_output(seq, &ev);
			
			//If chord mode switch is on then play the octave + the 5th note
			if (gpioRead(20))									//chord mode switch is on
			{
				snd_seq_ev_set_noteon(&ev, gpionote[i].notechannel, gpionote[i].notenum+12, gpionote[i].notevelocity);
				snd_seq_event_output(seq, &ev);
				snd_seq_ev_set_noteon(&ev, gpionote[i].notechannel, gpionote[i].notenum+19, gpionote[i].notevelocity);
				snd_seq_event_output(seq, &ev);
				printf(" chord mode midi command on \n");
				chord_mode_memory = 1; 				// chord mode turn off only if the corresponding note was turned off
			}
			
			snd_seq_drain_output(seq);
			
		}
		
		if ((gpio == gpionote[i].gpio) & !level)					//button released
		{
			snd_seq_ev_set_noteoff(&ev, gpionote[i].notechannel, gpionote[i].notenum, 64);
			printf(" notenum off: %d \n\n", gpionote[i].notenum);
			
			snd_seq_event_output(seq, &ev);
			
			if (gpioRead(20) || chord_mode_memory)									//chord mode switch is off
			{
				snd_seq_ev_set_noteoff(&ev, gpionote[i].notechannel, gpionote[i].notenum+12, gpionote[i].notevelocity);
				snd_seq_event_output(seq, &ev);
				snd_seq_ev_set_noteoff(&ev, gpionote[i].notechannel, gpionote[i].notenum+19, gpionote[i].notevelocity);
				snd_seq_event_output(seq, &ev);
				printf(" chord mode midi command off \n");
				chord_mode_memory = 0; // turning chord mode off when corresponding note is off
			}
			
			
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
	   for(i=0; i<12; i++)
		{
			gpionote[i].notenum = notenum_start+i;

		}
		printf(" notenum_start is: %d\n\n",notenum_start);
	}
   if (gpio==16 &&(notenum_start > 36) && level && !gpionotessactive)			//Octave down button
   
   {
	   
	   notenum_start = notenum_start-12;
	   for(i=0; i<12; i++)
		{
			gpionote[i].notenum = notenum_start+i;

		}
		printf(" notenum_start is: %d\n\n",notenum_start);
	}
}

void UpdateVelocity(int gpio, int level, uint32_t tick)					//gpio 8 true (max velocity), gpio 8 false (medium velocity)
{
	
   //repopulate velocity 
   int i;
   if (gpio==8 && level)			//Max velocity
   
	   for(i=0; i<12; i++)
		{
			gpionote[i].notevelocity = 126;

		}
   if (gpio==8 && !level)			//Medium velocity
   
	   for(i=0; i<12; i++)
		{
			gpionote[i].notevelocity = 80;

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
	
	
	main_midi_channel=4;					//set initial midi channel to be 5 (isymphonic)
       
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
    gpionote[0].gpio=4; 	gpionote[0].polarity = 0;		gpionote[0].notechannel = main_midi_channel;
	gpionote[1].gpio=17; 	gpionote[1].polarity = 0; 		gpionote[1].notechannel = main_midi_channel;
	gpionote[2].gpio=27;	gpionote[2].polarity = 0;		gpionote[2].notechannel = main_midi_channel;
	gpionote[3].gpio=22;	gpionote[3].polarity = 0;		gpionote[3].notechannel = main_midi_channel;
	gpionote[4].gpio=5;		gpionote[4].polarity = 0;		gpionote[4].notechannel = main_midi_channel;
	gpionote[5].gpio=6;		gpionote[5].polarity = 0;		gpionote[5].notechannel = main_midi_channel;
	gpionote[6].gpio=13;	gpionote[6].polarity = 0;		gpionote[6].notechannel = main_midi_channel;
	gpionote[7].gpio=19;	gpionote[7].polarity = 0;		gpionote[7].notechannel = main_midi_channel;
	gpionote[8].gpio=26;	gpionote[8].polarity = 0;		gpionote[8].notechannel = main_midi_channel;
	gpionote[9].gpio=18;	gpionote[9].polarity = 0;		gpionote[9].notechannel = main_midi_channel;
	gpionote[10].gpio=23;	gpionote[10].polarity = 0;		gpionote[10].notechannel = main_midi_channel;
	gpionote[11].gpio=24;	gpionote[11].polarity = 0;		gpionote[11].notechannel = main_midi_channel;
	gpionote[12].gpio=25;	gpionote[12].polarity = 0;		gpionote[12].notechannel = 9;				//cymbal channel 10 note E in symphonic percussion
	gpionote[13].gpio=7;	gpionote[13].polarity = 0;		gpionote[13].notechannel = 10;				//timpany roll channel 11 note E in symphonic percussion
	//gpionote[13].gpio=12;		//reserving these for octave up
	//gpionote[14].gpio=16;		//reserving this for octave down
	//gpionote[15].gpio=20;		//chord mode plays octave + 5
	//gpionote[16].gpio=21;		//interupt from mcp23017 for changes in inputs from previous value
	
	//AllocateNotes();
	
	//setting up gpio and populating default midi note number,gpio, and velocity
   int i;
   for(i=0; i<12; i++)
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
	
	//Setting up note 13th for cymbal crash
	
	gpionote[12].notenum = 40;			//E for cymbal crash in iSymphonic "Symphonic Percussion" sound set
	gpionote[12].notevelocity = 120;			//max is 127
	gpioSetMode(gpionote[12].gpio, PI_INPUT);	//Set as input.
	gpioSetPullUpDown(gpionote[12].gpio, PI_PUD_DOWN);// Set as pull-down. 
	gpioGlitchFilter(gpionote[12].gpio,4000); //set 4ms debounce for gpioSetAlertFunc
	gpioSetAlertFunc(gpionote[12].gpio, ProcessMIDIEvent);	
	
	//Setting up note 14th note for timpany roll
	
	gpionote[13].notenum = 40;			//E for cymbal crash in iSymphonic "Timpany Roll" sound set
	gpionote[13].notevelocity = 120;			//max is 127
	gpioSetMode(gpionote[13].gpio, PI_INPUT);	//Set as input.
	gpioSetPullUpDown(gpionote[13].gpio, PI_PUD_DOWN);// Set as pull-down. 
	gpioGlitchFilter(gpionote[13].gpio,4000); //set 4ms debounce for gpioSetAlertFunc
	gpioSetAlertFunc(gpionote[13].gpio, ProcessMIDIEvent);
	

	//setup octave up octave down button
	gpioSetMode(12, PI_INPUT);	//Set as input.
	gpioSetMode(16, PI_INPUT);	//Set as input.
	gpioSetPullUpDown(12, PI_PUD_DOWN);// Set as pull-down. 
	gpioSetPullUpDown(16, PI_PUD_DOWN);// Set as pull-down. 
	gpioGlitchFilter(12,20000); //set debounce for gpioSetAlertFunc
	gpioGlitchFilter(16,20000); //set debounce for gpioSetAlertFunc
	gpioSetAlertFunc(12, UpdateOctave);			// octave up button
	gpioSetAlertFunc(16, UpdateOctave);			// octave down button
	
	//setup update velocity
	gpioSetMode(8, PI_INPUT);	//Set as input. 
	gpioSetPullUpDown(8, PI_PUD_DOWN);// Set as pull-down. 
	gpioGlitchFilter(8,5000); //set debounce for gpioSetAlertFunc
	gpioSetAlertFunc(8, UpdateVelocity);			// octave down button
	
	//setup gpio 20 for chord mode
	gpioSetMode(20, PI_INPUT);	//Set as input.
	gpioSetPullUpDown(20, PI_PUD_DOWN);// Set as pull-down. 

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

