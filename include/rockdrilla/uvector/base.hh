/* uvector: dynamic array (c++-like version)
 *
 * - common definitions
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_UVECTOR_BASE_HH
#define HEADER_INCLUDED_UVECTOR_BASE_HH 1

#include "../misc/memfun.hh"
#include "../num/getmsb.hh"

namespace uvector {

template<typename value_t, typename index_t = size_t, unsigned int growth_factor = 0>
struct base {

	virtual index_t used(void) const = 0;
	virtual index_t allocated(void) const = 0;

	virtual index_t append(const value_t * source) = 0;
	virtual index_t append(const value_t & source) = 0;

	static constexpr int     r_ptr_bits = sizeof(size_t) * CHAR_BIT;
	static constexpr int     r_idx_bits = sizeof(index_t) * CHAR_BIT;
	static constexpr index_t r_idx_max  = ~((index_t) 0);

	static constexpr size_t item_size  = sizeof(value_t);
	static constexpr size_t align_size = (item_size) ? MEMFUN_MACRO_ALIGN(item_size) : 1;
	static constexpr size_t growth     = MEMFUN_MACRO_CALC_GROWTH_EX(align_size, (growth_factor) ? growth_factor : memfun_growth_factor);
	static constexpr int    align_bits = GETMSB_MACRO32(align_size);
	static constexpr int    fence_bits = (r_idx_bits < r_ptr_bits) ? 1 : align_bits;

	static constexpr index_t idx_inv  = r_idx_max;
	static constexpr index_t idx_max  = (r_idx_max) >> fence_bits;

	static constexpr int idx_bits   = r_idx_bits - fence_bits;
	static constexpr int wfall_bits = idx_bits - 1;

	typedef struct {
		union {
		char     bytes[align_size];
		value_t  value;
		} _;
	} value_align_t;

	CC_INLINE
	static size_t offset_of(index_t index) {
		return (size_t) memfun_ptr_offset_ex(nullptr, align_size, index);
	}

	CC_INLINE
	static bool is_inv(index_t index) {
		return ((index >> idx_bits) != 0);
	}

	CC_INLINE
	static bool is_wfall(index_t index) {
		return ((index >> wfall_bits) != 0);
	}

};

} /* namespace uvector */

#endif /* HEADER_INCLUDED_UVECTOR_BASE_HH */
