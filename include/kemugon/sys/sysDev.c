/**
 * @file sysDev.c
 * 
 * @brief Implementation, functions that depend on sys.c and dev.c
 */


#include "kemugon/sys/sysDev.h"


typedef enum {
	//Load register
	LD0, LD1, LD2, LD3, LD4, LD5, LD6, LD7,
	//Store register
	ST0, ST1, ST2, ST3, ST4, ST5, ST6, ST7,

	JMP, //jump 
	TRM, //terminate
}KemuSys_ins;

//------ Running devices ------

/** @brief Run single device 
 * 
*/
void kemuDev_runCPU(KemuSys *sys, KemuDev *dev){
	KemuDev_CPU *cpu = dev->bank[0];

	#if KAEL_DEBUG
	//debug
	printf("word: %04X\n", SYS_VAS(cpu->pc));
	#endif

	//Load next word
	cpu->rw[0] = SYS_VAS(cpu->pc);
	cpu->pc++;

	//interpret as instruction
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
 * @brief Return nth occurance of given type
 * 
*/
KemuDev *kemuDev_devByType(KemuSys *sys, uint16_t devType, uint8_t n){
	uint8_t occurrences=0;
	uint8_t devCount = kaelTree_length(&sys->dev);
	for(uint8_t i=0; i<devCount; i++){
		KemuDev *currentDev = kaelTree_get(&sys->dev, i);
		if(currentDev->head.type==devType){
			if(occurrences==n){
				return currentDev;
			}
			occurrences++;
		}
	}
	return NULL;
}

/**
 * @brief Emulate all connected devices
 * TODO: Potential for multi-threading
 * 
*/
void kemuDev_run(KemuSys *sys){
	uint8_t devCount = kaelTree_length(&sys->dev);
	for(uint8_t i=0; i<devCount ; i++ ){
		KemuDev *currentDev = kaelTree_get(&sys->dev, i);
		if(currentDev==NULL){
			continue;
		}
		switch(currentDev->head.type){
			case CPU_DEV:
				kemuDev_runCPU(sys, currentDev);
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
	kemuDev_alloc(newDev);
	kaelTree_push(&sys->dev, newDev);
}

void kemuSys_initDevices(KemuSys *sys){
	//CPU
	KemuDev_head sysCPUHeader = {
		.bankSize	=	sizeof(KemuDev_CPU),
		.bankCount	=	1,
		.isBigEnd	=	0,
		.isVolatile	=	1,
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
		.isBigEnd	=	0,
		.isVolatile	=	0,
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
		.isBigEnd	=	0,
		.isVolatile	=	1,
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
		.isBigEnd	=	0,
		.isVolatile	=	0,
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