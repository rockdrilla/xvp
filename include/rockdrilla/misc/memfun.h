/* memfun: memory management fun
 *
 * refs:
 * - [1] https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_MEMFUN
#define HEADER_INCLUDED_MEMFUN 1

#include "ext-c-begin.h"

#include <string.h>
#include <unistd.h>

#include "cc-inline.h"
#include "../num/degree2.h"
#include "../num/popcnt.h"
#include "../num/roundby.h"
#include "../num/uadd.h"
#include "../num/umul.h"

#define _MEMFUN_PAGE_DEFAULT           4096
#define _MEMFUN_BLOCK_DEFAULT          8192
#define _MEMFUN_GROWTH_FACTOR_DEFAULT  10

// #define MEMFUN_PAGE           _MEMFUN_PAGE_DEFAULT
// #define MEMFUN_BLOCK          _MEMFUN_BLOCK_DEFAULT
// #define MEMFUN_GROWTH_FACTOR  _MEMFUN_GROWTH_FACTOR_DEFAULT

#ifndef MEMFUN_MALLOC_DIRTY
#define MEMFUN_MALLOC_DIRTY 1
#endif

#ifndef MEMFUN_REALLOC_DIRTY
#define MEMFUN_REALLOC_DIRTY 1
#endif

#ifndef MEMFUN_FREE_SECURE
#define MEMFUN_FREE_SECURE 1
#endif

#if !(defined(MEMFUN_MALLOC) || defined(MEMFUN_REALLOC) || defined(MEMFUN_FREE))
#include <stdlib.h>
#endif

#ifndef MEMFUN_MALLOC
#define MEMFUN_MALLOC(size) malloc(size)
#endif

#ifndef MEMFUN_REALLOC
#define MEMFUN_REALLOC(ptr, size) realloc(ptr, size)
#endif

#ifndef MEMFUN_FREE
#define MEMFUN_FREE(ptr) free(ptr)
#endif

#ifdef MEMFUN_PAGE
static const size_t memfun_page_default
= ((MEMFUN_PAGE) < _MEMFUN_PAGE_DEFAULT)
? _MEMFUN_PAGE_DEFAULT
: ((POPCNT_MACRO64(MEMFUN_PAGE) == 1) ? (MEMFUN_PAGE) : DEGREE2_NEXT_MACRO64(MEMFUN_PAGE));
#else /* ! MEMFUN_PAGE */
static const size_t memfun_page_default
= _MEMFUN_PAGE_DEFAULT;
#endif /* MEMFUN_PAGE */

#ifdef _SC_PAGESIZE
static
size_t memfun_page_size(void)
{
	static size_t x = 0;
	if (x) return x;

	long len = sysconf(_SC_PAGESIZE);
	if (len > 0) return x = len;

	return x = memfun_page_default;
}
#else
static CC_FORCE_INLINE
size_t memfun_page_size(void)
{
	return memfun_page_default;
}
#endif

#ifdef MEMFUN_BLOCK
static const size_t memfun_block_default
= ((MEMFUN_BLOCK) < _MEMFUN_PAGE_DEFAULT)
? _MEMFUN_BLOCK_DEFAULT
: ((POPCNT_MACRO64(MEMFUN_BLOCK) == 1) ? (MEMFUN_BLOCK) : DEGREE2_NEXT_MACRO64(MEMFUN_BLOCK));
#else /* ! MEMFUN_BLOCK */
static const size_t memfun_block_default
= _MEMFUN_BLOCK_DEFAULT;
#endif /* MEMFUN_BLOCK */

static
size_t memfun_block_size(void)
{
	static size_t x = 0;
	if (x) return x;

	size_t page = memfun_page_size();

	return x = (memfun_block_default > page) ? memfun_block_default : page;
}

#ifdef MEMFUN_GROWTH_FACTOR
static const unsigned int memfun_growth_factor
= (((MEMFUN_GROWTH_FACTOR) > 1) && ((MEMFUN_GROWTH_FACTOR) < (__INTPTR_WIDTH__ / 2)))
? (MEMFUN_GROWTH_FACTOR)
: _MEMFUN_GROWTH_FACTOR_DEFAULT;
#else /* ! MEMFUN_GROWTH_FACTOR */
static const unsigned int memfun_growth_factor
= _MEMFUN_GROWTH_FACTOR_DEFAULT;
#endif /* MEMFUN_GROWTH_FACTOR */

// n2d = next 2' degree

#define _MEMFUN_N2D_DUMB(x) \
	( ((x)<<1) & ~((x)|((x)>>1)|((x)>>2)|((x)>>3)|((x)>>4)|((x)>>5)) )

static CC_FORCE_INLINE
size_t _memfun_n2d_dumb(size_t x)
{
	return (x<<1) & ~(x|(x>>1)|(x>>2)|(x>>3)|(x>>4)|(x>>5));
}

#define MEMFUN_MACRO_ALIGN(length) \
	( (length) < 1) ? 0 : ( \
		(POPCNT_MACRO64(length) == 1) \
		? (length) \
		: ( ((length) > sizeof(size_t)) \
		    ? ROUNDBY_MACRO(length, sizeof(size_t)) \
		    : _MEMFUN_N2D_DUMB(length) \
	) )
// ^- using _MEMFUN_N2D_DUMB instead of DEGREE2_NEXT_MACRO64 due to value "knowledge"

static
size_t memfun_align(size_t length)
{
	if (!length) return 0;

	if (popcntl(length) == 1) return length;

	if (length > sizeof(size_t))
		return roundbyl(length, sizeof(size_t));

	// using _memfun_n2d_dumb() instead of degree2_nextl() due to value "knowledge"
//	return degree2_nextl(length);
	return _memfun_n2d_dumb(length);
}

static CC_FORCE_INLINE
size_t memfun_block_align(size_t length)
{
	if (!length) return 0;

	return roundbyl(length, memfun_block_size());;
}

#define MEMFUN_MACRO_CALC_GROWTH(item_size) \
	( ((item_size) > (memfun_block_default >> memfun_growth_factor)) \
	? ROUNDBY_MACRO((item_size) << memfun_growth_factor, memfun_block_default) \
	: memfun_block_default \
	)

static
size_t memfun_calc_growth(size_t item_size)
{
	// waterfall
	static size_t x = 0;
	if (!x) x = memfun_block_size() >> memfun_growth_factor;

	if (item_size > x)
		return memfun_block_align(item_size << memfun_growth_factor);

	return memfun_block_size();
}

#define MEMFUN_MACRO_CALC_GROWTH_EX(item_size, growth_factor) \
	( ((item_size) > (memfun_block_default >> (growth_factor))) \
	? ROUNDBY_MACRO((item_size) << (growth_factor), memfun_block_default) \
	: memfun_block_default \
	)

static
size_t memfun_calc_growth_ex(size_t item_size, unsigned char growth_factor)
{
	// waterfall
	if (item_size > (memfun_block_size() >> growth_factor))
		return memfun_block_align(item_size << growth_factor);

	return memfun_block_size();
}

static
int memfun_want_realloc_raw(size_t length, size_t extend, size_t * new_length)
{
	if (!extend) return 0;

	size_t result = 0;
	if (!uaddl(length, extend, &result))
		return 0;

	if (result <= length) return 0;

	if (new_length) *new_length = result;
	return 1;
}

static
int memfun_want_realloc(size_t length, size_t extend, size_t * new_length)
{
	if (!extend) return 0;

	size_t a, b;
	a = memfun_block_align(length);
	if (length > a) return 0;

	b = 0;
	if (!memfun_want_realloc_raw(length, extend, &b))
		return 0;

	if (a >= b) return 0;

	a = memfun_block_align(b);
	if (b > a) return 0;

	if (new_length) *new_length = a;
	return 1;
}

static
void * memfun_alloc_ex(size_t * length)
{
	if (!length) return NULL;

	size_t len = memfun_block_align(*length);
	void * ptr = (MEMFUN_MALLOC(len));
	if (!ptr) return NULL;

#if MEMFUN_MALLOC_DIRTY
	if (len) memset(ptr, 0, len);
#endif

	*length = len;
	return ptr;
}

static CC_FORCE_INLINE
void * memfun_alloc(size_t length)
{
	size_t len = length;
	return memfun_alloc_ex(&len);
}

static
void * memfun_ptr_offset(void * ptr, size_t offset)
{
	void * nptr = NULL;
	if (!uaddl((size_t) ptr, offset, (size_t *) &nptr))
		return NULL;
	return nptr;
}

static
void * memfun_ptr_offset_ex(void * ptr, size_t item_size, size_t item_count)
{
	size_t _off;
	if (!umull(item_size, item_count, &_off))
		return NULL;
	return memfun_ptr_offset(ptr, _off);
}

static
void * _memfun_realloc(void * ptr, size_t _old, size_t _new)
{
	void * nptr = (MEMFUN_REALLOC(ptr, _new));
	if (!nptr) return ptr;

#if MEMFUN_REALLOC_DIRTY
	if (_new > _old) {
		void * dirty = memfun_ptr_offset(nptr, _old);
		if (dirty) memset(dirty, 0, _new - _old);
	}
#endif

	return nptr;
}

static
void * memfun_realloc_ex(void * ptr, size_t * length, size_t extend)
{
	if (!length) return ptr;

	size_t _old = memfun_block_align(*length);
	size_t _new = 0;
	if (!memfun_want_realloc(_old, extend, &_new))
		return ptr;

	*length = _new;

	return _memfun_realloc(ptr, _old, _new);
}

static CC_FORCE_INLINE
void * memfun_realloc(void * ptr, size_t length, size_t extend)
{
	size_t old = length;
	return memfun_realloc_ex(ptr, &old, extend);
}

static
void memfun_free(void * ptr, size_t length)
{
	if (!ptr) return;

#if MEMFUN_FREE_SECURE
	if (length) memset(ptr, 0, length);
#endif

	(void) MEMFUN_FREE(ptr);
}

#include "ext-c-end.h"

#endif /* HEADER_INCLUDED_MEMFUN */
