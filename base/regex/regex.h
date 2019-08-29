#ifndef BASE_REGEX_REGEX_H
#define BASE_REGEX_REGEX_H

#include "base/memory/pool.h"

#include <stdint.h>

//----------------------------------------------------------------

struct dm_regex;

/*
 * Initialise an array of num patterns for matching.
 * Uses memory from mem.
 */
struct dm_regex *dm_regex_create(struct dm_pool *mem, const char * const *patterns,
				 unsigned num_patterns);

/*
 * Match string s against the patterns.
 * Returns the index of the highest pattern in the array that matches,
 * or -1 if none match.
 */
int dm_regex_match(struct dm_regex *regex, const char *s);

/*
 * This is useful for regression testing only.  The idea is if two
 * fingerprints are different, then the two dfas are certainly not
 * isomorphic.  If two fingerprints _are_ the same then it's very likely
 * that the dfas are isomorphic.
 *
 * This function must be called before any matching is done.
 */
uint32_t dm_regex_fingerprint(struct dm_regex *regex);

//----------------------------------------------------------------

#endif
