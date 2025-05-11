/**
 * @file sysDev.c
 * 
 * @brief Implementation, functions that depend on sys.c and dev.c
 */

 #include "kemugon/sys/sys.h"
 #include "kemugon/dev/dev.h"

typedef enum{
	NONE_MBC,
	BUSY_MBC,
	REM_MBC,
	ADD_MBC,
}KemuDev_flagMBC;

typedef enum{
	CARRY_CPU		= 0b00000001,
	OVERFLOW_CPU	= 0b00000010,
}KemuDev_flagCPU;

KemuDev *kemuDev_devByID(const KemuSys *sys, const uint16_t devID);
KemuDev *kemuDev_devByType(const KemuSys *sys, const uint16_t devType, uint8_t n);

void kemuDev_runCPU(KemuSys *sys, KemuDev *dev);
void kemuDev_run(KemuSys *sys);

void kemuSys_pushDev(KemuSys *sys, KemuDev *newDev);
void kemuSys_initDevices(KemuSys *sys);