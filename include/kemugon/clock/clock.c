/**
 * @file clock.h
 * 
 * @brief Implementation, u16 emulator virtual hardware
 */

#include "kemugon/clock/clock.h"

void rdtsc_sleep(uint64_t sleepTime) {
	uint64_t startTime = __rdtsc();
	while( __rdtsc() - startTime < sleepTime ){
		__builtin_ia32_pause();
	};
}

void kemuClock_sync(KemuClock *clock) {
	uint8_t carry = 0;

	// If lag accumulates >=1 emu-cycle, start of next emu-cycle is advanced by 1 host-cycle
	clock->accumulator += clock->lagCycle;
	if(clock->accumulator >= clock->emuClockSpeed){
		clock->accumulator -= clock->emuClockSpeed;
		carry = 1;
	}

	uint64_t timeNow = __rdtsc();
	uint64_t elapsedTime = kaelMath_sub(timeNow, clock->startTime);
	uint64_t waitTime 	= kaelMath_sub(clock->cycleRatio, elapsedTime);
	rdtsc_sleep(waitTime);
	clock->startTime += clock->cycleRatio + carry;
}

void kemuClock_init(KemuClock *clock, uint64_t hostHz, uint64_t emuHz) {
	clock->hostClockSpeed = hostHz;
	clock->emuClockSpeed  = emuHz;

	clock->cycleRatio		= hostHz/emuHz; //How many host-cycles one emu-cycle takes
	clock->lagCycle		= hostHz%emuHz; //How many host-cycles emulation lags each emu-cycle
	clock->accumulator	= 0; 				//Accumulated lag
	clock->startTime		= __rdtsc();	//Cycle start time in host cpu cycles
}
