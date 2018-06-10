#include <inttypes.h>
#include "fixed_point.h" 

int32_t fixmulint(int32_t a, int32_t b)
{
	return((a*b) / (1 << SHIFT));
}

int32_t fixdiv(int32_t a, int32_t b)
{
	return(((a * (1 << SHIFT)) / b) / (1 << SHIFT));
}
