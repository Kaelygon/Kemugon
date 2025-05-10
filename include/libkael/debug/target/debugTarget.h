/**
 * @file kaelDebug.h
 * 
 * @brief kaelMacros implementation header for CMake DEBUG target
 */

#pragma once

#include "libkael/debug/kaelMacrosShared.h"

#include <assert.h>
#include <stdio.h>

//Select 1 or 2 argument macro
#define KAEL_MACRO_ARGS2(_1, _2, NAME, ...) NAME


//Debug assert
#define KAEL_ASSERT_ARGS1(condition) assert(condition)
#define KAEL_ASSERT_ARGS2(condition, message) assert((condition) && (message))

#define KAEL_ASSERT(...) KAEL_MACRO_ARGS2(__VA_ARGS__, KAEL_ASSERT_ARGS2, KAEL_ASSERT_ARGS1)(__VA_ARGS__)


//Debug NULL

#define NULL_CHECK_ARGS1(ptr) ( kaelDebug_nullCheck(ptr, #ptr, NULL) )
#define NULL_CHECK_ARGS2(ptr, note) ( kaelDebug_nullCheck(ptr, #ptr, note) )

//return 128 and print *char if ptr is NULL, otherwise return 0
#define NULL_CHECK(...) KAEL_MACRO_ARGS2(__VA_ARGS__, NULL_CHECK_ARGS2, NULL_CHECK_ARGS1)(__VA_ARGS__)



uint8_t kaelDebug_nullCheck(const void* ptr, const char *ptrName, const char *note); //DEBUG