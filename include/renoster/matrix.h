#ifndef RENOSTER_MATH_MATRIX_H_
#define RENOSTER_MATH_MATRIX_H_

#include <array>
#include <iostream>
#include <cassert>

#include "renoster/normal.h"
#include "renoster/point.h"
#include "renoster/vector.h"

namespace renoster {

template <size_t N, size_t M, typename T>
class Matrix {
public:
    using Scalar = T;
    static constexpr size_t Rows = N;
    static constexpr size_t Cols = M;

    Matrix() : _m{} {};

    explicit Matrix(Scalar a) {
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                _m[i][j] = a;
            }
        }
    }

    Matrix(float mat[N][M]) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                _m[i][j] = mat[i][j];
            }
        }
    }

    Matrix(const Matrix &) = default;
    Matrix & operator=(const Matrix &) = default;

    Matrix(Matrix &&) = default;
    Matrix & operator=(Matrix &&) = default;

    template <typename S>
    explicit Matrix(const Matrix<N, M, S> & m) {
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                _m[i][j] = m(i, j);
            }
        }
    }

    template <typename S>
    Matrix & operator=(const Matrix<N, M, S> & m) {
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                _m[i][j] = m(i, j);
            }
        }
        return *this;
    }

    const Scalar & operator()(size_t i, size_t j) const { return _m[i][j]; }

    Scalar & operator()(size_t i, size_t j) { return _m[i][j]; }

    template <typename S>
    Matrix & operator*=(S c) {
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                _m[i][j] *= c;
            }
        }
        return *this;
    }

    template <typename S>
    Matrix & operator/=(S c) {
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                _m[i][j] /= c;
            }
        }
        return *this;
    }

    template <typename S>
    Matrix & operator+=(const Matrix<N, M, S> & rhs) {
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                _m[i][j] += rhs._m[i][j];
            }
        }
        return *this;
    }

    template <typename S>
    Matrix & operator-=(const Matrix<N, M, S> & rhs) {
        for (size_t i = 0; i < Rows; ++i) {
            for (size_t j = 0; j < Cols; ++j) {
                _m[i][j] -= rhs._m[i][j];
            }
        }
        return *this;
    }

    template <typename S>
    Matrix & operator*=(const Matrix<M, M, S> & rhs) {
        assert(false);  // TODO
        return *this;
    }

private:
    std::array<std::array<Scalar, Cols>, Rows> _m;
};

using Matrix2x2f = Matrix<2, 2, float>;
using Matrix4x4f = Matrix<4, 4, float>;

template <size_t N, size_t M, typename T>
Matrix<N, M, T> operator-(const Matrix<N, M, T> & m) {
    Matrix<N, M, T> result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            result(i, j) = -m(i, j);
        }
    }
    return result;
}

template <size_t N, size_t M, typename T, typename S>
Matrix<N, M, decltype(T() * S())> operator*(const Matrix<N, M, T> & m, S c) {
    Matrix<N, M, decltype(T() * S())> result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            result(i, j) = m(i, j) * c;
        }
    }
    return result;
}

template <size_t N, size_t M, typename T, typename S>
Matrix<N, M, decltype(T() * S())> operator*(T c, const Matrix<N, M, S> & m) {
    Matrix<N, M, decltype(T() * S())> result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            result(i, j) = c * m(i, j);
        }
    }
    return result;
}

template <size_t N, size_t M, typename T, typename S>
Matrix<N, M, decltype(T() / S())> operator/(const Matrix<N, M, T> & m, S c) {
    Matrix<N, M, decltype(T() / S())> result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            result(i, j) = m(i, j) / c;
        }
    }
    return result;
}

template <size_t N, size_t M, typename T, typename S>
Matrix<N, M, decltype(T() + S())> operator+(const Matrix<N, M, T> & lhs,
                                            const Matrix<N, M, S> & rhs) {
    Matrix<N, M, decltype(T() + S())> result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            result(i, j) = lhs(i, j) + rhs(i, j);
        }
    }
    return result;
}

template <size_t N, size_t M, typename T, typename S>
Matrix<N, M, decltype(T() - S())> operator-(const Matrix<N, M, T> & lhs,
                                            const Matrix<N, M, S> & rhs) {
    Matrix<N, M, decltype(T() - S())> result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            result(i, j) = lhs(i, j) - rhs(i, j);
        }
    }
    return result;
}

template <size_t N, size_t M, size_t K, typename T, typename S>
Matrix<N, K, decltype(T() * S())> operator*(const Matrix<N, M, T> & lhs,
                                            const Matrix<M, K, S> & rhs) {
    Matrix<N, K, decltype(T() * S())> result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t k = 0; k < K; ++k) {
            decltype(T() * S()) sum = 0;
            for (size_t j = 0; j < M; ++j) {
                sum += lhs(i, j) * rhs(j, k);
            }
            result(i, k) = sum;
        }
    }
    return result;
}

template <size_t N, typename T>
Point<N - 1, T> operator*(const Matrix<N, N, T> & m,
                          const Point<N - 1, T> & p) {
    Point<N - 1, T> result;
    for (size_t i = 0; i < N - 1; ++i) {
        T sum = 0;
        for (size_t j = 0; j < N - 1; ++j) {
            sum += m(i, j) * p[j];
        }
        result[i] = sum + m(i, N - 1);
    }
    T w = 0;
    for (size_t j = 0; j < N - 1; ++j) {
        w += m(N - 1, j) * p[j];
    }
    result /= w + m(N - 1, N - 1);
    return result;
}

template <typename T>
Point<3, T> operator*(const Matrix<4, 4, T> & m, const Point<3, T> & p) {
    T x = m(0, 0) * p[0] + m(0, 1) * p[1] + m(0, 2) * p[2] + m(0, 3);
    T y = m(1, 0) * p[0] + m(1, 1) * p[1] + m(1, 2) * p[2] + m(1, 3);
    T z = m(2, 0) * p[0] + m(2, 1) * p[1] + m(2, 2) * p[2] + m(2, 3);
    T w = m(3, 0) * p[0] + m(3, 1) * p[1] + m(3, 2) * p[2] + m(3, 3);
    return Point<3, T>(x / w, y / w, z / w);
}

template <size_t N, typename T>
Vector<N - 1, T> operator*(const Matrix<N, N, T> & m,
                           const Vector<N - 1, T> & v) {
    Vector<N - 1, T> result;
    for (size_t i = 0; i < N - 1; ++i) {
        T sum = 0;
        for (size_t j = 0; j < N - 1; ++j) {
            sum += m(i, j) * v[j];
        }
        result[i] = sum;
    }
    return result;
}

template <typename T>
Vector<3, T> operator*(const Matrix<4, 4, T> & m, const Vector<3, T> & v) {
    T x = m(0, 0) * v[0] + m(0, 1) * v[1] + m(0, 2) * v[2];
    T y = m(1, 0) * v[0] + m(1, 1) * v[1] + m(1, 2) * v[2];
    T z = m(2, 0) * v[0] + m(2, 1) * v[1] + m(2, 2) * v[2];
    return Vector<3, T>(x, y, z);
}

template <size_t N, typename T>
Normal<N - 1, T> operator*(const Matrix<N, N, T> & m,
                           const Normal<N - 1, T> & n) {
    Normal<N - 1, T> result;
    for (size_t i = 0; i < N - 1; ++i) {
        T sum = 0;
        for (size_t j = 0; j < N - 1; ++j) {
            sum += m(j, i) * n[j];
        }
        result[i] = sum;
    }
    return result;
}

template <typename T>
Normal<3, T> operator*(const Matrix<4, 4, T> & m, const Normal<3, T> & n) {
    T x = m(0, 0) * n[0] + m(1, 0) * n[1] + m(2, 0) * n[2];
    T y = m(0, 1) * n[0] + m(1, 1) * n[1] + m(2, 1) * n[2];
    T z = m(0, 2) * n[0] + m(1, 2) * n[1] + m(2, 2) * n[2];
    return Normal<3, T>(x, y, z);
}

template <size_t N, size_t M, typename T, typename S>
bool operator==(const Matrix<N, M, T> & lhs, const Matrix<N, M, S> & rhs) {
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            if (lhs(i, j) != rhs(i, j)) {
                return false;
            }
        }
    }
    return true;
}

template <size_t N, size_t M, typename T, typename S>
bool operator!=(const Matrix<N, M, T> & lhs, const Matrix<N, M, S> & rhs) {
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            if (lhs(i, j) == rhs(i, j)) {
                return false;
            }
        }
    }
    return true;
}

template <size_t N, size_t M, typename T>
Matrix<N, M, T> Lerp(const Matrix<N, M, T> & m1, const Matrix<N, M, T> & m2,
                     T t) {
    Matrix<N, M, T> result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < M; ++j) {
            result(i, j) = Lerp(m1(i, j), m2(i, j), t);
        }
    }
    return result;
}

template <size_t N, size_t M, typename T>
Matrix<M, N, T> Transpose(const Matrix<N, M, T> & m) {
    Matrix<M, N, T> result;
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            result(j, i) = m(i, j);
        }
    }
    return result;
}

template <size_t N, typename T>
Matrix<N, N, T> Inverse(Matrix<N, N, T> m) {
    std::array<size_t, N> indxc{}, indxr{}, ipiv{};

    for (size_t i = 0; i < N; ++i) {
        // Choose a pivot
        size_t irow = 0;
        size_t icol = 0;
        T big = 0;
        for (size_t j = 0; j < N; ++j) {
            if (ipiv[j] != 1) {
                for (size_t k = 0; k < N; k++) {
                    if (ipiv[k] == 0) {
                        if (std::abs(m(j, k)) >= big) {
                            big = std::abs(m(j, k));
                            irow = j;
                            icol = k;
                        }
                    } else if (ipiv[k] > 1) {
                        // TODO: ERROR: Singular matrix
                    }
                }
            }
        }
        ++ipiv[icol];

        // Swap rows to place pivot on the diagonal
        if (irow != icol) {
            for (size_t j = 0; j < N; ++j) {
                std::swap(m(irow, j), m(icol, j));
            }
        }
        indxr[i] = irow;
        indxc[i] = icol;

        // Make sure the pivot is non-zero
        if (m(icol, icol) == 0) {
            // TODO: Error: singular matrix
        }

        // Divide row by pivot
        T pivinv = T(1) / m(icol, icol);
        m(icol, icol) = 1;
        for (size_t j = 0; j < 4; ++j) {
            m(icol, j) *= pivinv;
        }

        // Subtract the pivot row from the other rows
        for (size_t j = 0; j < N; ++j) {
            if (j != icol) {
                T save = m(j, icol);
                m(j, icol) = 0;
                for (size_t k = 0; k < N; ++k) {
                    m(j, k) -= m(icol, k) * save;
                }
            }
        }
    }

    // Swap the columns back
    for (size_t i = N; i-- > 0;) {
        if (indxr[i] != indxc[i]) {
            for (size_t j = 0; j < N; ++j) {
                std::swap(m(j, indxr[i]), m(j, indxc[i]));
            }
        }
    }

    return m;
}

}  // namespace renoster

#endif  // RENOSTER_MATH_MATRIX_H_
