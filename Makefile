all:
	@gcc -Wall -pthread -o GPIO-MIDI GPIO-MIDI.c -lasound -lpigpio -lrt -lwiringPi

clean:
	@rm GPIO-MIDI