/* umul: (safe) unsigned multiply
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_NUM_UMUL
#define HEADER_INCLUDED_NUM_UMUL 1

#include "../misc/ext-c-begin.h"

#include "../misc/cc-inline.h"

#ifdef __has_builtin
  #if __has_builtin(__builtin_umul_overflow)
    #ifndef _UMUL_HAVE_BUILTIN
    #define _UMUL_HAVE_BUILTIN 1
    #endif
  #endif /* __builtin_umul_overflow */

  #if __has_builtin(__builtin_umull_overflow)
    #ifndef _UMUL_HAVE_BUILTIN_L
    #define _UMUL_HAVE_BUILTIN_L 1
    #endif
  #endif /* __builtin_umull_overflow */

  #if __has_builtin(__builtin_umulll_overflow)
    #ifndef _UMUL_HAVE_BUILTIN_LL
    #define _UMUL_HAVE_BUILTIN_LL 1
    #endif
  #endif /* __builtin_umulll_overflow */
#endif /* __has_builtin */

// TODO: write fair enough fallback version

#define _UMUL_DEFINE_FUNC(n, t) \
	static CC_INLINE \
	int umul ## n (t a, t b, t * r) { \
		t res = a * b; \
		if (a > b) { \
			if (res < a) return 0; \
		} else { \
			if (res < b) return 0; \
		} \
		if (r) *r = res; \
		return 1; \
	}

#if _UMUL_HAVE_BUILTIN
#define umul(a, b, r) (!__builtin_umul_overflow(a, b, r))
#else /* ! _UMUL_HAVE_BUILTIN */
_UMUL_DEFINE_FUNC(, unsigned int)
#endif /* _UMUL_HAVE_BUILTIN */

#if _UMUL_HAVE_BUILTIN_L
#define umull(a, b, r) (!__builtin_umull_overflow(a, b, r))
#else /* ! _UMUL_HAVE_BUILTIN_L */
_UMUL_DEFINE_FUNC(l, unsigned long)
#endif /* _UMUL_HAVE_BUILTIN_L */

#if _UMUL_HAVE_BUILTIN_LL
#define umulll(a, b, r) (!__builtin_umulll_overflow(a, b, r))
#else /* ! _UMUL_HAVE_BUILTIN_LL */
_UMUL_DEFINE_FUNC(ll, unsigned long long)
#endif /* _UMUL_HAVE_BUILTIN_LL */

#include "../misc/ext-c-end.h"

#endif /* HEADER_INCLUDED_NUM_UMUL */
