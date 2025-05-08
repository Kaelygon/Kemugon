/**
 * @file core.c
 * 
 * @brief core for u16 emulator
 */

#include <stdlib.h>
#include <stdint.h>

#include "kaelygon/disk/disk.h"
#include "kaelygon/sys/sys.h"
#include "kaelygon/global/kaelMacros.h"

int main(){
	#if KAEL_DEBUG==1
		kaelDebug_allocGlobal();
	#endif


	//Initialize disk
	KemuDisk disk = {
		.path = "disk/disk.img",
		.fd = -1,
		.bankSize = 16*1024,
		.bankCount = 16,
	};

	//Initialize system
	KemuSys system = {
		.stackSize = 256,
		.bankSize = 16*1024,
		.vasBankCount = 4,
		.firstDiskBank = 32,

		.bankListSize = 32,
	};

	kemuDisk_alloc(&disk);
	kemuSys_alloc(&system);

	kemuSys_boot(&system, &disk);

	kemuSys_loop(&system);
	
	//Cleanup
	kemuSys_free(&system);
	kemuDisk_free(&disk);
	
	#if KAEL_DEBUG==1
		printf("\n");
		kaelDebug_printInfoStr();
		kaelDebug_freeGlobal();
	#endif

	return 0;
}


