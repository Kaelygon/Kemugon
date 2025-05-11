/**
 * @file sys.c
 * 
 * @brief Implementation, u16 emulator virtual hardware
 */

#include "kemugon/sys/sys.h"
#include "kemugon/sys/sysDev.h"

//------ Virtual Address Space Macro ------
static uint16_t kemuSys_nullBank[256] = {0}; //Logically disconnected bank

/**
 * @brief Convert pageTable banks addresses into one 16-bit address
 * 
*/
uint16_t* kemuSys_resolveVAS(const KemuSys *sys, const uint16_t addr) {
	if(NULL_CHECK(sys)){
		return &kemuSys_nullBank[0];
	}
	uint16_t frameIndex = addr / sys->pageSize;
	uint16_t *frame = sys->frameTable[frameIndex];
	uint16_t subAddr = addr - frameIndex * sys->pageSize;
	uint16_t *element = &frame[subAddr]; //Pointer directly to dev->rawData element
	return element;
}

//Called if sys->pageTable is modified
void kemuSys_mapFrameTable(const KemuSys *sys) {
	for(uint16_t i=0; i<sys->mapPageCount; i++){
		KemuSys_pageEntry entry = sys->pageTable[i];
		if(entry.devID==0){ //null terminated
			break;
		}
		//Get device
		KemuDev *curDev = kemuDev_devByID(sys, entry.devID);
		uint16_t bankCount = (entry.lastBank - entry.firstBank + 1);
		uint16_t devStart	= curDev->head.bankSize * entry.firstBank;
		uint16_t devEnd 	= curDev->head.bankSize * bankCount + devStart;

		//Create pointers to rawData in sys->pageSize sized chunks
		uint16_t entryPageCount = (devEnd - devStart)/sys->pageSize;
		entryPageCount = kaelMath_min(entryPageCount,sys->mapPageCount);
		for(uint16_t j=0; j<entryPageCount; j++){
			size_t offset = j * sys->pageSize *  sizeof(uint16_t);
			sys->frameTable[entry.pageIndex + j] = curDev->data + offset;
		}
	}
}

//------ System ------


uint16_t kemuSys_u8Pack(uint8_t hi, uint8_t lo){
	return ((uint16_t)hi << 8U) | (lo & 0xFF);
}

/**
 * @brief Allocate emulated system to host memory
*/
void kemuSys_alloc(KemuSys *sys){
	sys->mapPageCount = ((UINT16_MAX+1)/sys->pageSize);
	sys->frameTable = calloc(sys->mapPageCount,sizeof(uint16_t*));
	for (int i = 0; i < 256; ++i) {
		sys->frameTable[i] = kemuSys_nullBank;
	}

	uint16_t pageCount = (UINT16_MAX+1)/sys->pageSize;
	sys->pageTable = calloc(pageCount,sizeof(KemuSys_pageEntry));
	kaelTree_alloc(&sys->dev, sizeof(KemuDev));
}

/**
 * @brief Free emulated system
*/
void kemuSys_free(KemuSys *sys){
	//Free devices and pageTable
	while( !kaelTree_empty(&sys->dev) ){
		KemuDev *lastDev = (KemuDev*)kaelTree_back(&sys->dev);
		kemuDev_free(lastDev);
		kaelTree_pop(&sys->dev);
	}

	free(sys->frameTable);
	free(sys->pageTable);
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

	KemuSys_pageEntry ramEntry = {
		.devID	= ramDev->devID,
		.pageIndex	= 0,
		.firstBank	= 0,
		.lastBank	= 0,
	};
	KemuSys_pageEntry romEntry = {
		.devID	= romDev->devID,
		.pageIndex	= 64,
		.firstBank	= 0,
		.lastBank	= 0,
	};

	sys->pageTable[0] = ramEntry;
	sys->pageTable[1] = romEntry;

	kemuSys_mapFrameTable(sys);

	return KEMU_SUCCESS;
}

/**
 * @brief Initialize system state
 * 
*/
uint8_t kemuSys_boot(KemuSys *sys){
	//Fetch CPU and set program counter
	KemuDev *cpu = kemuDev_devByType(sys, CPU_DEV, 0);
	if( NULL_CHECK(cpu) || NULL_CHECK(cpu->bank[0]) ){
		printf("CPU Fail\n");
		return KEMU_FAIL;
	}
	KemuDev_CPU *cpuReg = (void *)cpu->bank[0];
	cpuReg->pc = BOOT_ADDR;

	if( kemuSys_bootload(sys) == KEMU_FAIL ){
		printf("Failed to fetch RAM_DEV and DATA_DEV\n");
		return KEMU_FAIL;
	}

	//MBC Test program
	KemuDev *ramDev = kemuDev_devByType(sys, RAM_DEV, 0);
	KemuDev *dataDev = kemuDev_devByType(sys, DATA_DEV, 1);

	//reference
	KemuSys_pageEntry ramEntry = {
		.devID	= ramDev->devID,
		.pageIndex	= 0,
		.firstBank	= 0,
		.lastBank	= 1,
	};
	KemuSys_pageEntry romEntry = {
		.devID	= dataDev->devID,
		.pageIndex	= 128,
		.firstBank	= 0,
		.lastBank	= 1,
	};

	//Write the two structs 
	SYS_VAS(MBC_FLAG_ADDR) = ADD_MBC;
	SYS_VAS(PAGE_TABLE_ADDR+0x0000) = kemuSys_u8Pack( 0, 	ramDev->devID  	); //ramDev to page 0
	SYS_VAS(PAGE_TABLE_ADDR+0x0001) = kemuSys_u8Pack( 1, 	0				 		); //Banks 0 to 1
	SYS_VAS(PAGE_TABLE_ADDR+0x0002) = kemuSys_u8Pack( 128, dataDev->devID	); //dataDev to page 128
	SYS_VAS(PAGE_TABLE_ADDR+0x0003) = kemuSys_u8Pack( 1, 	0				 		); //Banks 0 to 1

	{
	//TODO: Implement these CPU instructions and run the program. Verfiy that written values are identical to above. 
	//
	#include "kemugon/sys/instr.h"
		uint16_t loader[] = {
			//Write MBC Add flag
			LD, R0, ADD_MBC,
			ST, R0, MBC_FLAG_ADDR,

			//Pack ram devID and page index 
			LD, R0, ramDev->devID,
			LD, R1, 0,
			LD, R2, PAGE_TABLE_ADDR,
			ADD, R2, 0,

			//R2 = R0<<8 | R1
			SHL, R0, 8,
			OR, R0, R1,
			ST, R2, R0,

			//Pack ram bank range 
			LD, R0, 0,
			LD, R1, 1,
			LD, R2, PAGE_TABLE_ADDR,
			ADD, R2, 1,

			//R2 = R0<<8 | R1
			SHL, R0, 8,
			OR, R0, R1,
			ST, R2, R0,
			
			TRM,
		};
		
		for(uint16_t i=0; i<sizeof(loader)/sizeof(uint16_t); i++){
			SYS_VAS(BOOT_ADDR+i) = loader[i];
		}
		
	}
/*	
	int ramCmp = memcmp(&ramEntry, (KemuSys_pageEntry *)&SYS_VAS(PAGE_TABLE_ADDR+0x0000), sizeof(KemuSys_pageEntry) );
	int romCmp = memcmp(&romEntry, (KemuSys_pageEntry *)&SYS_VAS(PAGE_TABLE_ADDR+0x0002), sizeof(KemuSys_pageEntry) );
	
	printf("ramCmp %d\n",ramCmp );
	printf("romCmp %d\n",romCmp );

	int i = 1;
	
	if (*((char *)&i) == 1) puts("little endian");
	else puts("big endian");

	printf("Packed value: 0x%04X\n", kemuSys_u8Pack(0x12,0x34));
*/
	return KEMU_SUCCESS;
}


/**
 * @brief Emulate devices synced to system clock
 * 
*/
void kemuSys_loop(KemuSys *sys){
	sys->quitFlag = 0;
	uint64_t cycleCount = 0;

	KemuClock clock;
	kemuClock_init(&clock, sys->hostClockSpeed, sys->emuClockSpeed);
	
	while(!sys->quitFlag){
		kemuDev_run(sys); 
		kemuClock_sync(&clock);
		cycleCount++;

		//debug terminate
		if (cycleCount >= sys->emuClockSpeed) {
			sys->quitFlag = 1;
		}
	}
}

