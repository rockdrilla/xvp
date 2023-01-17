/* uadd: (safe) unsigned add
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_NUM_UADD
#define HEADER_INCLUDED_NUM_UADD 1

#include "../misc/ext-c-begin.h"

#include "../misc/cc-inline.h"

#ifdef __has_builtin
  #if __has_builtin(__builtin_uadd_overflow)
    #ifndef _UADD_HAVE_BUILTIN
    #define _UADD_HAVE_BUILTIN 1
    #endif
  #endif /* __builtin_uadd_overflow */

  #if __has_builtin(__builtin_uaddl_overflow)
    #ifndef _UADD_HAVE_BUILTIN_L
    #define _UADD_HAVE_BUILTIN_L 1
    #endif
  #endif /* __builtin_uaddl_overflow */

  #if __has_builtin(__builtin_uaddll_overflow)
    #ifndef _UADD_HAVE_BUILTIN_LL
    #define _UADD_HAVE_BUILTIN_LL 1
    #endif
  #endif /* __builtin_uaddll_overflow */
#endif /* __has_builtin */

// TODO: write fair enough fallback version

#define _UADD_DEFINE_FUNC(n, t) \
	static CC_INLINE \
	int uadd ## n (t a, t b, t * r) { \
		t res = a + b; \
		if (a > b) { \
			if (res < a) return 0; \
		} else { \
			if (res < b) return 0; \
		} \
		if (r) *r = res; \
		return 1; \
	}

#if _UADD_HAVE_BUILTIN
#define uadd(a, b, r) (!__builtin_uadd_overflow(a, b, r))
#else /* ! _UADD_HAVE_BUILTIN */
_UADD_DEFINE_FUNC(, unsigned int)
#endif /* _UADD_HAVE_BUILTIN */

#if _UADD_HAVE_BUILTIN_L
#define uaddl(a, b, r) (!__builtin_uaddl_overflow(a, b, r))
#else /* ! _UADD_HAVE_BUILTIN_L */
_UADD_DEFINE_FUNC(l, unsigned long)
#endif /* _UADD_HAVE_BUILTIN_L */

#if _UADD_HAVE_BUILTIN_LL
#define uaddll(a, b, r) (!__builtin_uaddll_overflow(a, b, r))
#else /* ! _UADD_HAVE_BUILTIN_LL */
_UADD_DEFINE_FUNC(ll, unsigned long long)
#endif /* _UADD_HAVE_BUILTIN_LL */

#include "../misc/ext-c-end.h"

#endif /* HEADER_INCLUDED_NUM_UADD */
