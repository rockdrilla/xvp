/* roundby: align value at certain alignment value
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_NUM_ROUNDBY
#define HEADER_INCLUDED_NUM_ROUNDBY 1

#include "../misc/ext-c-begin.h"

#include "popcnt.h"
#include "uadd.h"

#define _ROUNDBY_2D(v, a) \
	( ((v) & ~((a) - 1)) + ((((v) & ((a) - 1)) != 0) ? (a) : 0) )

#define _ROUNDBY_ANY(v, a) \
	( (v) + ((((v) % (a)) != 0) ? ((a) - ((v) % (a))) : 0) )

#define ROUNDBY_MACRO(value, align) \
	( ((align) < 2) ? (value) : ( \
		(_POPCNT32(align) == 1) \
		? _ROUNDBY_2D(value, align) \
		: _ROUNDBY_ANY(value, align) \
	) )

#define _ROUNDBY_DEFINE_FUNC(n, t) \
	static CC_INLINE \
	int roundby ## n (t value, t align) { \
		if (align < 2) return value; \
		t r, x; \
		r = (popcnt ## n (align) == 1) ? (value & (align - 1)) : (value % align); \
		if (!r) return value; \
		x = value - r; \
		return (uadd ## n (x, align, &r)) ? r : x; \
	}

_ROUNDBY_DEFINE_FUNC(,   unsigned int)
_ROUNDBY_DEFINE_FUNC(l,  unsigned long)
_ROUNDBY_DEFINE_FUNC(ll, unsigned long long)

#include "../misc/ext-c-end.h"

#endif /* HEADER_INCLUDED_NUM_ROUNDBY */
