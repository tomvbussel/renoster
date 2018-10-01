#ifndef RENOSTER_MATH_COLOR_H_
#define RENOSTER_MATH_COLOR_H_

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace renoster {

class Color {
public:
    Color() : _r(0.f), _g(0.f), _b(0.f) {}

    explicit Color(float val) : _r(val), _g(val), _b(val) {}

    Color(float r, float g, float b) : _r(r), _g(g), _b(b) {}

    Color(const Color &) = default;
    Color & operator=(const Color &) = default;

    Color(Color &&) = default;
    Color & operator=(Color &&) = default;

    float r() const { return _r; }

    float & r() { return _r; }

    float g() const { return _g; }

    float & g() { return _g; }

    float b() const { return _b; }

    float & b() { return _b; }

    float operator[](size_t i) const { return (&_r)[i]; }

    float & operator[](size_t i) { return (&_r)[i]; }

    Color & operator+=(const Color & rhs) {
        _r += rhs._r;
        _g += rhs._g;
        _b += rhs._b;
        return *this;
    }

    friend Color operator+(Color lhs, const Color & rhs) {
        lhs += rhs;
        return lhs;
    }

    Color & operator-=(const Color & rhs) {
        _r -= rhs._r;
        _g -= rhs._g;
        _b -= rhs._b;
        return *this;
    }

    friend Color operator-(Color lhs, const Color & rhs) {
        lhs -= rhs;
        return lhs;
    }

    Color & operator*=(const Color & rhs) {
        _r *= rhs._r;
        _g *= rhs._g;
        _b *= rhs._b;
        return *this;
    }

    friend Color operator*(Color lhs, const Color & rhs) {
        lhs *= rhs;
        return lhs;
    }

    Color & operator/=(const Color & rhs) {
        _r /= rhs._r;
        _g /= rhs._g;
        _b /= rhs._b;
        return *this;
    }

    friend Color operator/(Color lhs, const Color & rhs) {
        lhs /= rhs;
        return lhs;
    }

    Color & operator*=(float rhs) {
        _r *= rhs;
        _g *= rhs;
        _b *= rhs;
        return *this;
    }

    friend Color operator*(Color lhs, float rhs) {
        lhs *= rhs;
        return lhs;
    }

    friend Color operator*(float lhs, Color rhs) {
        rhs *= lhs;
        return rhs;
    }

    Color & operator/=(float rhs) {
        float inv = 1.f / rhs;
        _r *= inv;
        _g *= inv;
        _b *= inv;
        return *this;
    }

    friend Color operator/(Color lhs, float rhs) {
        lhs /= rhs;
        return lhs;
    }

    bool IsBlack() const { return _r == 0.f && _g == 0.f && _b == 0.f; }

    float ChannelMin() const { return std::min({_r, _g, _b}); }

    float ChannelMax() const { return std::max({_r, _g, _b}); }

    float ChannelAvg() const { return (_r + _g + _b) / 3.f; }

    friend bool operator==(const Color & lhs, const Color & rhs) {
        return lhs._r == rhs._r && lhs._g == rhs._g && lhs._b == rhs._b;
    }

    friend bool operator!=(const Color & lhs, const Color & rhs) {
        return lhs._r != rhs._r || lhs._g != rhs._g || lhs._b != rhs._b;
    }

private:
    float _r, _g, _b;
};

inline Color Sqrt(const Color & c) {
    return Color(std::sqrt(c.r()), std::sqrt(c.g()), std::sqrt(c.b()));
}

}  // namespace renoster

#endif  // RENOSTER_MATH_COLOR_H_
