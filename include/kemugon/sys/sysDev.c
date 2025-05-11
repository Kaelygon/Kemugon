/**
 * @file sysDev.c
 * 
 * @brief Implementation, functions that depend on sys.c and dev.c
 */


#include "kemugon/sys/sysDev.h"

/**
 * @brief Return device by id
 * 
*/
KemuDev *kemuDev_devByID(const KemuSys *sys, const uint16_t devID){
	uint8_t devCount = kaelTree_length(&sys->dev);
	for(uint8_t i=0; i<devCount; i++){
		KemuDev *curDev = kaelTree_get(&sys->dev, i);
		if(curDev->devID == devID){
			return curDev;
		}
	}
	return NULL;
}

/**
 * @brief Return nth occurance of given type
 * 
*/
KemuDev *kemuDev_devByType(const KemuSys *sys, const uint16_t devType, uint8_t n){
	uint8_t count=0;
	uint8_t devCount = kaelTree_length(&sys->dev);
	for(uint8_t i=0; i<devCount; i++){
		KemuDev *curDev = kaelTree_get(&sys->dev, i);
		if(curDev->head.type==devType){
			if(count==n){
				return curDev;
			}
			count++;
		}
	}
	return NULL;
}

//------ Running devices ------

/** @brief Run single device 
 * 
*/
void kemuDev_runCPU(KemuSys *sys, KemuDev *dev){
	KemuDev_CPU *cpu = (void *)dev->bank[0];

	#if KAEL_DEBUG
	//debug
	printf("word: %04X\n", SYS_VAS(cpu->pc));
	#endif

	//Load next word
	cpu->rw[0] = SYS_VAS(cpu->pc);
	cpu->pc++;

	//interpret as instruction
	#include "kemugon/sys/instr.h"
	switch(cpu->rw[0]){ 
		case TRM:
			printf("Terminate instruction\n");
			sys->quitFlag=1;
			break;

		case JMP: //Jump to address in next word
			cpu->pc = SYS_VAS(cpu->pc);

		default:
	}

}

/**
 * @brief Emulate Memory Bank Controller
 * TODO: Read section of ram and repopulate pageTable accordingly
*/
void kemuDev_runMBC(KemuSys *sys){
}

/**
 * @brief Emulate connected special devices
 * TODO: Potential for multi-threading
 * 
*/
void kemuDev_run(KemuSys *sys){
	uint8_t devCount = kaelTree_length(&sys->dev);
	for(uint8_t i=0; i<devCount ; i++ ){
		KemuDev *curDev = kaelTree_get(&sys->dev, i);
		if(curDev==NULL){
			continue;
		}
		switch(curDev->head.type){
			
			case MBC_DEV:
				kemuDev_runMBC(sys);
				break;
				
			case CPU_DEV:
				kemuDev_runCPU(sys, curDev);
				break;
			
			case GPU_DEV:
				break;
			
			case AUDIO_DEV:
				break;

			default:
		}
	}
}



 /**
 * @brief Allocate and add devices to dev list
*/
void kemuSys_pushDev(KemuSys *sys, KemuDev *newDev){
	//Each device contains data which must be allocated and freed. 
	//Tree takes the memory ownership
	if( kemuDev_alloc(newDev) == KEMU_FAIL ){
		return;
	};

	//Generate device ID
	const uint8_t devCount = kaelTree_length(&sys->dev);
	
	//List all used IDs
	uint8_t usedList[devCount];
	for(uint16_t i=0; i<devCount; i++){
		KemuDev *curDev = kaelTree_get(&sys->dev, i);
		usedList[i] = curDev->devID;
	}

	//Find first unused ID
	for (uint16_t i=1; i!=0; i++) {
		uint8_t isUsed = 0;
		for (uint8_t j=0; j<devCount; j++) {
			if (usedList[j] == i) {
				isUsed = 1;
				break;
			}
		}
		if (!isUsed) {
				newDev->devID = i;
		break;
		}
	}

	kaelTree_push(&sys->dev, newDev);
}

void kemuSys_initDevices(KemuSys *sys){
	//CPU
	KemuDev_head sysCPUHeader = {
		.bankSize	=	sizeof(KemuDev_CPU),
		.bankCount	=	1,
		.isROM		=	0,
		.type			=	CPU_DEV,
	};
	KemuDev sysCPU = {
		.path = NULL,
		.fd = -1,
		.head = sysCPUHeader,
	};


	//ROM
	KemuDev_head sysROMHeader = {
		.bankSize	=	4*1024,
		.bankCount	=	1,
		.isROM		=	1,
		.type			=	DATA_DEV,
	};
	KemuDev sysROM = {
		.path = "disk/rom.img",
		.fd = -1,
		.head = sysROMHeader,
	};


	//RAM
	KemuDev_head sysRAMHeader = {
		.bankSize	=	16*1024,
		.bankCount	=	4,
		.isROM		=	0,
		.type			=	RAM_DEV,
	};
	KemuDev sysRAM = {
		.path = NULL,
		.fd = -1,
		.head = sysRAMHeader,
	};


	//Disk
	KemuDev_head dataDiskHeader = {
		.bankSize	=	16*1024,
		.bankCount	=	16,
		.isROM		=	0,
		.type			=	DATA_DEV,
	};
	KemuDev dataDisk = {
		.path = "disk/disk.img",
		.fd = -1,
		.head = dataDiskHeader,
	};

	kemuSys_pushDev(sys, &sysCPU);
	kemuSys_pushDev(sys, &sysRAM);
	kemuSys_pushDev(sys, &sysROM);
	kemuSys_pushDev(sys, &dataDisk);
}