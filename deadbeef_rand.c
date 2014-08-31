#include "deadbeef_rand.h"
#include <time.h>

static uint32_t deadbeef_seed;
static uint32_t deadbeef_beef = 0xdeadbeef;

uint32_t deadbeef_rand(void)
{
	deadbeef_seed = (deadbeef_seed << 7) ^ ((deadbeef_seed >> 25) + deadbeef_beef);
	deadbeef_beef = (deadbeef_beef << 7) ^ ((deadbeef_beef >> 25) + 0xdeadbeef);
	return deadbeef_seed;
}

void deadbeef_srand(uint32_t x)
{
	deadbeef_seed = x;
	deadbeef_beef = 0xdeadbeef;
}

/* Taken directly from the documentation:
 * http://inglorion.net/software/cstuff/deadbeef_rand/ */
uint32_t deadbeef_generate_seed(void)
{
	  uint32_t t = (uint32_t)time(NULL);
	  uint32_t c = (uint32_t)clock();
	  return (t << 24) ^ (c << 11) ^ t ^ (size_t) &c;
}
