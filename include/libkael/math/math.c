/**
 * @file math.c
 * 
 * @brief Header, Strictly unsigned arithmetic
 */
#include "libkael/math/math.h"

static const uint64_t SIGN_MASK = 0x8000000000000000U;

inline uint64_t kaelMath_min(uint64_t a, uint64_t b){
	return a<b ? a : b;
}

inline uint64_t kaelMath_max(uint64_t a, uint64_t b){
	return a>b ? a : b;
}
/**
 * @brief Is the MSB 1
 */
inline uint64_t kaelMath_isNegative(uint64_t a){
	return a&SIGN_MASK;
}

/**
 * @brief Is the number greater than 0 signed
 */
inline uint64_t kaelMath_gtZeroSigned(uint64_t a){
	return (a!=0) && (!kaelMath_isNegative(a));
}

/**
 * @brief Ignore sign bit
 */
inline uint64_t kaelMath_abs(uint64_t a){
	return a&~SIGN_MASK;
}

/**
 * @brief uint subtract that doesn't underflow
 */
inline uint64_t kaelMath_sub(uint64_t a, uint64_t b){
	return a >= b ? a - b : 0;
}
