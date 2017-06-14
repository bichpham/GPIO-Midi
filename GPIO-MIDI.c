/*
To send some events at a specific time:

	-open the sequencer;
	-create your own (source) port;
	-construct and send some events.
	* 
	* compile: gcc -o GPIO-MIDI GPIO-MIDI.c -lasound


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

#define MAX_SENDERS 16



int main()
{
	

       

/*
To open the sequencer, call snd_seq_open. (You can get your client 
* number with snd_seq_client_id.)

*/

    snd_seq_t *seq;
    snd_seq_open(&seq, "default", SND_SEQ_OPEN_DUPLEX, 0);
    snd_seq_set_client_name(seq, "GPIO MIDI Client");
    
/*
To create a port, allocate a port info object with snd_seq_port_info_alloca, 
* set port parameters with snd_seq_port_info_set_xxx, and call snd_seq_create_port. 
* Or simply call snd_seq_create_simple_port.

*/
    
    
    int port;
    port = snd_seq_create_simple_port(seq, "GPIO MIDI VIRTUAL Port",
            SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ | SND_SEQ_PORT_CAP_WRITE,
            SND_SEQ_PORT_TYPE_HARDWARE);
            
     snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    snd_seq_ev_set_direct(&ev);
    
/*
To send an event, allocate an event structure (just for a change, you can use a 
* local snd_seq_event_t variable), and call various snd_seq_ev_xxx functions to 
* set its properties. Then call snd_seq_event_output, and snd_seq_drain_output after 
* you've sent all events.


*/
    

    /* either */
    //snd_seq_ev_set_dest(&ev, 129, 0); /* send to 64:0 */
    /* or */
    snd_seq_ev_set_subs(&ev);        /* send to subscribers of source port */

    //snd_seq_ev_set_noteon(&ev, 0, 60, 127);
    //snd_seq_event_output(seq, &ev);

    //snd_seq_ev_set_noteon(&ev, 0, 67, 127);
    //snd_seq_event_output(seq, &ev);

    //snd_seq_drain_output(seq);   
    
   if (gpioInitialise() < 0)
   {
      fprintf(stderr, "pigpio initialisation failed\n");
      return 1;
   }
   gpioSetMode(17, PI_INPUT);  // Set GPIO17 as input.
   gpioSetPullUpDown(17, PI_PUD_DOWN); // Sets a pull-down. 
   int testpin;
    
    while (true)
    { 
		/*sleep(5);
		snd_seq_ev_set_noteon(&ev, 0, 60, 127);
		snd_seq_event_output(seq, &ev);
		

		snd_seq_drain_output(seq);
		
		sleep(5);
		snd_seq_ev_set_noteoff(&ev, 0, 60, 127);
		snd_seq_event_output(seq, &ev);
		
		snd_seq_drain_output(seq);*/
		
		if (gpioRead(17)==1)
		{
			snd_seq_ev_set_noteon(&ev, 0, 60, 127);
			snd_seq_event_output(seq, &ev);
			snd_seq_drain_output(seq);
		}
		if (gpioRead(17) ==0)
		{
			snd_seq_ev_set_noteoff(&ev, 0, 60, 127);
			snd_seq_event_output(seq, &ev);
		
			snd_seq_drain_output(seq);
		}
		
		//sleep(2);
		//testpin = gpioRead(17);
		//printf("GPIO17 is level %d \n", gpioRead(17));
		
	}     
    
            
 return 0;
}
