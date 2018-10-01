#include "renoster/sampling.h"

#include <algorithm>

namespace renoster {

Distribution1D::Distribution1D(std::vector<float> func)
{
    _cdf.resize(func.size() + 1);
    _cdf[0] = 0.f;
    for (size_t i = 0; i < func.size(); ++i) {
        _cdf[i + 1] = _cdf[i] + func[i];
    }

    _funcInt = _cdf.back();
    if (_funcInt != 0.f) {
        for (size_t i = 0; i < _cdf.size(); ++i) {
            _cdf[i] /= _funcInt;
        }
    } else {
        for (size_t i = 0; i < _cdf.size(); ++i) {
            _cdf[i] = float(i) / float(func.size());
        }
    }
}

int Distribution1D::SampleDiscrete(float u, float * pdf, float * uRemapped) const
{
    assert(u >= 0.f && u < 1.f);
    auto it = std::upper_bound(_cdf.begin(), _cdf.end(), u);
    size_t index = std::distance(_cdf.begin(), it) - 1;
    assert(index < _cdf.size() - 1);
    assert(u >= _cdf[index] && u < _cdf[index + 1]);

    if (uRemapped) {
        *uRemapped = (u - _cdf[index]) / (_cdf[index + 1] - _cdf[index]);
    }

    if (pdf) {
        *pdf = _cdf[index + 1] - _cdf[index];
    }

    return index;
}

float Distribution1D::PdfDiscrete(int index) const
{
    return _cdf[index + 1] - _cdf[index];
}

Distribution2D::Distribution2D(std::vector<float> func, const Point2i & n)
{
    _pConditional.resize(n.y());
    std::vector<float> marginalFunc(n.y());
    for (int j = 0; j < n.y(); ++j) {
        std::vector<float> conditionalFunc(n.x());
        std::copy_n(func.begin() + j * n.x(), n.x(), conditionalFunc.begin());
        _pConditional[j] = Distribution1D(std::move(conditionalFunc));
        marginalFunc[j] = _pConditional[j].Integral();
    }
    _pMarginal = Distribution1D(std::move(marginalFunc));
}

Point2i Distribution2D::SampleDiscrete(Point2f uv, float * pdf, Point2f * uvRemapped) const
{
    float pdfs[2];
    int i = _pMarginal.SampleDiscrete(uv.x(), &pdfs[0], &uv.x());
    int j = _pConditional[i].SampleDiscrete(uv.y(), &pdfs[1], &uv.y());

    if (pdf) {
        *pdf = pdfs[0] * pdfs[1];
    }

    if (uvRemapped) {
        *uvRemapped = uv;
    }

    return Point2i(i, j);
}

Point2f UniformSampleDisk(const Point2f & u)
{
    float r = std::sqrt(u.x());
    float theta = 2.f * Pi * u.y();
    return Point2f(r * std::cos(theta), r * std::sin(theta));
}

float UniformSampleDiskPdf(const Point2f &)
{
    return InvPi;
}

Vector3f UniformSampleSphere(const Point2f & u)
{
    float z = 1.f - 2.f * u.x();
    float r = std::sqrt(std::max(0.f, 1.f - z * z));
    float phi = TwoPi * u.y();
    return Vector3f(r * std::cos(phi), r * std::sin(phi), z);
}

float UniformSampleSpherePdf(const Vector3f & w)
{
    return InvFourPi;
}

Point2f UniformSampleTriangle(const Point2f & u)
{
    float a = std::sqrt(u.x());
    return Point2f(1.f - a, u.y() * a);
}

float UniformSampleTrianglePdf(const Point2f &)
{
    return 0.5f;
}

Vector3f CosineSampleHemisphere(const Point2f & u)
{
    Point2f d = UniformSampleDisk(u);
    float z = std::sqrt(std::max(0.f, 1.f - d.x() * d.x() - d.y() * d.y()));
    return Vector3f(d.x(), d.y(), z);
}

float CosineSampleHemispherePdf(const Vector3f & w)
{
    Point2f d(w.x(), w.y());
    return CosTheta(w) * UniformSampleDiskPdf(d);
}

RENO_API float MISPowerHeuristic(int nF, float pdfF, int nG, float pdfG)
{
    float f = nF * pdfF, g = nG * pdfG;
    return (f * f) / (f * f + g * g);
}

} // namespace renoster
