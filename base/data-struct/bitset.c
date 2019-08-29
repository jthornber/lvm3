#include "base/data-struct/bitset.h"

#include "base/memory/zalloc.h"

#include <ctype.h>
#include <strings.h>

/* FIXME: calculate this. */
#define INT_SHIFT 5

dm_bitset_t dm_bitset_create(unsigned num_bits)
{
	unsigned n = (num_bits / DM_BITS_PER_INT) + 2;
	size_t size = sizeof(int) * n;
	dm_bitset_t bs = zalloc(size);

	if (!bs)
		return NULL;

	*bs = num_bits;

	return bs;
}

void dm_bitset_destroy(dm_bitset_t bs)
{
	free(bs);
}

int dm_bitset_equal(dm_bitset_t in1, dm_bitset_t in2)
{
	int i;

	for (i = (in1[0] / DM_BITS_PER_INT) + 1; i; i--)
		if (in1[i] != in2[i])
			return 0;

	return 1;
}

void dm_bit_and(dm_bitset_t out, dm_bitset_t in1, dm_bitset_t in2)
{
	int i;

	for (i = (in1[0] / DM_BITS_PER_INT) + 1; i; i--)
		out[i] = in1[i] & in2[i];
}
void dm_bit_union(dm_bitset_t out, dm_bitset_t in1, dm_bitset_t in2)
{
	int i;
	for (i = (in1[0] / DM_BITS_PER_INT) + 1; i; i--)
		out[i] = in1[i] | in2[i];
}

static int _test_word(uint32_t test, int bit)
{
	uint32_t tb = test >> bit;

	return (tb ? ffs(tb) + bit - 1 : -1);
}

static int _test_word_rev(uint32_t test, int bit)
{
	uint32_t tb = test << (DM_BITS_PER_INT - 1 - bit);

	return (tb ? bit - __builtin_clz(tb) : -1);
}

int dm_bit_get_next(dm_bitset_t bs, int last_bit)
{
	int bit, word;
	uint32_t test;

	last_bit++;		/* otherwise we'll return the same bit again */

	/*
	 * bs[0] holds number of bits
	 */
	while (last_bit < (int) bs[0]) {
		word = last_bit >> INT_SHIFT;
		test = bs[word + 1];
		bit = last_bit & (DM_BITS_PER_INT - 1);

		if ((bit = _test_word(test, bit)) >= 0)
			return (word * DM_BITS_PER_INT) + bit;

		last_bit = last_bit - (last_bit & (DM_BITS_PER_INT - 1)) +
		    DM_BITS_PER_INT;
	}

	return -1;
}

int dm_bit_get_prev(dm_bitset_t bs, int last_bit)
{
	int bit, word;
	uint32_t test;

	last_bit--;		/* otherwise we'll return the same bit again */

	/*
	 * bs[0] holds number of bits
	 */
	while (last_bit >= 0) {
		word = last_bit >> INT_SHIFT;
		test = bs[word + 1];
		bit = last_bit & (DM_BITS_PER_INT - 1);

		if ((bit = _test_word_rev(test, bit)) >= 0)
			return (word * DM_BITS_PER_INT) + bit;

		last_bit = (last_bit & ~(DM_BITS_PER_INT - 1)) - 1;
	}

	return -1;
}

int dm_bit_get_first(dm_bitset_t bs)
{
	return dm_bit_get_next(bs, -1);
}

int dm_bit_get_last(dm_bitset_t bs)
{
	return dm_bit_get_prev(bs, bs[0] + 1);
}
