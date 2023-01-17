/* uvector: dynamic array (c++-like version)
 *
 * - "contiguous" string stream
 *
 * SPDX-License-Identifier: Apache-2.0
 * (c) 2022-2023, Konstantin Demin
 */

#ifndef HEADER_INCLUDED_UVECTOR_STR_HH
#define HEADER_INCLUDED_UVECTOR_STR_HH

#include "base.hh"
#include "dynmem.hh"

namespace uvector {

template<typename index_t = unsigned int>
struct str {

protected:

	using _base = base<size_t, index_t>;

	static constexpr size_t  item_size = _base::item_size;
	static constexpr index_t idx_max   = _base::idx_max;
	static constexpr index_t idx_inv   = _base::idx_inv;
	static constexpr int     idx_bits  = _base::idx_bits;

	size_t _used = 0, _allocated = 0;
	char * _ptr = nullptr;
	dynmem<size_t, index_t> _offsets;

	CC_INLINE
	void flush_self(void) {
		memset(this, 0, sizeof(*this));
	}

	CC_INLINE
	const char * _get(index_t index) const {
		return memfun_t_ptr_offset(_ptr, _offsets.get_val(index));
	}

public:

	static CC_INLINE
	bool is_inv(index_t index) {
		return ((index >> idx_bits) != 0);
	}

	str() {
		flush_self();
	}

	str(const str & source) {
		flush_self();

		_used = _allocated = source._used;
		_ptr = memfun_t_alloc_ex<char>(&_allocated);
		if (!_ptr) {
			flush_self();
			return;
		}

		_offsets = dynmem(source._offsets);
		if (!_offsets.allocated()) {
			memfun_t_free(_ptr, 0);
			flush_self();
			return;
		}

		memcpy(_ptr, source.get(0), _used);
	}

	str & operator = (const str & other) = default;

	void free(void) {
		_offsets.free();
		memfun_free(_ptr, _used);
		flush_self();
	}

	CC_INLINE
	size_t used(void) const {
		return _used;
	}

	CC_INLINE
	size_t allocated(void) const {
		return _allocated;
	}

	CC_INLINE
	index_t count(void) const {
		return _offsets.used();
	}

	const char * get(index_t index) const {
		if (index >= count()) return nullptr;

		return _get(index);
	}

	template<typename T = unsigned int>
	index_t append(const char * string, T length) {
		if (!string) return idx_inv;
		if (length < 0) return idx_inv;

		size_t new_used = roundbyl(_used + length + 1, sizeof(size_t));
		if (new_used > _allocated) {
			auto nptr = memfun_t_realloc_ex(_ptr, &(_allocated), length + 1);
			if (!nptr) return idx_inv;

			_ptr = nptr;
		}

		index_t idx = _offsets.append(_used);
		if (is_inv(idx)) return idx_inv;

		if (length > 0)
			(void) memcpy(memfun_t_ptr_offset(_ptr, _used), string, length);

		_used = new_used;

		return idx;
	}

	CC_INLINE
	index_t append(const char * string) {
		return append<size_t>(string, (string) ? strlen(string) : 0);
	}

	index_t append(const str & source, index_t begin, index_t count) {
		if (begin >= source.count()) return 0;

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

	index_t append(const str & source) {
		return append(source, 0, source.count());
	}

	template<typename T = const char * const>
	T * to_ptrlist(void) const {
		auto ptrlist = (const char **) memfun_alloc((_offsets.used() + 1) * sizeof(char *));
		if (!ptrlist) return nullptr;

		for (index_t i = 0; i < count(); i++) {
			ptrlist[i] = get(i);
		}

		return (T *) ptrlist;
	}

	void walk(void (*visitor)(index_t, const char *)) const {
		for (index_t i = 0; i < _used; i++) {
			visitor(i, get(i));
		}
	}

	template<typename T = void>
	void walk(void (*visitor)(index_t, const char *, T *), T * state) const {
		for (index_t i = 0; i < _used; i++) {
			visitor(i, get(i), state);
		}
	}

	void rwalk(void (*visitor)(index_t, const char *)) const {
		for (index_t i = _used; (i--) != 0; ) {
			visitor(i, get(i));
		}
	}

	template<typename T = void>
	void rwalk(void (*visitor)(index_t, const char *, T *), T * state) const {
		for (index_t i = _used; (i--) != 0; ) {
			visitor(i, get(i), state);
		}
	}

};

} /* namespace uvector */

#endif /* HEADER_INCLUDED_UVECTOR_STR_HH */
