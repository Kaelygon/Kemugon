/**
 * @file clock.h
 * 
 * @brief Header, sync emulator to host clock
 */
#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <x86intrin.h>

#include "libkael/debug/kaelMacros.h"
#include "libkael/math/math.h"

#include "kemugon/dev/dev.h"


typedef struct {
	uint64_t hostClockSpeed;
	uint64_t emuClockSpeed;

	uint64_t cycleRatio;
	uint64_t lagCycle;
	uint64_t accumulator;
	uint64_t startTime;
} KemuClock;

void rdtsc_sleep(uint64_t sleepTime);

void kemuClock_sync(KemuClock *clock);

void kemuClock_init(KemuClock *clock, uint64_t hostHz, uint64_t emuHz);