// bitmask.hpp
#pragma once

#include <sss/bit_operation/bit_operation.h>
#include <sss/util/list_helper.hpp>

#include <cassert>
#include <cstring>
#include <iostream>
#include <vector>
#include <array>
#include <cstdlib>

#include <boost/move/utility.hpp>
#include <boost/functional/hash.hpp>

namespace sss {
namespace ECS {

template<size_t N>
struct group_mask;

template<size_t N>
class bitmask
{
public:
    typedef bitmask this_type;

    typedef std::uint32_t mask_type;
    typedef std::uint64_t mask2_type;
    typedef std::array<mask_type, N> bit_arr_type;

    enum {
        E_size = N,
        E_bitwidth = sizeof(mask_type) * 8,
        E_bitsize = N * E_bitwidth,
    };

    static const mask_type E_mask = ~0xFFFFFFFFu;

    bitmask()
    {
        sss::utility::init_list(_bit_arr);
    }

    bitmask(const mask_type * data, size_t len)
    {
        for (size_t i = 0; i != N; ++i) {
            _bit_arr[i] = data[i];
        }
    }

    bitmask(const this_type& ref)
        :
            _bit_arr (ref._bit_arr)
    {}

    bitmask(const mask_type& v)
    {
        sss::utility::init_list(_bit_arr);
        assert(N >= 1);
        _bit_arr[0] = v;
    }

    bitmask(const mask2_type& v)
    {
        sss::utility::init_list(_bit_arr);
        assert(N >= 1);
        _bit_arr[0] = v;
        if (N >= 2)
        {
            _bit_arr[1] = (v >> E_bitwidth);
        }
    }

    this_type& operator &= (mask_type m)
    {
        _bit_arr[0] &= m;
        for (size_t i = 1; i != N; ++i) {
            _bit_arr[i] = 0u;
        }
        return *this;
    }

    this_type& operator |= (mask_type m)
    {
        assert(N >= 1);
        _bit_arr[0] |= m;
        return *this;
    }

    this_type& operator &= (const this_type& o)
    {
        for (size_t i = 0; i != N; ++i) {
            _bit_arr[i] &= o._bit_arr[i];
        }
        return *this;
    }

    this_type& operator |= (const this_type& o)
    {
        for (size_t i = 0; i != N; ++i) {
            _bit_arr[i] |= o._bit_arr[i];
        }
        return *this;
    }

    this_type& operator >>= (size_t os)
    {
        size_t pad_len = os % E_bitwidth;
        mask_type mask = ~0u >> (E_bitwidth - pad_len);
        size_t iend = os / E_bitwidth;
        for (int i = N, j = N - iend; i > iend; --i, --j) {
            _bit_arr[i - 1] = (_bit_arr[j - 1] << pad_len);
            if (j >= 2 && pad_len) {
                _bit_arr[i - 1] |= (mask & (_bit_arr[j - 2] >> (E_bitwidth - pad_len)));
            }
        }
        for (int i = 0; i != iend; ++i) {
            _bit_arr[i] = 0u;
        }
        return *this;
    }

    this_type& operator <<= (size_t os)
    {
        size_t pad_len = os % E_bitwidth;
        mask_type mask = ~0u >> (E_bitwidth - pad_len);
        size_t iend = os / E_bitwidth;
        for (int i = 0, j = iend; i != N - iend; ++i, ++j) {
            _bit_arr[i] = (_bit_arr[j] >> pad_len);
            if (j + 1 < N && pad_len) {
                _bit_arr[i] |= ((mask & _bit_arr[j + 1]) << (E_bitwidth - pad_len));
            }
        }
        for (int i = 0; i != iend; ++i) {
            _bit_arr[N - i - 1] = 0u;
        }
        return *this;
    }

    this_type operator~() const
    {
        this_type rst(*this);
        rst.inverse();
        return rst;
    }

    this_type& inverse()
    {
        for (int i = 0; i != N; ++i) {
            _bit_arr[i] = ~_bit_arr[i];
        }
        return *this;
    }

    bool operator[](size_t i) const
    {
        assert(i < E_bitwidth);
        return _bit_arr[i / E_bitwidth] & (0x01u << (i % E_bitwidth));
    }

    void set(size_t i, bool v)
    {
        assert(i < E_bitwidth);
        if (v) {
            _bit_arr[i / E_bitwidth] |=  (0x01u << (i % E_bitwidth));
        } else {
            _bit_arr[i / E_bitwidth] &= ~(0x01u << (i % E_bitwidth));
        }
    }

    size_t size() const
    {
        return E_bitsize;
    }

    operator const void*() const
    {
        for (size_t i = 0; i != N; ++i) {
            if (_bit_arr[i]) {
                return this;
            }
        }
        return 0;
    }

    void reverse()
    {
        for (size_t i = 0, iend = N/2; i != iend; ++i) {
            mask_type l = sss::bit::bit_reverse(_bit_arr[i]);
            mask_type r = sss::bit::bit_reverse(_bit_arr[N - i - 1]);
            _bit_arr[i] = r;
            _bit_arr[N - i - 1] = l;
        }
        if (N & 1) {
            size_t middle = N/2;
            _bit_arr[middle] = sss::bit::bit_reverse(_bit_arr[middle]);
        }
    }

    // NOTE
    // 0: all zero
    // 1: the lowest bit is 1
    // ...
    size_t highest_bit_pos() const
    {
        for (size_t i = N, offset = E_bitsize - E_bitwidth; i; --i, offset -= E_bitwidth) {
            const mask_type& m = _bit_arr[i - 1];
            if (m) {
                return sss::bit::highest_bit_pos(m) + offset;
            }
        }
        return 0;
    }

    size_t lowest_bit_pos() const
    {
        for (size_t i = 0, offset = 0;  i != N; ++i, offset += E_bitwidth) {
            const mask_type& m = _bit_arr[i];
            if (m) {
                return sss::bit::lowest_bit_pos(m) + offset;
            }
        }
        return 0;
    }

    this_type& on(size_t i)
    {
        this->set(i, true);
        return *this;
    }

    this_type& off(size_t i)
    {
        this->set(i, false);
        return *this;
    }

    struct op_on_type
    {
        typedef void result_type;
        void operator()(mask_type& v, mask_type m)
        {
            v |= m;
        }
    };

    struct op_off_type
    {
        typedef void result_type;
        void operator()(mask_type& v, mask_type m)
        {
            v &= ~m;
        }
    };

    this_type& on_range(size_t range_beg, size_t range_size)
    {
        this_type::prv_range_algo(op_on_type(), range_beg, range_size);
        return *this;
    }

    this_type& off_range(size_t range_beg, size_t range_size)
    {
        this_type::prv_range_algo(op_off_type(), range_beg, range_size);
        return *this;
    }

    bool is_any_range(size_t range_beg, size_t range_size)
    {
        struct check_type
        {
            bool& has_any;
            check_type(bool& has)
                :
                    has_any (has)
            {}
            //typedef bool result_type;
            bool operator() (mask_type v, mask_type m) const
            {
                if (!has_any && (v & m))
                {
                    has_any = true;
                }
                return has_any;
            }
        };

        bool has_any = false;

        this_type::prv_range_algo_view(check_type(has_any), range_beg, range_size);

        return has_any;
    }

private:
    template<typename Op>
    void prv_range_algo(Op op, size_t range_beg, size_t range_size)
    {
        assert(range_beg < E_bitwidth);
        assert(range_beg + range_size < E_bitsize + 1);
        if (range_size == 0)
        {
            return;
        }
        size_t range_last = range_beg + range_size - 1;
        size_t sub_beg    = range_beg / E_bitwidth;
        size_t sub_last   = range_last / E_bitwidth;

        if (sub_beg == sub_last)
        {
            mask_type mod_mask = ~0u;
            mod_mask >>= (E_bitwidth - range_size);
            mod_mask <<= (range_beg % E_bitwidth);
            op(_bit_arr[sub_beg], mod_mask);
        }
        else
        {
            // NOTE first partial part
            {
                mask_type mod_mask = ~0u;
                mod_mask <<= (range_beg % E_bitwidth);
                op(_bit_arr[sub_beg], mod_mask);
            }
            // NOTE middle part
            for (size_t i = sub_beg + 1, isize = sub_last; i != isize; ++i)
            {
                op(_bit_arr[i], ~0u);
            }
            // NOTE last partial part
            {
                mask_type mod_mask = ~0u;
                mod_mask >>= (E_bitwidth - (range_last % E_bitwidth) - 1);
                op(_bit_arr[sub_last], ~0u);
            }
        }
    }

    template<typename Op>
    void prv_range_algo_view(Op op, size_t range_beg, size_t range_size) const
    {
        assert(range_beg < E_bitwidth);
        assert(range_beg + range_size < E_bitsize + 1);
        if (range_size == 0)
        {
            return;
        }
        size_t range_last = range_beg + range_size - 1;
        size_t sub_beg    = range_beg / E_bitwidth;
        size_t sub_last   = range_last / E_bitwidth;

        if (sub_beg == sub_last)
        {
            mask_type mod_mask = ~0u;
            mod_mask >>= (E_bitwidth - range_size);
            mod_mask <<= (range_beg % E_bitwidth);
            if (op(_bit_arr[sub_beg], mod_mask))
            {
                return;
            }
        }
        else
        {
            // NOTE first partial part
            {
                mask_type mod_mask = ~0u;
                mod_mask <<= (range_beg % E_bitwidth);
                if (op(_bit_arr[sub_beg], mod_mask))
                {
                    return;
                }
            }
            // NOTE middle part
            for (size_t i = sub_beg + 1, isize = sub_last; i != isize; ++i)
            {
                if (op(_bit_arr[i], ~0u))
                {
                    return;
                }
            }
            // NOTE last partial part
            {
                mask_type mod_mask = ~0u;
                mod_mask >>= (E_bitwidth - (range_last % E_bitwidth) - 1);
                if (op(_bit_arr[sub_last], ~0u))
                {
                    return;
                }
            }
        }
    }
public:

    void clear()
    {
        sss::utility::init_list(_bit_arr);
    }

    bool is_any() const
    {
        return this_type::operator void*();
    }

    bool is_full() const
    {
        for (size_t i = 0; i != N; ++i) {
            if (_bit_arr[i] != ~0u) {
                return false;
            }
        }
        return true;
    }

    size_t count() const
    {
        size_t rst = 0u;
        for (size_t i = 0; i != N; ++i) {
            rst += sss::bit::count_1_bit(_bit_arr[i]);
        }
        return rst;
    }

    bool is_only() const
    {
        size_t i = N;
        bool found_first = false;
        for (; i; --i) {
            if (_bit_arr[i - 1]) {
                if (sss::bit::is_power_of_2(_bit_arr[i - 1])) {
                    found_first = true;
                    --i;
                    break;
                }
                else {
                    return false;
                }
            }
        }

        if (!found_first) {
            return false;
        }

        for (; i; --i) {
            if (_bit_arr[i - 1]) {
                return false;
            }
        }
        return true;
    }

    void on_low_bits()
    {
        size_t i = N;
        for (; i; --i) {
            if (_bit_arr[i - 1]) {
                _bit_arr[i - 1] = sss::bit::on_lowest_bit(_bit_arr[i - 1]);
                --i;
                break;
            }
        }
        for (; i; --i) {
            _bit_arr[i - 1] = ~0u;
        }
    }
    void print(std::ostream& out) const
    {
        out << "(";
        for (size_t i = N; i; --i) {
            if (i != N) {
                out << " ";
            }
            out << ext::binary << _bit_arr[i - 1];
        }
        out << ")";
    }

    bool contains(const this_type& r) const
    {
        for (size_t i = 0; i != N; ++i) {
            if ((_bit_arr[i] & r._bit_arr[i]) != r._bit_arr[i]) {
                return false;
            }
        }
        return true;
    }

    bool contains(mask_type m) const
    {
        return ((_bit_arr[0] & m) != m);
    }

    const bit_arr_type& data() const
    {
        return _bit_arr;
    }

    bit_arr_type& data()
    {
        return _bit_arr;
    }

    friend struct group_mask<N>;

    bool operator == (const this_type& ref) const
    {
        for (size_t i = 0; i != N; ++i) {
            if (_bit_arr[i] != ref._bit_arr[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator < (const this_type& o) const
    {
        for (size_t i = N; i; --i) {
            if (_bit_arr[i - 1] > o._bit_arr[i - 1]) {
                return false;
            }
            else if (_bit_arr[i - 1] < o._bit_arr[i - 1]) {
                return true;
            }
        }
        return false;
    }

    bool operator > (const this_type& o) const
    {
        for (size_t i = N; i; --i) {
            if (_bit_arr[i - 1] > o._bit_arr[i - 1]) {
                return true;
            }
            else if (_bit_arr[i - 1] < o._bit_arr[i - 1]) {
                return false;
            }
        }
        return false;
    }

    bool empty_intersect(const this_type& ref) const
    {
        for (size_t i = 0; i != N; ++i) {
            if (_bit_arr[i] & ref._bit_arr[i]) {
                return false;
            }
        }
        return true;
    }

    static bool empty_intersect(const this_type& l, const this_type& r)
    {
        return l.empty_intersect(r);
    }

    size_t hash_code() const
    {
        size_t seed = boost::hash_value(_bit_arr[0]);
        for (size_t i = 1; i != N; ++i) {
            boost::hash_combine(seed, _bit_arr[i]);
        }
        return seed;
    }

    static this_type off(const this_type& v, size_t i)
    {
        this_type rst(v);
        v.off(i);
        return rst;
    }

    static this_type&& off(this_type&& v, size_t i)
    {
        this_type rst = boost::forward<this_type>(v);
        v.off(i);
        return rst; // ROV
    }

    static this_type on(const this_type& v, size_t i)
    {
        this_type rst(v);
        v.on(i);
        return rst;
    }

    static this_type&& on(this_type&& v, size_t i)
    {
        this_type rst = boost::forward<this_type>(v);
        v.on(i);
        return rst;
    }

    // from low bit to high bit
    template<typename Func>
    void loop_mask(Func&& func) const
    {
        for (size_t i = 0, offset = 0; i != N; ++i, offset += E_bitwidth)
        {
            mask_type mask = _bit_arr[i];
            while (mask)
            {
                size_t pos = sss::bit::lowest_bit_pos(mask);
                assert(pos);
                // TODO check return value version, if needed
                func(pos - 1 + offset);

                sss::bit::bit_off(mask, pos - 1);
            }
        }
    }

    // from high bit to low bit
    template<typename Func>
    void loop_mask_reverse(Func&& func) const
    {
        for (size_t i = N, offset = E_bitsize - E_bitwidth; i; --i, offset -= E_bitwidth)
        {
            mask_type mask = _bit_arr[i - 1];
            while (mask)
            {
                size_t pos = sss::bit::highest_bit_pos(mask);
                assert(pos);
                // TODO check return value version, if needed
                func(pos - 1 + offset);

                sss::bit::bit_off(mask, pos - 1);
            }
        }
    }

private:
    bit_arr_type _bit_arr;
}; // bitmask<>

template<size_t N> inline bitmask<N> operator & (const bitmask<N>& l, const bitmask<N>& r) { bitmask<N> rst(l); rst &= r; return rst; }
template<size_t N> inline bitmask<N> operator | (const bitmask<N>& l, const bitmask<N>& r) { bitmask<N> rst(l); rst |= r; return rst; }

template<size_t N> inline bitmask<N> operator << (const bitmask<N>& l, const bitmask<N>& r) { bitmask<N> rst(l); rst <<= r; return rst; }
template<size_t N> inline bitmask<N> operator >> (const bitmask<N>& l, const bitmask<N>& r) { bitmask<N> rst(l); rst >>= r; return rst; }

} // namespace ECS
} // namespace sss

namespace boost {
template<size_t N>
inline std::size_t hash_value(const sss::ECS::bitmask<N>& o)
{
    return o.hash_code();
}
} // namespace boost

namespace std {
template<size_t N>
std::ostream& operator << (std::ostream& out, const sss::ECS::bitmask<N>& b)
{
    b.print(out);
    return out;
}
} // namespace std
