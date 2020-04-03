#ifndef BASE_MATH_ROUNDING_H
#define BASE_MATH_ROUNDING_H

#include <stdint.h>

//-------------------------------------------

static inline uint64_t round_down(uint64_t n, uint64_t d)
{
	return (n / d) * d;
}

static inline uint64_t round_up(uint64_t n, uint64_t d)
{
	return ((n + d - 1) / d) * d;
}

//-------------------------------------------

#endif

