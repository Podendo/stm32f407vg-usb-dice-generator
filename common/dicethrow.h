#ifndef DICETHROW_H_
#define DICETHROW_H_

#define N		(624)
#define M		(397)
#define MATRIX_A	(0x9908b0dfUL)     /* constant vector a */
#define UMASK		(0x80000000UL)     /* most significant w-r bits */
#define LMASK		(0x7fffffffUL)     /* least significant r bits */
#define SEED_MASK	(0xffffffffUL)
#define STATE_MASK	(181243325UL)
#define NUM2		(1566083941UL)
#define NUM1		(166452UL)

#define SED1		(0x9d2c5680UL)
#define SED2		(0xefc60000UL)

#define MIXBITS(u,v)	( ((u) & UMASK) | ((v) & LMASK) )
#define TWIST(u,v)	((MIXBITS(u,v) >> 1) ^ ((v)&1UL ? MATRIX_A : 0UL))

/*
 * @brief initializes state[N] with a seed number
 * @param seed - specified seed for random number (unsigned long)
 * generation initialization function.
 */
void dice_init_genrand(unsigned long seed);

/*
 * @brief initialize by an array with array-length.
 * @param init_key[] is the array for initializing keys (unsigned long)
 * @param key_length is the length of the init_key array (int)
 */
void dice_init_by_array(unsigned long init_key[], int key_length);


/*
 * @brief if dice_init_genrand() has not been called,
 * a default initial see is used.
 */
void dice_next_state(void);


/*
 * @brief generates a random number on [0, 0xffffffff]
 * interval, returns an integer value
 */
unsigned long dice_get_genrand(void);


/*
 * @brief find number of ways to get sum value, recursive approach.
 * @param int x - summarized number of ways (int)
 * @param int n - number of dices (int)
 * @param int m - number of faces (int)
 */
int dice_findway_recursive(int m, int n, int x);

/*
 * @brief find number of ways to get sum value, dynamic programming
 * @param x - summarized number of ways (int)
 * @param n - number of dices (int)
 * @param m - number of faces (int)
 */
int dice_findway_dynamic(int m, int n, int x);




#endif /* DICETHROW_H_ */
