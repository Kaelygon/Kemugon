/**
 * @file dev.h
 * 
 * @brief Header, banked virtual device
 */
#pragma once


#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum{
	NULL_DEV,
	MBC_DEV,	//memory bank controller
	CPU_DEV,	
	GPU_DEV,	
	RAM_DEV,	
	AUDIO_DEV,
	DATA_DEV,
} KemuDev_type;

/**
 * @brief Device header is only visible to MBC, optimize size once the structure is decided
*/
typedef struct{
	size_t bankSize;
	size_t bankCount;
	size_t totalSize;
	uint8_t isROM;
	uint8_t type;
}KemuDev_head;

typedef struct {
	uint16_t devID;
	const char *path;
	int fd;
	KemuDev_head head;
	uint16_t *data; // Raw memory on host system
	uint16_t **bank; // Split image to bankSized segments to emulate banks
}KemuDev;

//------ Special Devices ------

typedef struct{
	uint16_t rw[8]; 	//register word
	uint16_t pc; 		//program counter
	uint16_t sp; 		//stack pointer
	uint16_t flags;
}KemuDev_CPU;

// Virtual device mapped to host system NVM or RAM
uint8_t kemuDev_alloc( KemuDev *dev );
void kemuDev_free( KemuDev *dev );