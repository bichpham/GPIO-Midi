all:
	@gcc -o GPIO-MIDI GPIO-MIDI.c -lasound

clean:
	@rm GPIO-MIDI