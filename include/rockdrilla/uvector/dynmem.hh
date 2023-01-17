/* uvector: dynamic array (c++-like version)
 *
 * - dynamic memory allocation
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_UVECTOR_DYNMEM_HH
#define HEADER_INCLUDED_UVECTOR_DYNMEM_HH 1

#include "base.hh"

namespace uvector {

template<typename value_t, typename index_t = size_t, unsigned int growth_factor = 0>
struct dynmem {

protected:

	using _base = base<value_t, index_t, growth_factor>;
	using value_align_t = typename _base::value_align_t;

	static constexpr size_t  item_size  = _base::item_size;
	static constexpr size_t  align_size = _base::align_size;
	static constexpr size_t  growth     = _base::growth;
	static constexpr index_t idx_max    = _base::idx_max;
	static constexpr index_t idx_inv    = _base::idx_inv;
	static constexpr int     idx_bits   = _base::idx_bits;

	index_t _used = 0, _allocated = 0;
	value_align_t * _ptr = nullptr;

	CC_INLINE
	void flush_self(void) {
		memset(this, 0, sizeof(*this));
	}

	CC_INLINE
	const value_t * ptr_of(index_t index) const {
		return (const value_t *) memfun_ptr_offset_ex(_ptr, align_size, index);
	}

	CC_INLINE
	const value_t & get_raw(index_t index) const {
		return _ptr[index]._.value;
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

	int _grow_by_bytes(size_t bytes) {
		size_t _new = _base::offset_of(_allocated);
		auto nptr = memfun_t_realloc_ex(_ptr, &_new, bytes);
		if ((!nptr) || (!_new)) return 0;

		size_t _alloc = _new / align_size;
		_allocated = (_alloc < idx_max) ? _alloc : idx_max;

		if (_ptr == nptr) return 1;

		_ptr = nptr;
		return 2;
	}

	int _grow_by_count(index_t count) {
		size_t _new = 0;
		if (_base::is_wfall(_allocated)) {
			if (!uaddl(_allocated, count, &_new))
				return 0;
		} else {
			_new = _allocated + count;
		}

		if (is_inv(_new)) return 0;

		return _grow_by_bytes(_base::offset_of(count));
	}

public:

	static CC_INLINE
	bool is_inv(index_t index) {
		return ((index >> idx_bits) != 0);
	}

	dynmem() {
		flush_self();
	}

	template<unsigned int source_growth>
	dynmem(const dynmem<value_t, index_t, source_growth> & source) {
		flush_self();

		if (!grow_by_count(source.used())) return;

		_used = source.used();
		if (_used)
			(void) memcpy(_ptr, source.get(0), _base::offset_of(_used));
	}

	dynmem(index_t reserve_count) {
		flush_self();
		grow_by_count(reserve_count);
	}

	dynmem & operator = (const dynmem & other) = default;

	void free(void) {
		memfun_t_free(_ptr, _base::offset_of(_used));
		flush_self();
	}

	CC_INLINE
	index_t used(void) const {
		return _used;
	}

	CC_INLINE
	index_t allocated(void) const {
		return _allocated;
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
		if (!grow_auto()) return idx_inv;

		set_by_ptr(_used, source);
		return (_used++);
	}

	index_t append(const value_t & source) {
		if (!grow_auto()) return idx_inv;

		set_by_ptr(_used, &source);
		return (_used++);
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

	int grow_by_bytes(size_t bytes) {
		if (!bytes) return 0;
		if (_allocated >= idx_max) return 0;

		return _grow_by_bytes(bytes);
	}

	int grow_by_count(index_t count) {
		if (!count) return 0;
		if (_base::is_wfall(count)) return 0;
		if (_allocated >= idx_max) return 0;

		return _grow_by_count(count);
	}

	int grow_auto(void) {
		if (_used < _allocated)
			return 1;

		return grow_by_bytes(growth);
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

#endif /* HEADER_INCLUDED_UVECTOR_DYNMEM_HH */
