/* degree2: current and next 2's degree
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_NUM_DEGREE2
#define HEADER_INCLUDED_NUM_DEGREE2 1

#include "../misc/ext-c-begin.h"

#include "../misc/cc-inline.h"
#include "setlower.h"
#include "uadd.h"

#define _DEGREE2_CURR32(v)  ( 1 + _SETLOWER32((v) >> 1) )
#define _DEGREE2_CURR64(v)  ( 1 + _SETLOWER64((v) >> 1) )

#define DEGREE2_CURR_MACRO32(v)  ( (((v) & UINT_MAX)   == 0) ? 0 : _DEGREE2_CURR32(v) )
#define DEGREE2_CURR_MACRO64(v)  ( (((v) & ULLONG_MAX) == 0) ? 0 : _DEGREE2_CURR64(v) )

#define _DEGREE2_CURR_FUNC(n, t) \
	static CC_INLINE \
	t degree2_curr ## n (t v) { \
		if (v == 0) return 0; \
		t r, x = set_lower ## n (v >> 1); \
		return (uadd ## n (x, 1, &r)) ? r : 0; \
	}

_DEGREE2_CURR_FUNC(,   unsigned int)
_DEGREE2_CURR_FUNC(l,  unsigned long)
_DEGREE2_CURR_FUNC(ll, unsigned long long)

#define _DEGREE2_NEXT32(v)  ( 1 + _SETLOWER32(v) )
#define _DEGREE2_NEXT64(v)  ( 1 + _SETLOWER64(v) )

#define DEGREE2_NEXT_MACRO32(v)  ( (((v) & UINT_MAX)   == 0) ? 1 : _DEGREE2_NEXT32(v) )
#define DEGREE2_NEXT_MACRO64(v)  ( (((v) & ULLONG_MAX) == 0) ? 1 : _DEGREE2_NEXT64(v) )

#define _DEGREE2_NEXT_FUNC(n, t) \
	static CC_INLINE \
	t degree2_next ## n (t v) { \
		if (v == 0) return 1; \
		t r, x = set_lower ## n (v); \
		return (uadd ## n (x, 1, &r)) ? r : 0; \
	}

_DEGREE2_NEXT_FUNC(,   unsigned int)
_DEGREE2_NEXT_FUNC(l,  unsigned long)
_DEGREE2_NEXT_FUNC(ll, unsigned long long)

#include "../misc/ext-c-end.h"

#endif /* HEADER_INCLUDED_NUM_DEGREE2 */
