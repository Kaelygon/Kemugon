/**
 * @file dev.c
 * 
 * @brief Implementation, banked virtual disk
 */

#include "libkael/debug/kaelMacros.h"

#include "kemugon/sys/sys.h"
#include "kemugon/dev/dev.h"

//------ Disk ------

/**
 * @brief Allocate as image or in RAM on host
 * 
*/
uint8_t kemuDev_alloc( KemuDev *disk ) {
	if(disk==NULL){
		return KEMU_FAIL;
	}
	disk->head.totalSize = disk->head.bankSize * disk->head.bankCount * sizeof(uint16_t);
	disk->bank = calloc(disk->head.bankCount, sizeof(void*));
	if(NULL_CHECK(disk->bank)){
		return KEMU_FAIL;
	};

	if(disk->path!=NULL){ 
		//Stored as image file on host
		// Open file for read/write, create if not exists
		disk->fd = open(disk->path, O_RDWR | O_CREAT, 0666);
		if (disk->fd < 0) {
			perror("Failed to open disk file");
			exit(EXIT_FAILURE);
		}

		// Ensure file is of correct size
		if (ftruncate(disk->fd, disk->head.totalSize) < 0) {
			perror("Failed to set disk size");
			close(disk->fd);
			exit(EXIT_FAILURE);
		}

		// Map file into memory
		disk->data = mmap(NULL, disk->head.totalSize, PROT_READ | PROT_WRITE, MAP_SHARED, disk->fd, 0);
		if (disk->data == MAP_FAILED) {
			perror("Failed to mmap disk file");
			close(disk->fd);
			exit(EXIT_FAILURE);
		}

	}else{
		//Exists only in host ram
		disk->data = calloc(disk->head.totalSize, sizeof(uint16_t));
	}

	// Addresses of the emulated disk bank pointers
	for(uint64_t i=0; i<disk->head.bankCount; i++){
		size_t offset = i * disk->head.bankSize *  sizeof(uint16_t);
		void *diskBankPtr  = (uint8_t *)disk->data + offset;
		disk->bank[i] = diskBankPtr;
	}

	return KEMU_SUCCESS;
}

void kemuDev_free( KemuDev *disk ) {
	free(disk->bank);
	disk->bank = NULL;

	if(disk->path!=NULL){ 
		//Sync image
		if (disk->data && disk->data != MAP_FAILED) {
			msync(disk->data, disk->head.totalSize, MS_SYNC);
			munmap(disk->data, disk->head.totalSize);
		}
		if (disk->fd >= 0) {
			close(disk->fd);
		}
		disk->fd = -1;
	}else{
		free(disk->data);
	}
	disk->data = NULL;
}