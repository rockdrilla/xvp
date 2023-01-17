/* minmax: min/max
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_NUM_MINMAX
#define HEADER_INCLUDED_NUM_MINMAX 1

#include "../misc/ext-c-begin.h"

#include "../misc/cc-inline.h"

#ifndef min
#define min(a,b) (((a) > (b)) ? (b) : (a))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#define _MIN_POSITIVE_DEFINE_FUNC(n, t) \
	static CC_INLINE \
	t min_positive ## n (t a, t b) { \
		if (a < 1) return max(b, 0); \
		if (b < 1) return max(a, 0); \
		return min(a, b); \
	}

_MIN_POSITIVE_DEFINE_FUNC(,   int)
_MIN_POSITIVE_DEFINE_FUNC(l,  long)
_MIN_POSITIVE_DEFINE_FUNC(ll, long long)

#include "../misc/ext-c-end.h"

#endif /* HEADER_INCLUDED_NUM_MINMAX */
