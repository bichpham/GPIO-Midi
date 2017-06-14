all:
	@gcc -Wall -pthread -o GPIO-MIDI GPIO-MIDI.c -lasound -lpigpio -lrt

clean:
	@rm GPIO-MIDI