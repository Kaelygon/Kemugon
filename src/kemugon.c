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
		.pageSize = 256U,
	};
	kemuSys_alloc(&system);

	kemuSys_initDevices(&system);

	uint8_t err = kemuSys_boot(&system);
	if(err!=KEMU_FAIL){
		kemuSys_loop(&system);
	}

	kemuSys_free(&system);

	return 0;
}


