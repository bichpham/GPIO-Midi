/*

	* compile: gcc -o GPIO-MIDI GPIO-MIDI.c -lasound -lpigpio -lrt -lwiringPi
	* dedicated gpio (4, 17, 27, 22, 5, 6, 13, 19, 26,
	* 				 	18, 23, 24, 25) - note gpio
	* 						(, 12, 16)	octave up, down
	* 						gpio 20 - spare
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
};

struct midinote gpionote[17];		//Create array to store 17 gpio and 17 note 

int notenum_start = 36;		//default start note is c for pedal

bool gpionotessactive = 0;	//detecting whether gpio inputs for notes are active or not


void ProcessMIDIEvent(int gpio, int level, uint32_t tick)
{
	 printf("GPIO %d became %d at %d \n", gpio, level, tick);

	int i; 
	for(i=0; i<13; i++)
	{
		if ((gpio == gpionote[i].gpio) & level)
		{
			snd_seq_ev_set_noteon(&ev, midi_channel, gpionote[i].notenum, gpionote[i].notevelocity);
			printf(" notenum on: %d \n\n", gpionote[i].notenum);
			snd_seq_event_output(seq, &ev);
			snd_seq_drain_output(seq);
			
		}
		
		if ((gpio == gpionote[i].gpio) & !level)
		{
			snd_seq_ev_set_noteoff(&ev, midi_channel, gpionote[i].notenum, 64);
			printf(" notenum off: %d \n\n", gpionote[i].notenum);
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

void UpdateI2C(int gpio, int level, uint32_t tick)
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
	//specifying gpio to use for input switch
    gpionote[0].gpio=4;
	gpionote[1].gpio=17;
	gpionote[2].gpio=27;
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
	//gpionote[15].gpio=20;		//midi channel up pb
	//gpionote[16].gpio=21;		//midi channel down pb
	
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
	
	
	
	//setup pin 21 for MCP23017 for extra inputs
	gpioSetMode(21, PI_INPUT);	//Set gpio 21 to receive interrupt from MCP23017
	gpioSetPullUpDown(21, PI_PUD_DOWN);// Set as pull-down. 
	gpioGlitchFilter(21,4000); //set single debounce for all inputs from MCP23107
	
	wiringPiSetup () ;
	//mcp23017Setup (100, 0x20) ;
	
	// set up MCP23017
	int fd ;
	
	if ((fd = wiringPiI2CSetup (0x20)) < 0)
	{
    printf("wiringPiFindNode failed\n\r");}
    
    //setup bank B
	wiringPiI2CWriteReg8 (fd, MCP23x17_IOCON, 0x00) ;		//MCP23017 io configuration
	wiringPiI2CWriteReg8 (fd, MCP23x17_IODIRB, 0xff) ;		//set all to be inputs
	wiringPiI2CWriteReg8 (fd, MCP23x17_INTCONB, 0xff) ;		//set all pins for interupt
	wiringPiI2CWriteReg8 (fd, MCP23x17_DEFVALB, 0xff) ;		//set default value to be 1 for all
	wiringPiI2CWriteReg8 (fd, MCP23x17_GPINTENB, 0xff) ; 	//enable interupt on all pins
	wiringPiI2CWriteReg8 (fd, MCP23x17_GPPUB, 0xff) ;		//set inputs for pull up 
	wiringPiI2CReadReg8 (fd, MCP23x17_INTCAPB ); 			// clear interrupt flag
	/*
	//setup interrupt
	//wiringPiI2CWriteReg16 (0x20, 0x02, 0xFF) ;		//setup interrupt
	//wiringPiI2CWriteReg16 (0x20, 0x04, 0xFF) ;		//setup interrupt
	
	//set everything at address 20 to be inputs and pulled up.
	for(i=0; i<16; i++)
	{
		pinMode (100 + i, INPUT);
		pullUpDnControl (100 + i, PUD_UP) ;
	}
	
	gpioSetAlertFunc(21, UpdateI2C);			// read I2C inputs if there are changes from IC
	
	*/
    
    printf("MCP23x17_GPINTENA: 0x%02x \n", MCP23x17_GPINTENA);
    //printf("mcp23017Setup: %d \n", mcp23017Setup (100, 0x20));
    while (true)
    { 
		sleep(2);		

			/*
		for(i=0; i<17; i++)
		{
			gpioSetPullUpDown(gpionote[i].gpio, PI_PUD_UP);
			sleep(1);
			gpioSetPullUpDown(gpionote[i].gpio, PI_PUD_DOWN);
			sleep(1);
		}*/
		
		
	}
    
            
 return 0;
}

