#ifndef DEADBEEF_RAND_H
#define DEADBEEF_RAND_H

#include <stdint.h>

#define DEADBEEF_MAX UINT32_MAX

/* Dead Beef Random Number Generator
 * From: http://inglorion.net/software/deadbeef_rand
 * A fast, portable psuedo-random number generator by BJ Amsterdam Zuidoost.
 * Stated in license terms: "Feel free to use the code in your own software." */

/* Generates a random number between 0 and DEADBEEF_MAX. */
uint32_t deadbeef_rand(void);

/* Seeds with the given integer. */
void deadbeef_srand(uint32_t x);

/* Generates seed from the current time. */
uint32_t deadbeef_generate_seed(void);

/* Seeds with the above function. */
#define deadbeef_srand_time() deadbeef_srand(deadbeef_generate_seed())

/* Returns random double in the range [a, b).
 * Taken directly from the rand() man page. */
#define DEADBEEF_UNIFORM(a, b) \
	((a) + (deadbeef_rand() / (((double)DEADBEEF_MAX / (b - a) + 1))))

/* Returns random integer in the range [a, b).
 * Also taken from the rand() man page. */
#define DEADBEEF_RANDRANGE(a, b) \
	(uint32_t)DEADBEEF_UNIFORM(a, b)

#endif /* DEADBEEF_RAND_H */
