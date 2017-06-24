/*

	* compile: gcc -o GPIO-MIDI GPIO-MIDI.c -lasound -lpigpio -lrt
	* dedicated gpio (4, 17, 27, 22, 5, 6, 13, 19, 26,
	* 				 	18, 23, 24, 25) - note gpio
	* 						(, 12, 16, 20, 21)	octave up, down, channel up, down pb


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

void ProcessMIDIEvent(int gpio, int level, uint32_t tick)
{
	 printf("GPIO %d became %d at %d \n", gpio, level, tick);

	int i; 
	for(i=0; i<17; i++)
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


int main()
{
	
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
    

   if (gpioInitialise() < 0)
   {
      fprintf(stderr, "pigpio initialisation failed\n");
      return 1;
   }
   
   //setting up gpio and populating midi note number,gpio, and velocity
   int i;
   for(i=0; i<17; i++)
	{
		gpionote[i].notenum = 36+i;
		gpionote[i].notevelocity = 80;			//max is 127
		gpioSetMode(gpionote[i].gpio, PI_INPUT);	//Set as input.
		gpioSetPullUpDown(gpionote[i].gpio, PI_PUD_DOWN);// Sets as pull-down. 
		gpioGlitchFilter(gpionote[i].gpio,4000); //set 4ms debounce for gpioSetAlertFunc
		gpioSetAlertFunc(gpionote[i].gpio, ProcessMIDIEvent);
		 printf("     notes : %d \n", i);
         printf(" gpio is: %d \n", gpionote[i].gpio);
         printf(" notenum is: %d \n", gpionote[i].notenum);
         printf(" notevelocity is: %d\n\n",gpionote[i].notevelocity);
	}
    
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

