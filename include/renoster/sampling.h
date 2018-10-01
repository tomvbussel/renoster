#ifndef RENOSTER_SAMPLING_H_
#define RENOSTER_SAMPLING_H_

#include <vector>

#include "renoster/export.h"
#include "renoster/frame.h"
#include "renoster/mathutil.h"
#include "renoster/point.h"
#include "renoster/vector.h"

namespace renoster {

class RENO_API Distribution1D {
public:
    Distribution1D() = default;

    Distribution1D(std::vector<float> func);

    int SampleDiscrete(float u, float * pdf, float * uRemapped) const;

    float PdfDiscrete(int index) const;

    float SampleContinuous(float u, float * pdf) const {
        int idx = SampleDiscrete(u, pdf, &u);
        return idx + u;
    }

    float PdfContinuous(float u) const {
        return PdfDiscrete((int)std::floor(u));
    }

    float Integral() const { return _funcInt; }

private:
    std::vector<float> _cdf;
    float _funcInt;
};

class RENO_API Distribution2D {
public:
    Distribution2D() = default;

    Distribution2D(std::vector<float> func, const Point2i & n);

    Point2i SampleDiscrete(Point2f uv, float * pdf, Point2f * uvRemapped) const;

    Point2f SampleContinuous(Point2f uv, float * pdf) const {
        Point2i idx = SampleDiscrete(uv, pdf, &uv);
        return idx + uv;
    }

private:
    Distribution1D _pMarginal;
    std::vector<Distribution1D> _pConditional;
};

RENO_API Point2f UniformSampleDisk(const Point2f & u);

RENO_API float UniformSampleDiskPdf(const Point2f & p);

RENO_API Vector3f UniformSampleSphere(const Point2f & u);

RENO_API float UniformSampleSpherePdf(const Vector3f & w);

RENO_API Point2f UniformSampleTriangle(const Point2f & u);

RENO_API float UniformSampleTrianglePdf(const Point2f & p);

RENO_API Vector3f CosineSampleHemisphere(const Point2f & u);

RENO_API float CosineSampleHemispherePdf(const Vector3f & w);

RENO_API float MISPowerHeuristic(int nF, float pdfF, int nG, float pdfG);

}  // namespace renoster

#endif  // RENOSTER_SAMPLING_H_
