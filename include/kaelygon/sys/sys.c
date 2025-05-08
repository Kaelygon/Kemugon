/**
 * @file math.h
 * 
 * @brief Implementation, u16 emulator virtual hardware
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "kaelygon/global/kaelMacros.h"

#include "kaelygon/disk/disk.h"
#include "kaelygon/sys/sys.h"


/*
	Proposed architecture

	Separate video and audio acceleration

	64KiB ram
	64KiB vram
	16KiB bank size

	16bit VAS sector ranges
	0x0000 0x3FFF
	0x4000 0x7FFF
	0x8000 0xBFFF
	0xC000 0xFFFF

*/

//------ Virtual Address Space Macro ------
static uint16_t kemuSys_zeroBank = 0; //Logically disconnected bank

/**
 * @brief Convert pageTable banks addresses into one 16-bit address
 * 
*/
uint16_t* kemuSys_resolveVAS(uint16_t **pageTable, uint16_t addr) {
	uint16_t bankIndex = addr >> 14;
	uint16_t offset = addr & 0x3FFF;
	uint16_t *bank = pageTable[bankIndex];
	return bank!=NULL ? &bank[offset] : &kemuSys_zeroBank;
}


//------ System ------

/**
 * @brief System work memory layout
 * 
*/
typedef enum {
	/**
		bank 1 System work ram
		NULL 0x0000
		Disk type 0x0001
		Stack 0x0100-0x01FF
		4 banks, page table addresses 0x0200-0x0203
		
		Unused space in bank 1 and ram banks 2,3,4 can be used as program ram
	*/
	DISK_TYPE_ADDR = 0x0001,
	STACK_ADDR = 0x0100,
	PAGE_TABLE_ADDR = 0x0200,
}KemuSys_addr;


/**
 * @brief Memory bank controller indices
 * 
*/
typedef enum {
/*
	Some indices are unused up till (sys->firstDiskBank - 1)
	Disk banks start at index sys->firstDiskBank
	0xFFFF at PAGE_TABLE_ADDR+(N * sys->vasBankCount) means no change on Nth paged bank
*/
	NULL_BANK,
	ROM_BANK,
	RAM0_BANK, RAM1_BANK, RAM2_BANK, RAM3_BANK,
	VRAM0_BANK, VRAM1_BANK, VRAM2_BANK, VRAM3_BANK,
	AUDIO_BANK,
	UNCHANGED_BANK = 0xFFFF
}KemuSys_bankIndex;

typedef enum {
	//Load register
	LD0, LD1, LD2, LD3, LD4, LD5, LD6, LD7,

	//Store register
	ST0, ST1, ST2, ST3, ST4, ST5, ST6, ST7,

	JMP, //jump 
	TRM, //terminate
}KemuSys_ins;

/**
 * @brief Allocate emulated system to host memory
 * 
*/
void kemuSys_alloc(KemuSys *sys){
	for(uint64_t i=0; i<sys->vasBankCount; i++){
		sys->ram[i]	= malloc( sys->bankSize * sizeof(uint16_t) );
		KAEL_ASSERT(sys->ram[i]!=NULL);
	}
	sys->bankList = malloc( sys->firstDiskBank * sizeof(void*) );
	KAEL_ASSERT(sys->bankList!=NULL);
}

/**
 * @brief Free emulated memory
 * 
*/
void kemuSys_free(KemuSys *sys){
	for(uint64_t i=0; i<sys->vasBankCount; i++){
		free(sys->ram[i]);
		sys->ram[i] = NULL;
	}
	free(sys->bankList);
}

/**
 * @brief Create memory bank list, some aren't implemented
 * 
*/
void kemuSys_addressBanks(KemuSys *sys, KemuDisk *disk){

	//Resize bankList now when we know the disk size
	sys->bankListSize = sys->firstDiskBank + disk->bankCount;
	sys->bankList = realloc( sys->bankList, sys->bankListSize * sizeof(void*) );
	KAEL_ASSERT(sys->bankList!=NULL);
	memset(sys->bankList, 0, sys->bankListSize * sizeof(void*));

	//NULL = logically disconnected bank
	sys->bankList[0] = NULL;

	//Address ram banks
	for(uint64_t i=0; i < sys->vasBankCount; i++){
		sys->bankList[RAM0_BANK+i] = sys->ram[i];
	}

	//Address disk banks
	for(uint64_t i=0; i < disk->bankCount; i++){
		if(NULL_CHECK(disk->bank[i])){
			continue;
		}
		sys->bankList[sys->firstDiskBank+i] = disk->bank[i];
	}
}

/**
 * @brief Emulate Memory Bank Controller
 * 
*/
void kemuSys_MBC(KemuSys *sys){

	//if PAGE_TABLE_ADDR is not UNCHANGED_BANK, switch bank to the set value
	for(size_t i=0; i < sys->vasBankCount; i++){
		uint16_t bankIndex = SYS_VAS(PAGE_TABLE_ADDR+i);

		if( bankIndex==UNCHANGED_BANK ){
			continue;
		}
		//NULL is valid pageTable value
		uint16_t* bankPtr = bankIndex < sys->bankListSize ? sys->bankList[bankIndex] : NULL;
		sys->pageTable[i] = bankPtr;

		SYS_VAS(PAGE_TABLE_ADDR+i)=UNCHANGED_BANK;
	}
}

/**
 * @brief Since ROM isn't implemented, we write a test program into ram
 * 
*/
void kemuSys_bootload(KemuSys *sys){
	//CPU must be given work ram which MBC reads
	//If we had ROM, MBC would switch pageTable[0] to it 
	sys->pageTable[0] = sys->bankList[RAM0_BANK];
	SYS_VAS(PAGE_TABLE_ADDR) = UNCHANGED_BANK;

	//Tell MBC page rest of the ram
	for(uint64_t i=1; i < sys->vasBankCount; i++){
		SYS_VAS(PAGE_TABLE_ADDR+i) = RAM0_BANK+i;
	}
	kemuSys_MBC(sys);

	//Jump to address 0x8000 which is termination signal
	SYS_VAS(0x0001) = JMP;
	SYS_VAS(0x0002) = 0x8000;
	SYS_VAS(0x8000) = TRM; 
}

/**
 * @brief Initialize system state
 * 
*/
void kemuSys_boot(KemuSys *sys, KemuDisk *disk){
	sys->cpu =  (KemuCpu){
		.rw = {0,0,0,0},
		.rb = {0,0,0,0},
		.pc = 0x0001,
		.sp = 0,
		.flags = 0
	};

	kemuSys_addressBanks(sys, disk);
	kemuSys_bootload(sys);
}

/**
 * @brief Execute instructions from ram and increment pc by word 
 * 
*/
void kemuSys_loop(KemuSys *sys){
	uint8_t quitFlag = 0;
	while(!quitFlag){

		//MBC runs on background
		kemuSys_MBC(sys);

		#if KAEL_DEBUG==1
			//debug
			printf("word: %04X\n", SYS_VAS(sys->cpu.pc));
		#endif

		//Load next word
		sys->cpu.rb[0] = SYS_VAS(sys->cpu.pc);
		sys->cpu.pc++;

		//interpret as instruction
		switch(sys->cpu.rb[0]){ 
			case TRM:
				printf("Terminate instruction\n");
				quitFlag=1;
				break;

			case JMP: //Jump to address in next word
				sys->cpu.pc = SYS_VAS(sys->cpu.pc);

			default:
		}
		
	}
}