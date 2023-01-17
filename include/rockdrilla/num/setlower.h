/* set lower: set all bits to 1 starting from most significant set bit
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_NUM_SETLOWER
#define HEADER_INCLUDED_NUM_SETLOWER 1

#include "../misc/ext-c-begin.h"

#include <limits.h>

#include "../misc/cc-inline.h"
#include "../misc/dumb-recurse.h"

#define _SETLOWER_a(v)     ( (v) | ((v) - 1) | ((v) >> 1) )
#define _SETLOWER_b(v)     ( (v) << 1 )
#define _SETLOWER_x(a, b)  ( ((a) | (b)) >> 1 )

// 32 - 3 = 29
#define __SETLOWER32(v)  DUMB_RECURSE_29(_SETLOWER_x, _SETLOWER_a(v), _SETLOWER_b(v))
// 64 - 3 = 61
#define __SETLOWER64(v)  DUMB_RECURSE_61(_SETLOWER_x, _SETLOWER_a(v), _SETLOWER_b(v))

#define _SETLOWER32(v)  (__SETLOWER32((v) & UINT_MAX))
#define _SETLOWER64(v)  (__SETLOWER64((v) & ULLONG_MAX))

#define SET_LOWER_MACRO32(v)  ( (((v) & UINT_MAX)   == 0) ? 0 : _SETLOWER32(v) )
#define SET_LOWER_MACRO64(v)  ( (((v) & ULLONG_MAX) == 0) ? 0 : _SETLOWER64(v) )

#define _SETLOWER_DEFINE_FUNC(n, t) \
	static CC_INLINE \
	t set_lower ## n (t v) { \
		if (v == 0) return 0; \
		t a = v | (v - 1) | (v >> 1); \
		t b = v << 1; \
		for (unsigned int i = 0; i < ((sizeof(t) * CHAR_BIT) - 3); i++) { \
			a = (a | b) >> 1; \
		} \
		return a; \
	}

_SETLOWER_DEFINE_FUNC(,   unsigned int)
_SETLOWER_DEFINE_FUNC(l,  unsigned long)
_SETLOWER_DEFINE_FUNC(ll, unsigned long long)

#include "../misc/ext-c-end.h"

#endif /* HEADER_INCLUDED_NUM_SETLOWER */
