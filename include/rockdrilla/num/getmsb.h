/* getmsb: number of most significant bit set
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_NUM_GETMSB
#define HEADER_INCLUDED_NUM_GETMSB 1

#include "../misc/ext-c-begin.h"

#include "../misc/cc-inline.h"
#include "popcnt.h"
#include "setlower.h"

#define _GETMSB32(v)  ( _POPCNT32(_SETLOWER32(v)) )
#define _GETMSB64(v)  ( _POPCNT64(_SETLOWER64(v)) )

#define GETMSB_MACRO32(v)  ( (((v) & UINT_MAX)   == 0) ? 0 : _GETMSB32(v) )
#define GETMSB_MACRO64(v)  ( (((v) & ULLONG_MAX) == 0) ? 0 : _GETMSB64(v) )

#define _GETMSB_DEFINE_FUNC(n, t) \
	static CC_INLINE \
	int getmsb ## n (t v) { \
		if (v == 0) return 0; \
		return popcnt ## n (set_lower ## n (v)); \
	}

_GETMSB_DEFINE_FUNC(,   unsigned int)
_GETMSB_DEFINE_FUNC(l,  unsigned long)
_GETMSB_DEFINE_FUNC(ll, unsigned long long)

#include "../misc/ext-c-end.h"

#endif /* HEADER_INCLUDED_NUM_GETMSB */
