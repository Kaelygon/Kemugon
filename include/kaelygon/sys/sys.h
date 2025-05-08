/**
 * @file sys.h
 * 
 * @brief Implementation, u16 emulator virtual hardware
 */
#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "kaelygon/disk/disk.h"


typedef struct {
	uint16_t rw[4]; 	//register word
	uint8_t rb[4]; 	//register byte
	uint16_t pc; 		//program counter
	uint16_t sp; 		//stack pointer
	uint16_t flags;
} KemuCpu;

typedef struct {
	const size_t stackSize;
	const size_t bankSize;
	const size_t vasBankCount;

	const size_t firstDiskBank; //MBC index of the first disk bank

	void **bankList;
	size_t bankListSize;

	uint16_t *pageTable[4]; //Null means logically disconnected
	uint16_t *ram[4];

	KemuCpu cpu;
} KemuSys;


//------ Virtual Address Space Macro ------

uint16_t* kemuSys_resolveVAS(uint16_t **pageTable, uint16_t addr);
#define SYS_VAS(addr) (*kemuSys_resolveVAS(sys->pageTable, (addr)))


//------ System ------

void kemuSys_alloc(KemuSys *sys);
void kemuSys_free(KemuSys *sys);

void kemuSys_boot(KemuSys *sys, KemuDisk *disk);

void kemuSys_loop(KemuSys *sys);

