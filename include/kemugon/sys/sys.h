/**
 * @file sys.h
 * 
 * @brief header, u16 emulator virtual hardware
 */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <x86intrin.h>

#include "libkael/debug/kaelMacros.h"
#include "libkael/treeMem/tree.h"
#include "libkael/math/math.h"

#include "kemugon/clock/clock.h"

#define EMU_CHAR_BIT

/**
* @brief System configuration addresses in VAS
*/
typedef enum {
	MBC_FLAG_ADDR		= 1,
	PAGE_TABLE_ADDR	= 2,
	STACK_ADDR			= PAGE_TABLE_ADDR + 0x0020,
	DATA_ADDR			= 0x0200, //End of system reserved
	BOOT_ADDR			= 0x4000
}KemuSys_reservedAddr;

//------ Page table ------

typedef struct{
	uint8_t devID; 
	uint8_t pageIndex; 
	uint8_t firstBank; //first bank of the device
	uint8_t lastBank; 
}KemuSys_pageEntry;

typedef struct{
	uint64_t emuClockSpeed;
	uint64_t hostClockSpeed;

	size_t mapPageCount;
	size_t pageSize;
	uint16_t **frameTable;
	KemuSys_pageEntry *pageTable; 
	KaelTree dev;

	uint8_t quitFlag;
}KemuSys;

//------ Virtual Address Space Macro ------

uint16_t* kemuSys_resolveVAS(const KemuSys *sys, const uint16_t addr) ;
#define SYS_VAS(addr) (*kemuSys_resolveVAS(sys, (addr)))


//------ System ------

uint16_t kemuSys_u8Pack(uint8_t hi, uint8_t lo);

void kemuSys_alloc(KemuSys *sys);
void kemuSys_free(KemuSys *sys);

void kemuSys_pushDev(KemuSys *sys, KemuDev *dev);

void kemuSys_addDevices(KemuSys *sys);

uint8_t kemuSys_boot(KemuSys *sys);
void kemuSys_loop(KemuSys *sys);

