#include <stdio.h>
#include <string.h>
 
struct midinote 
{
     int gpio;
     int notenum;
     int notevelocity;
};
 
int main() 
{
     int i;
     struct midinote gpionote[17];
      
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
	gpionote[13].gpio=12;
	gpionote[14].gpio=16;
	gpionote[15].gpio=20;
	gpionote[16].gpio=21;
	
	for(i=0; i<17; i++)
	{
		gpionote[i].notenum = 60+i;;
		gpionote[i].notevelocity = 127;
	}

 
     for(i=0; i<17; i++)
     {
		struct midinote currentnote = gpionote[i];
         printf("     notes : %d \n", i+1);
         printf(" gpio is: %d \n", gpionote[i].gpio);
         printf(" notenum is: %d \n", gpionote[i].notenum);
		printf(" notenum is: %d \n", currentnote.notenum);
         printf(" notevelocity is: %d\n\n",gpionote[i].notevelocity);
     }
     return 0;
}
