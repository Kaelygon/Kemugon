
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

/*
64KiB ram
16KiB bank size

16bit VAS sector ranges
0x0000 0x3FFF
0x4000 0x7FFF
0x8000 0xBFFF
0xC000 0xFFFF

System memory bank indices
rom 1
ram 2-5
vram 6-9
audio 10
Disk 16->
*/

int main() {
	//Virtual address space
	uint16_t *vas;

	uint16_t **ram = malloc(sizeof(uint16_t*)*4);
	uint16_t **disk = malloc(sizeof(uint16_t*)*4);

	for(int i=0; i<4; i++){
		ram[i] = malloc(sizeof(uint16_t)*16384);
		disk[i] = malloc(sizeof(uint16_t)*16384);
	}

	/*
		Initialize the emulator with VAS state
		ram0 | ram1 | ram2 | ram3 // vas[0x8000] points to ram[2][0]

		Then use bank switching to readdress VAS to
		ram0 | disk0 | disk1 | disk2  // vas[0x8000] points to disk[1][0]
	*/


	for(int i=0; i<4; i++){
		free(ram[i]);
		free(disk[i]);
	}

	free(ram);
	free(disk);
	return 0;
}
