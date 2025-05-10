// ./include/kaelygon/kaelMacros.h
/**
 * @file kaelMacros.h
 * 
 * @brief Global macros and variables, ideally only for the debugging phase
*/
#pragma once

#include <stdlib.h>
#include <stdint.h>

//CMake should generate this macro 'KAEL_DEBUG 1'
#ifndef KAEL_DEBUG
		#define KAEL_DEBUG 1
#endif


//------ All targets shared variables ------

#include "libkael/debug/kaelMacrosShared.h"


//------ Debug and Release target exclusives ------

#if KAEL_DEBUG==1
	#include "libkael/debug/target/debugTarget.h"

#else 
	#include "libkael/debug/target/releaseTarget.h"

#endif