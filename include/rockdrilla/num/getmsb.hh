/* getmsb: number of most significant bit set
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_NUM_GETMSB_HH
#define HEADER_INCLUDED_NUM_GETMSB_HH 1

#include "getmsb.h"

template<typename T>
static CC_FORCE_INLINE \
T getmsb_t(T a);

#define _GETMSB_T_DEFINE_FUNC(n, T) \
	template<> CC_FORCE_INLINE \
	T getmsb_t<T>(T a) \
	{ \
		return getmsb ## n (a); \
	}

_GETMSB_T_DEFINE_FUNC(,   unsigned int)
_GETMSB_T_DEFINE_FUNC(l,  unsigned long)
_GETMSB_T_DEFINE_FUNC(ll, unsigned long long)

#endif /* HEADER_INCLUDED_NUM_GETMSB_HH */
