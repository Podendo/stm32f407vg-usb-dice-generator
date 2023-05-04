#include <stdio.h>
#include "string.h"
#include "dicethrow.h"

/*
 * how many bits is left in random
 * number generation process.
 */
static int left = 1;

/*
 * Initialization completion flag.
 */
static int initf = 0;

/*
 * The array of the state vector
 */
static unsigned long state[N];

/*
 * Generation stages, used with init-flag.
 */
static unsigned long *next;




int dice_findway_recursive(int m, int n, int x)
{
	int ways_number = 0, i = 0;

	if ( x < 1 ) {
		return 0;
	} else {
		__asm__("nop");
	}

	for ( i = 0; i <= m; ++i ) {
		ways_number += dice_findway_recursive(m, (n - 1), (x - i));
	}

	return ways_number;
}


int dice_findway_dynamic(int m, int n, int x)
{
	int table[n + 1][x + 1], i = 0, j = 0, k = 0;

	memset(table, 0, sizeof(table));

	for ( j = 1; j <= m && j <= x; j++ ) {
		table[1][j] = 1;
	}

	for (i = 2; i <= n; i++)
		for (j = 1; j <= x; j++)
			for (k = 1; k < m && k < j; k++)
				table[i][j] += table[i-1][j-k];

	return table[n][x];
}


void dice_init_genrand(unsigned long seed)
{
	int j;

	state[0] = seed & SEED_MASK;

	for ( j = 1; j < N; ++j) {
		state[j] = STATE_MASK * ((state[j-1] ^ (state[j-1] >> 30)) + j);
		/* this one used for 32-bit machines, for >32 bit use AND MSB */
	}

	left = 1; initf = 1;

	return;
}


void dice_init_by_array(unsigned long init_key[], int key_length)
{
	int i, j, k;

	dice_init_genrand(19650218UL);
	i = 1; j = 0; k = ((N >key_length) ? N : key_length);

	while (k) {
		state[i] = (state[i] ^ ((state[i-1] ^	\
			(state[i-1] >> 30)) * NUM1)) + init_key[j] + j;

		i = i + 1;
		j = j + 1;
		if ( j >= N ) {
			state[0] = state[N-1]; i = 1;
		}

		if ( j >= key_length ) {
			j = 0;
		}

		k = k - 1;
	}

	for ( k = N - 1; k; k-- ) {
		state[i] = (state[i] ^ \
			((state[i-1] ^ (state[i-1] >> 30)) * NUM2)) - i;

		i = i + 1;
		if ( i >= N ) {
			state[0] = state[N-1]; i = 1;
		}
	}

	left = 1; initf = 1;

	return;
}

void dice_next_state(void)
{
	unsigned long *p = state;
	int j;

	if ( initf == 0 ) {
		dice_init_genrand(5489UL);
	}

	left = N;
	next = state;

	for ( j = N - M + 1; --j; p++ )
		*p = p[M] ^ TWIST(p[0], p[1]);

	for ( j = M; --j; p++ )
		*p = p[M-N] ^ TWIST(p[0], p[1]);

	*p = p[M-N] ^ TWIST(p[0], state[0]);

	return;
}


unsigned long dice_get_genrand(void)
{
	unsigned long y;

	if( --left == 0 ) {
		dice_next_state();
	}

	y = *next++;

	y ^= (y >> 11);
	y ^= (y << 7) & SED1;
	y ^= (y << 15) & SED2;
	y ^= (y >> 18);

	return y;
}
/* EOF */
