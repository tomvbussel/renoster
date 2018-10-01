#ifndef RENOSTER_UTIL_SPAN_H_
#define RENOSTER_UTIL_SPAN_H_

namespace renoster {

constexpr ptrdiff_t dynamic_extent = -1;

template <class T, ptrdiff_t Extent = dynamic_extent>
class span {
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using index_type = ptrdiff_t;
    using difference_type = ptrdiff_t;
    using pointer = T *;
    using reference = T &;
    using iterator = pointer;
    using const_iterator = const pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    static constexpr ptrdiff_t extent = Extent;

    constexpr span() noexcept
    {
    }

    constexpr span(pointer data, index_type size)
        : _data(data), _size(size)
    {
    }

    constexpr span(pointer firstElem, pointer lastElem)
        : _data(firstElem), _size(lastElem - firstElem)
    {
    }

    template <size_t N>
    constexpr span(element_type (&data)[N]) noexcept
        : _data(data), _size(N)
    {
    }

    template <size_t N>
    constexpr span(std::array<value_type, N> & arr) noexcept
        : _data(arr.data()), _size(N)
    {
    }

    template <size_t N>
    constexpr span(const std::array<value_type, N> & arr) noexcept
        : _data(arr.data()), _size(N)
    {
    }

    template <class Container>
    constexpr span(Container & cont)
        : _data(cont.data()), _size(cont.size())
    {
    }

    template <class Container>
    constexpr span(const Container & cont)
        : _data(cont.data()), _size(cont.size())
    {
    }

    template <class U, ptrdiff_t N>
    constexpr span(const span<U, N> & s)
        : _data(s._data), _size(s._size)
    {
    }

    constexpr span(const span & other) noexcept = default;

    constexpr span & operator=(const span & o) noexcept = default;

    constexpr iterator begin() const noexcept
    {
        return _data;
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return _data;
    }

    constexpr iterator end() const noexcept
    {
        return _data + _size;
    }

    constexpr const_iterator cend() const noexcept
    {
        return _data + _size;
    }

    constexpr reverse_iterator rbegin() const noexcept
    {
        return _data + _size - 1;
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return _data + _size - 1;
    }

    constexpr reverse_iterator rend() const noexcept
    {
        return _data - 1;
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
        return _data - 1;
    }

    constexpr reference operator[](index_type idx) const
    {
        return _data[idx];
    }

    constexpr reference operator()(index_type idx) const
    {
        return _data[idx];
    }

    constexpr pointer data() const noexcept
    {
        return _data;
    }

    constexpr index_type size() const noexcept
    {
        return _size;
    }

    constexpr index_type size_bytes() const noexcept
    {
        return _size * sizeof(element_type);
    }

    constexpr bool empty() const noexcept
    {
        return _size == 0;
    }

    template <ptrdiff_t Count>
    constexpr span<element_type, Count> first() const
    {
        return {_data, Count};
    }

    constexpr span<element_type, dynamic_extent> first(ptrdiff_t Count) const
    {
        return {_data, Count};
    }

    template <ptrdiff_t Count>
    constexpr span<element_type, Count> last() const
    {
        return {_data + _size - Count, Count};
    }

    constexpr span<element_type, dynamic_extent> last(ptrdiff_t Count) const
    {
        return {_data + _size - Count, Count};
    }

    template <ptrdiff_t Offset, ptrdiff_t Count = dynamic_extent>
    constexpr span<element_type, Count> subspan() const
    {
        return {_data + Offset, 0};
    }

    constexpr span<element_type, dynamic_extent>
    subspan(ptrdiff_t Offset, ptrdiff_t Count = dynamic_extent)
    {
        if (Count == dynamic_extent)
        {
            return {_data + Offset, _size - Offset};
        }
        else
        {
            return {_data + Offset, Count};
        }
    }

private:
    pointer _data = nullptr;
    index_type _size = 0;
};

} // namespace renoster

#endif // RENOSTER_UTIL_SPAN_H_
