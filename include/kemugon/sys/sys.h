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

typedef struct {
	size_t emuClockSpeed;
	size_t hostClockSpeed;

	size_t pageTableSize;
	KaelTree pageTable; 
	KaelTree dev;

	uint8_t quitFlag;
} KemuSys;

//------ Virtual Address Space Macro ------

uint16_t* kemuSys_resolveVAS(KemuSys *sys, uint16_t addr) ;
#define SYS_VAS(addr) (*kemuSys_resolveVAS(sys, (addr)))


//------ System ------

void kemuSys_alloc(KemuSys *sys);
void kemuSys_free(KemuSys *sys);

void kemuSys_pushDev(KemuSys *sys, KemuDev *dev);

void kemuSys_addDevices(KemuSys *sys);

void kemuSys_boot(KemuSys *sys);
void kemuSys_loop(KemuSys *sys);

