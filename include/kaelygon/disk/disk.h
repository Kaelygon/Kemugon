/**
 * @file disk.h
 * 
 * @brief Implementation, banked virtual disk
 */
#pragma once


#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	const char *path;
	int fd;
	void *image; // File image mapped onto host memory
	void **bank; // pointers to bankSized segments in image to emulate banks
	size_t totalSize;
	size_t bankSize;
	size_t bankCount;
} KemuDisk;

void kemuDisk_alloc( KemuDisk *disk );
void kemuDisk_free( KemuDisk *disk );