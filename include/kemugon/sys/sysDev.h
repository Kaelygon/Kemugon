/**
 * @file sysDev.c
 * 
 * @brief Implementation, functions that depend on sys.c and dev.c
 */

 #include "kemugon/sys/sys.h"
 #include "kemugon/dev/dev.h"

typedef struct {
	uint16_t rw[8]; 	//register word
	uint16_t pc; 		//program counter
	uint16_t sp; 		//stack pointer
	uint16_t flags;
} KemuDev_CPU;

KemuDev *kemuDev_devByType(KemuSys *sys, uint16_t devType, uint8_t n);

void kemuDev_runCPU(KemuSys *sys, KemuDev *dev);
void kemuDev_run(KemuSys *sys);

void kemuSys_pushDev(KemuSys *sys, KemuDev *newDev);
void kemuSys_initDevices(KemuSys *sys);