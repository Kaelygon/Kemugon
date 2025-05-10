/**
 * @file sys.c
 * 
 * @brief Implementation, u16 emulator virtual hardware
 */

#include "kemugon/sys/sys.h"
#include "kemugon/sys/sysDev.h"

 /**
 * @brief System configuration addresses in VAS
*/
typedef enum {
	PAGE_TABLE_ADDR	= 1,
	DATA_ADDR			= 0x0200, //End of system reserved in bank0
}KemuSys_addr;

/**
 * @brief Returns direct active bank address by index
 */
void *kemuSys_getPageTablePtr(const KaelTree *pageTable, uint16_t index){
	/*
		Raw data is stored in (void *)dev->data, this data is split into list of bank pointers (void**)dev->bank 
		pageTable holds pointers of banks, kaelTree_get returns pageTable's pointer by index

		(void*)kaelTree_get(pageTable, index) -> (void*)pageTable[index] -> (void**)dev->bank[N] -> (void*)dev->data + (uint8_t*)N*bankSize
	*/
	return *(void**)kaelTree_get(pageTable, index);
}

//------ Virtual Address Space Macro ------
static uint16_t kemuSys_zeroBank = 0; //Logically disconnected bank

/**
 * @brief Convert pageTable banks addresses into one 16-bit address
 * Since we don't know anything about the banks, sys->pageTableSize determines how much of the bank is allocated
*/
uint16_t* kemuSys_resolveVAS(KemuSys *sys, uint16_t addr) {
	if(sys==NULL){//Invalid state
		return &kemuSys_zeroBank;
	}
	uint16_t bankSize = (UINT16_MAX/sys->pageTableSize);
	uint16_t tableIndex = addr/bankSize;
	uint16_t subAddr = addr - bankSize*tableIndex;

	uint16_t *bank = kemuSys_getPageTablePtr(&sys->pageTable, tableIndex);
	return bank!=NULL ? &bank[subAddr] : &kemuSys_zeroBank;
}

//------ System ------

/**
 * @brief Allocate emulated system to host memory
*/
void kemuSys_alloc(KemuSys *sys){
	kaelTree_alloc(&sys->pageTable, sizeof(void*)); 
	kaelTree_resize(&sys->pageTable, 16); 

	kaelTree_alloc(&sys->dev, sizeof(KemuDev));
}

/**
 * @brief Free emulated memory
*/
void kemuSys_free(KemuSys *sys){
	//Free devices and pageTable
	while( !kaelTree_empty(&sys->dev) ){
		KemuDev *lastDev = (KemuDev*)kaelTree_back(&sys->dev);
		kemuDev_free(lastDev);
		kaelTree_pop(&sys->dev);
	}

	//Free trees
	kaelTree_free(&sys->pageTable);
	kaelTree_free(&sys->dev);
}


/**
 * @brief Find and address RAM_DEV and DATA_DEV to VAS
 * Lower index is prioritized
 * 
*/
uint8_t kemuSys_bootload(KemuSys *sys){
	uint8_t devCount = kaelTree_length(&sys->dev);
	if(devCount<3){
		//Not enough devices. Minimum = CPU, RAM, ROM
		return KEMU_FAIL;
	}
	
	//Find first RAM and data device
	KemuDev *ramDev = kemuDev_devByType(sys, RAM_DEV, 0);
	KemuDev *romDev = kemuDev_devByType(sys, DATA_DEV, 0);

	//Add 0th banks to page table
	if(ramDev!=NULL && romDev!=NULL){
		kaelTree_set(&sys->pageTable, 0, &ramDev->bank[0] );
		kaelTree_set(&sys->pageTable, 1, &romDev->bank[0] );

		#if KAEL_DEBUG
			//Pointer sanity check
			if((void*)ramDev->bank[0] != kemuSys_getPageTablePtr(&sys->pageTable, 0) ||
				(void*)romDev->bank[0] != kemuSys_getPageTablePtr(&sys->pageTable, 1)
			 ){
				printf("pageTable pointer copy failed.\n");
				return KEMU_FAIL;
			 }
		#endif

		return KEMU_SUCCESS;
	}
	return KEMU_FAIL;
}


/**
 * @brief Emulate Memory Bank Controller
 * TODO: Read section of ram and repopulate pageTable accordingly
*/
void kemuSys_MBC(KemuSys *sys){
}


/**
 * @brief Initialize system state
 * 
*/
void kemuSys_boot(KemuSys *sys){
	//Fetch CPU and set program counter
	KemuDev *cpu = kemuDev_devByType(sys, CPU_DEV, 0);
	if( NULL_CHECK(cpu) || NULL_CHECK(cpu->bank[0]) ){
		printf("CPU Fail\n");
		return;
	}
	KemuDev_CPU *cpuReg = cpu->bank[0];
	cpuReg->pc = DATA_ADDR;

	if( kemuSys_bootload(sys) == KEMU_FAIL ){
		printf("Failed to fetch RAM_DEV and DATA_DEV\n");
		return;
	}

	//TODO: write bootloader program to rom that's run by cpu

	kemuSys_MBC(sys);
}


/**
 * @brief Execute instructions from ram and increment pc by word 
 * 
*/
void kemuSys_loop(KemuSys *sys){
	uint64_t cycleCount = 0;

	KemuClock clock;
	kemuClock_init(&clock, sys->hostClockSpeed, sys->emuClockSpeed);
	
	while(!sys->quitFlag){
		kemuDev_run(sys);
		kemuClock_sync(&clock);
		cycleCount++;

		if (cycleCount >= 16) {
			sys->quitFlag = 1;
		}
	}
}

