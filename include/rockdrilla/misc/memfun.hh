/* memfun: memory management fun (c++-like version)
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_MEMFUN_HH
#define HEADER_INCLUDED_MEMFUN_HH 1

#include "memfun.h"

template<typename T = void>
CC_FORCE_INLINE
T * memfun_t_alloc_ex(size_t * length)
{
	return (T *) memfun_alloc_ex(length);
}

template<typename T = void>
CC_FORCE_INLINE
T * memfun_t_alloc(size_t length)
{
	return (T *) memfun_alloc(length);
}

template<typename T = void>
CC_FORCE_INLINE
T * memfun_t_ptr_offset(T * ptr, size_t offset)
{
	return (T *) memfun_ptr_offset(ptr, offset);
}

template<typename T = void>
CC_FORCE_INLINE
T * memfun_t_ptr_offset_ex(T * ptr, size_t item_size, size_t item_count)
{
	return (T *) memfun_ptr_offset_ex(ptr, item_size, item_count);
}

template<typename T = void>
CC_FORCE_INLINE
T * memfun_t_realloc_ex(T * ptr, size_t * length, size_t extend)
{
	return (T *) memfun_realloc_ex(ptr, length, extend);
}

template<typename T = void>
CC_FORCE_INLINE
T * memfun_t_realloc(T * ptr, size_t length, size_t extend)
{
	return (T *) memfun_realloc(ptr, length, extend);
}

template<typename T = void>
CC_FORCE_INLINE
void memfun_t_free(T * ptr, size_t length)
{
	memfun_free(ptr, length);
}

#endif /* HEADER_INCLUDED_MEMFUN_HH */
