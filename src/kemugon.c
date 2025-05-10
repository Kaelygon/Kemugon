/**
 * @file core.c
 * 
 * @brief core for u16 emulator
 */

#include <stdlib.h>
#include <stdint.h>

#include "kemugon/sys/sys.h"
#include "kemugon/dev/dev.h"
#include "kemugon/sys/sysDev.h"
#include "libkael/debug/kaelMacros.h"


int main(){
	KemuSys system = {
		.emuClockSpeed  = 4194304U,
		.hostClockSpeed = 3700003502U,
		.pageTableSize = 4,
		.pageTable = {0},
		.dev = {0},
		.quitFlag = 0,
	};
	kemuSys_alloc(&system);

	kemuSys_initDevices(&system);

	kemuSys_boot(&system);
	kemuSys_loop(&system);
	
	//Cleanup
	kemuSys_free(&system);

	return 0;
}


