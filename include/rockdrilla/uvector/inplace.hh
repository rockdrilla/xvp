/* uvector: dynamic array (c++-like version)
 *
 * - "in-place" static allocation
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_UVECTOR_INPLACE_HH
#define HEADER_INCLUDED_UVECTOR_INPLACE_HH 1

#include "base.hh"
#include "dynmem.hh"

#include "../num/minmax.h"

namespace uvector {

template<typename value_t, typename index_t = size_t, index_t max_elements = 0>
struct inplace {

protected:

	using _base = base<value_t, index_t>;
	using value_align_t = typename _base::value_align_t;

	static constexpr size_t  item_size = _base::item_size;
	static constexpr index_t idx_max   = (max_elements < _base::idx_max) ? max_elements : _base::idx_max;
	static constexpr index_t idx_inv   = _base::idx_inv;

	index_t _used = 0;
	value_align_t _arr[idx_max];

	CC_INLINE
	void flush_self(void) {
		memset(this, 0, sizeof(*this));
	}

	CC_INLINE
	const value_t * ptr_of(index_t index) const {
		return (const value_t *) &(_arr[index]);
	}

	CC_INLINE
	const value_t & get_raw(index_t index) const {
		return _arr[index]._.value;
	}

	CC_INLINE
	void set_by_ptr(index_t index, const value_t * source) {
		auto item = (value_t *) ptr_of(index);
		if (!item) return;

		if (source)
			(void) memcpy(item, source, item_size);
		else
			(void) memset(item, 0, item_size);
	}

public:

	static CC_INLINE
	bool is_inv(index_t index) {
		return (index >= idx_max);
	}

	inplace() {
		flush_self();
	}

	template<index_t source_max>
	inplace(const inplace<value_t, index_t, source_max> * source) {
		flush_self();
		if (!source) return;

		_used = min(idx_max, source->used());
		if (_used)
			(void) memcpy(_arr, source->get(0), _base::offset_of(_used));
	}

	template<unsigned int source_growth>
	inplace(const dynmem<value_t, index_t, source_growth> & source) {
		flush_self();

		_used = min(idx_max, source.used());
		if (_used)
			(void) memcpy(_arr, (void *) source.get(0), _base::offset_of(_used));
	}

	inplace & operator = (const inplace & other) = default;

	void free(void) {
		flush_self();
	}

	CC_INLINE
	index_t used(void) const {
		return _used;
	}

	CC_INLINE
	index_t allocated(void) const {
		return idx_max;
	}

	const value_t * get(index_t index) const {
		if (index >= _used) return nullptr;

		return ptr_of(index);
	}

	bool set(index_t index, const value_t * source) {
		if (index >= _used) return false;

		set_by_ptr(index, source);
		return true;
	}

	const value_t get_val(index_t index) const {
		if (index >= _used) {
			value_t _default[1] = {};
			return _default[0];
		}

		return get_raw(index);
	}

	const value_t get_val(index_t index, const value_t & fallback) const {
		if (index >= _used) return fallback;

		return get_raw(index);
	}

	bool set(index_t index, const value_t & source) {
		if (index >= _used) return false;

		set_by_ptr(index, &source);
		return true;
	}

	index_t append(const value_t * source) {
		if (_used >= idx_max) return idx_inv;

		set_by_ptr(_used, source);
		return (_used++);
	}

	index_t append(const value_t & source) {
		if (_used >= idx_max) return idx_inv;

		set_by_ptr(_used, &source);
		return (_used++);
	}

	template<index_t source_max>
	index_t append(const inplace<value_t, index_t, source_max> * source, index_t begin, index_t count) {
		if (!source) return 0;
		if (begin >= source->used()) return 0;

		index_t end = begin + count;
		if (end > source->used())
			count = source->used() - begin;

		index_t i, k = idx_inv;
		for (i = begin; i < end; i++) {
			k = append(source->get(i));
			if (is_inv(k)) break;
		}

		if (is_inv(k)) count = i - begin;
		return count;
	}

	template<index_t source_max>
	index_t append(const inplace<value_t, index_t, source_max> * source) {
		return append(source, 0, source->used());
	}

	template<unsigned int source_growth>
	index_t append(const dynmem<value_t, index_t, source_growth> & source, index_t begin, index_t count) {
		if (begin >= source.used()) return 0;

		index_t end = begin + count;
		if (end > source.used())
			count = source.used() - begin;

		index_t i, k = idx_inv;
		for (i = begin; i < end; i++) {
			k = append(source.get(i));
			if (is_inv(k)) break;
		}

		if (is_inv(k)) count = i - begin;
		return count;
	}

	template<unsigned int source_growth>
	index_t append(const dynmem<value_t, index_t, source_growth> & source) {
		return append(source, 0, source.used());
	}

	void walk(void (*visitor)(index_t, const value_t *)) const {
		for (index_t i = 0; i < _used; i++) {
			visitor(i, ptr_of(i));
		}
	}

	template<typename T = void>
	void walk(void (*visitor)(index_t, const value_t *, T *), T * state) const {
		for (index_t i = 0; i < _used; i++) {
			visitor(i, ptr_of(i), state);
		}
	}

	void rwalk(void (*visitor)(index_t, const value_t *)) const {
		for (index_t i = _used; (i--) != 0; ) {
			visitor(i, ptr_of(i));
		}
	}

	template<typename T = void>
	void rwalk(void (*visitor)(index_t, const value_t *, T *), T * state) const {
		for (index_t i = _used; (i--) != 0; ) {
			visitor(i, ptr_of(i), state);
		}
	}

};

} /* namespace uvector */

#endif /* HEADER_INCLUDED_UVECTOR_INPLACE_HH */
