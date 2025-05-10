/**
 * @file kaelMacros.c
 * 
 * global kaelMacros implementation file
 * 
 */


#include "libkael/debug/kaelMacros.h"


// ------ Shared implementation ------



// ------ Debug implementation ------
#if KAEL_DEBUG==1

//--- Functions called by macros ---

uint8_t kaelDebug_nullCheck(const void* ptr, const char *ptrName, const char *note) {
	if(ptr==NULL){
		printf("NULL%s_%s ", ptrName, note);
		return KAEL_ERR_NULL;
	}
	return KAEL_SUCCESS;
}


#else
// --- Release specific implementation ---
//If any

#endif