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

typedef enum {
	NULL_DEV,
	CPU_DEV,	
	GPU_DEV,	
	RAM_DEV,	
	AUDIO_DEV,
	DATA_DEV,
} KemuDev_type;

/**
 * @brief Device header is only visible to MBC, optimize size once the structure is decided
*/
typedef struct {
	size_t bankSize;
	size_t bankCount;
	size_t totalSize;
	uint8_t isBigEnd;
	uint8_t isVolatile;
	uint8_t isROM;
	uint8_t type;
} KemuDev_head;

typedef struct {
	const char *path;
	int fd;
	KemuDev_head head;
	void *data; // Raw memory on host system
	void **bank; // Split image to bankSized segments to emulate banks
} KemuDev;

//------ Base ------

// Virtual device mapped to host system NVM or RAM
void kemuDev_alloc( KemuDev *dev );
void kemuDev_free( KemuDev *dev );