/* popcnt: simple wrapper
 *
 * - if using "builtins" is desired (POPCNT_NO_BUILTIN is not defined):
 *   - if compiler can't optimize current call in build time:
 *     - if "popcnt" instruction is supported by CPU in runtime [determined only once]:
 * 		 - then instruction is to be "called".
 *     - if "popcnt" instruction isn't supported by CPU in runtime:
 *       - "bithacks" function is called (inspired by [1]).
 *   - if compiler can optimize current call in build time:
 *     - nothing is called (you're getting intermediate value somewhere).
 * - if using "builtins" isn't desired (POPCNT_NO_BUILTIN is defined):
 *   - "bithacks" function is called (inspired by [1]).
 *
 * Nota bene: consider NOT using POPCNT_NO_BUILTIN because compiler does greater work
 * than you're expecting (sometimes).
 *
 * refs:
 * - [1] https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
 * - [2] https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
 * - [3] https://gcc.gnu.org/onlinedocs/gcc/x86-Built-in-Functions.html
 *
 * Nota bene: code snippets from [1] are in public domain.
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_NUM_POPCNT
#define HEADER_INCLUDED_NUM_POPCNT 1

#include "../misc/ext-c-begin.h"

#include <limits.h>

#include "../misc/cc-inline.h"

#ifdef __has_builtin
  #if __has_builtin(__builtin_cpu_init)
  #if __has_builtin(__builtin_cpu_supports)
    #ifndef _POPCNT_HAVE_BUILTIN
    #define _POPCNT_HAVE_BUILTIN 1
    #endif
  #endif /* __has_builtin(__builtin_cpu_supports) */
  #endif /* __has_builtin(__builtin_cpu_init) */
#endif /* __has_builtin */
#ifndef _POPCNT_HAVE_BUILTIN
#define _POPCNT_HAVE_BUILTIN 0
#endif
#ifndef POPCNT_NO_BUILTIN
  #if _POPCNT_HAVE_BUILTIN
    #define _POPCNT_USE_BUILTIN 1
  #endif
#endif /* ! POPCNT_NO_BUILTIN */

static int popcnt(unsigned int x);
static int popcntl(unsigned long x);
static int popcntll(unsigned long long x);

#define _POPCNT_BITHACKS(n) popcnt## n ##_bithacks
#define _POPCNT_DECLARE_BITHACKS(n, t) \
	static CC_FORCE_INLINE \
	int _POPCNT_BITHACKS(n) (t x)

_POPCNT_DECLARE_BITHACKS(,   unsigned int);
_POPCNT_DECLARE_BITHACKS(l,  unsigned long);
_POPCNT_DECLARE_BITHACKS(ll, unsigned long long);

#if _POPCNT_USE_BUILTIN

#define _POPCNT_BUILTIN_NONE  0
#define _POPCNT_BUILTIN_OK    1
#define _POPCNT_BUILTIN_NACK  2

static int _popcnt_builtin = _POPCNT_BUILTIN_NONE;

static CC_FORCE_INLINE
int _popcnt_cpu_supports(void)
{
	/* ref:
	 * - https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
	 * - https://gcc.gnu.org/onlinedocs/gcc/x86-Built-in-Functions.html
	 */
	__builtin_cpu_init();
	return (__builtin_cpu_supports("popcnt"))
		? _POPCNT_BUILTIN_OK
		: _POPCNT_BUILTIN_NACK;
}

#define _POPCNT_TRY_BUILTIN(f) \
	{ while (1) { \
		if (_popcnt_builtin == _POPCNT_BUILTIN_OK) \
			return f(x); \
		if (_popcnt_builtin == _POPCNT_BUILTIN_NACK) \
			break; \
		_popcnt_builtin = _popcnt_cpu_supports(); \
	} }

#else /* ! _POPCNT_USE_BUILTIN */

#define _POPCNT_TRY_BUILTIN(f) {}

#endif /* _POPCNT_USE_BUILTIN */

#define _POPCNT_DEFINE_FUNC(n, t) \
	static CC_INLINE \
	int popcnt ## n (t x) { \
		_POPCNT_TRY_BUILTIN(__builtin_popcount ## n) \
		return _POPCNT_BITHACKS(n) (x); \
	}

_POPCNT_DEFINE_FUNC(,   unsigned int)
_POPCNT_DEFINE_FUNC(l,  unsigned long)
_POPCNT_DEFINE_FUNC(ll, unsigned long long)

/* ref:
 * - https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
 */

#define _POPCNT_T_m(t)   ((t) ~0)
#define _POPCNT_T_c0(t)  (_POPCNT_T_m(t) / (t) 3)
#define _POPCNT_T_c1(t)  (_POPCNT_T_m(t) / (t) 15 * (t) 3)
#define _POPCNT_T_c2(t)  (_POPCNT_T_m(t) / (t) 255 * (t) 15)
#define _POPCNT_T_c3(t)  (_POPCNT_T_m(t) / (t) 255)
#define _POPCNT_T_c4(t)  ((sizeof(t) - 1) * CHAR_BIT)

#define _POPCNT_T_R0(v,t)  ((t) (v) - (((t) (v) >> 1) & _POPCNT_T_c0(t)))
#define _POPCNT_T_R1(v,t)  ((_POPCNT_T_R0(v,t) & _POPCNT_T_c1(t)) + ((_POPCNT_T_R0(v,t) >> 2) & _POPCNT_T_c1(t)))
#define _POPCNT_T_R2(v,t)  ((_POPCNT_T_R1(v,t) + (_POPCNT_T_R1(v,t) >> 4)) & _POPCNT_T_c2(t))
#define _POPCNT_T_R3(v,t)  ((_POPCNT_T_R2(v,t) * _POPCNT_T_c3(t)) >> _POPCNT_T_c4(t))

#define _POPCNT_MACRO(v, t)  _POPCNT_T_R3(v, t)

#define _POPCNT32(v) _POPCNT_MACRO(((v) & UINT_MAX),   unsigned int)
#define _POPCNT64(v) _POPCNT_MACRO(((v) & ULLONG_MAX), unsigned long long)

#define POPCNT_MACRO32(v)  ( (((v) & UINT_MAX)   == 0) ? 0 : _POPCNT32(v) )
#define POPCNT_MACRO64(v)  ( (((v) & ULLONG_MAX) == 0) ? 0 : _POPCNT64(v) )

#define _POPCNT_DEFINE_BITHACKS(n, t) \
	_POPCNT_DECLARE_BITHACKS(n, t) \
	{ \
		static const t c0 = _POPCNT_T_c0(t); \
		static const t c1 = _POPCNT_T_c1(t); \
		static const t c2 = _POPCNT_T_c2(t); \
		static const t c3 = _POPCNT_T_c3(t); \
		static const t c4 = _POPCNT_T_c4(t); \
		x = x - ((x >> 1) & c0); \
		x = (x & c1) + ((x >> 2) & c1); \
		x = (x + (x >> 4)) & c2; \
		x = (x * c3) >> c4; \
		return x; \
	}

_POPCNT_DEFINE_BITHACKS(,   unsigned int)
_POPCNT_DEFINE_BITHACKS(l,  unsigned long)
_POPCNT_DEFINE_BITHACKS(ll, unsigned long long)

#include "../misc/ext-c-end.h"

#endif /* HEADER_INCLUDED_NUM_POPCNT */
