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
	uint64_t waitTime = clock->cycleRatio - elapsedTime;
	if(!kaelMath_isNegative(waitTime)){
		rdtsc_sleep((uint64_t)waitTime);
	}
	clock->startTime = timeNow + clock->cycleRatio + carry;

	#if KAEL_DEBUG
		if(clock->printDelay==0){
			if(kaelMath_isNegative(waitTime)){
					printf("Emulation lagging behind by %ld host-cycles\n", -1*waitTime);
			}else{
				printf("Emulation headroom %ld host-cycles\n", waitTime);
			}
			clock->printDelay = clock->printFreq;
		}else{
			clock->printDelay--;
		}
	#endif
}

void kemuClock_init(KemuClock *clock, uint64_t hostHz, uint64_t emuHz) {
	clock->hostClockSpeed = hostHz;
	clock->emuClockSpeed  = emuHz;

	clock->cycleRatio		= hostHz/emuHz; //How many host-cycles one emu-cycle takes
	clock->lagCycle		= hostHz%emuHz; //How many host-cycles emulation lags each emu-cycle
	clock->accumulator	= 0; 				//Accumulated lag
	clock->startTime		= __rdtsc();	//Cycle start time in host cpu cycles
	clock->printDelay		= 0;
	clock->printFreq		= emuHz/4;
}
