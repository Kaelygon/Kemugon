/**
 * @file math.h
 * 
 * @brief Implementation, Strictly unsigned arithmetic
 */
#pragma once

#include <stdint.h>
#include <limits.h>
#include <string.h>
#include "libkael/debug/kaelMacros.h"

//kaelmath

uint64_t kaelMath_min(uint64_t a, uint64_t b);
uint64_t kaelMath_max(uint64_t a, uint64_t b);
uint64_t kaelMath_isNegative(uint64_t a);
uint64_t kaelMath_gtZeroSigned(uint64_t a);
uint64_t kaelMath_abs(uint64_t a);
uint64_t kaelMath_sub(uint64_t a, uint64_t b);
