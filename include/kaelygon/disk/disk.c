/**
 * @file disk.c
 * 
 * @brief Header, banked virtual disk
 */

 #include "kaelygon/global/kaelMacros.h"

#include "kaelygon/disk/disk.h"

//------ Disk ------

/**
 * @brief allocate virtual disk
 * 
*/
void kemuDisk_alloc( KemuDisk *disk ) {
	disk->totalSize = disk->bankSize * disk->bankCount;

	// Open file for read/write, create if not exists
	disk->fd = open(disk->path, O_RDWR | O_CREAT, 0666);
	if (disk->fd < 0) {
		perror("Failed to open disk file");
		exit(EXIT_FAILURE);
	}

	// Ensure file is of correct size
	if (ftruncate(disk->fd, disk->totalSize) < 0) {
		perror("Failed to set disk size");
		close(disk->fd);
		exit(EXIT_FAILURE);
	}

	// Map file into memory
	disk->image = mmap(NULL, disk->totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, disk->fd, 0);
	if (disk->image == MAP_FAILED) {
		perror("Failed to mmap disk file");
		close(disk->fd);
		exit(EXIT_FAILURE);
	}

	// Addresses of the emulated disk bank pointers
	disk->bank = malloc(sizeof(void*) * disk->bankCount);
	NULL_CHECK(disk->bank);
	for(uint64_t i=0; i<disk->bankCount; i++){
		size_t offset = i*disk->bankSize;
		void *diskBankPtr  = (uint8_t *)disk->image + offset;
		disk->bank[i] = diskBankPtr;
	}
}

/**
 * @brief free virtual disk
 * 
*/
void kemuDisk_free( KemuDisk *disk ) {
	free(disk->bank);
	disk->bank = NULL;

	if (disk->image && disk->image != MAP_FAILED) {
		msync(disk->image, disk->totalSize, MS_SYNC);
		munmap(disk->image, disk->totalSize);
	}
	if (disk->fd >= 0) {
		close(disk->fd);
	}
	disk->image = NULL;
	disk->fd = -1;
}