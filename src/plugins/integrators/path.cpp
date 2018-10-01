#include "renoster/integrator.h"

namespace renoster {

class PathTracer : public Integrator {
public:
    PathTracer(int maxDepth, int rrDepth, int rrThreshold);

    void Integrate(IntegratorContext & ctx, const Ray3f & ray,
                   FilmAccumulator * accum) const;

private:
    void DirectLighting(IntegratorContext &ctx, const ShadingPoint &sp,
                        int numLightSamples, int numBsdfSamples,
                        FilmAccumulator * accum) const;

    int _maxDepth;
    int _rrDepth;
    float _rrThreshold;
    int _numLightSamples;
    int _numBsdfSamples;
};

PathTracer::PathTracer(int maxDepth, int rrDepth, int rrThreshold)
    : _maxDepth(maxDepth),
    _rrDepth(rrDepth),
    _rrThreshold(rrThreshold)
{
}

void PathTracer::Integrate(IntegratorContext & ctx, const Ray3f & r,
                           FilmAccumulator * accum) const
{
    Ray3f ray(r);
    Color throughput(1.f);

    for (int depth = 0; depth <= _maxDepth; ++depth) {
        // Intersect ray with scene
        ShadingPoint sp;
        if (!ctx.scene.Intersect(ray, &sp)) {
            return;
        }

        if (depth == 0) {
            float pdfEmit;
            Color Le = ctx.scene.EvaluateEmission(sp, &pdfEmit);
            accum->AddSample(Le);
        }

        // Compute scattering function
        sp.ComputeScatteringFunctions(ctx.alloc);
        if (!sp.bsdf) {
            return;
        }

        // Compute direct lighting
        int numLightSamples = 1; // TODO
        int numBsdfSamples = 1; // TODO
        DirectLighting(ctx, sp, numLightSamples, numBsdfSamples, accum);

        // Sample BSDF
        Vector3f wi;
        float pdfBsdf;
        throughput *= sp.bsdf->Sample(ctx.sampler, &wi, &pdfBsdf);
        if (pdfBsdf == 0.f || throughput.IsBlack()) {
            return;
        }

        // Russian roulette
        if (throughput.ChannelMax() < _rrThreshold && depth >= _rrDepth) {
            float q = std::max(0.05f, 1.f - throughput.ChannelMax());
            if (ctx.sampler.Get1D() < q) {
                return;
            }
            throughput /= 1.f - q;
        }

        ray = Ray3f(sp.p, wi, Epsilon, Infinity, ray.time());
    }
}

float AreaToSolidAngle(const ShadingPoint & ref, const ShadingPoint & pos,
                       float pdf)
{
    Vector3f wi = Normalize(pos.p - ref.p);
    pdf *= DistanceSquared(ref.p, pos.p) / std::abs(Dot(pos.ng, -wi));
    if (std::isinf(pdf)) pdf = 0.f;
    return pdf;
}

void PathTracer::DirectLighting(IntegratorContext & ctx,
                                const ShadingPoint & sp,
                                int numLightSamples,
                                int numBsdfSamples,
                                FilmAccumulator * accum) const
{
    for (int i = 0; i < numLightSamples; ++i) {
        ShadingPoint spLight;
        float pdfLight;
        Color Li = ctx.scene.SampleDirect(ctx.sampler, sp, &spLight, &pdfLight);
        if (pdfLight == 0.f || Li.IsBlack()) {
            continue;
        }
        pdfLight = AreaToSolidAngle(sp, spLight, pdfLight);
        
        Vector3f wi = Normalize(spLight.p - sp.p);
        float dist = Distance(spLight.p, sp.p);
        Ray3f shadowRay(sp.p, wi, 0.01f, dist - 0.01f, sp.time);
        if (ctx.scene.Occluded(shadowRay)) {
            continue;
        }

        float pdfBsdf;
        Color f = sp.bsdf->Evaluate(ctx.sampler, wi, &pdfBsdf);
        if (f.IsBlack()) {
            continue;
        }
        
        float weight = MISPowerHeuristic(numLightSamples, pdfLight,
                                         numBsdfSamples, pdfBsdf);

        accum->AddSample(weight * f * Li / (pdfLight * numLightSamples));
    }

    for (int i = 0; i < numBsdfSamples; ++i) {
        Vector3f wi;
        float pdfBsdf;
        Color f = sp.bsdf->Sample(ctx.sampler, &wi, &pdfBsdf);
        if (pdfBsdf == 0.f || f.IsBlack()) {
            continue;
        }

        Ray3f lightRay(sp.p, wi, 0.01f, Infinity, sp.time);
        ShadingPoint spLight;
        if (!ctx.scene.Intersect(lightRay, &spLight)) {
            continue;
        }

        float pdfLight;
        Color Li = ctx.scene.EvaluateDirect(sp, spLight, &pdfLight);
        if (Li.IsBlack()) {
            continue;
        }
        pdfLight = AreaToSolidAngle(sp, spLight, pdfLight);

        float weight = MISPowerHeuristic(numBsdfSamples, pdfBsdf,
                                         numLightSamples, pdfLight);

        accum->AddSample(weight * f * Li / numBsdfSamples);
    }
}

extern "C"
RENO_EXPORT
Integrator * CreateIntegrator(ParameterList & params)
{
    int defMaxDepth = 8;
    int maxDepth = params.GetInt("maxdepth", &defMaxDepth);

    int defRrDepth = 1;
    int rrDepth = params.GetInt("rrdepth", &defRrDepth);

    float defRrThreshold = 1.f;
    float rrThreshold = params.GetFloat("rrthreshold", &defRrThreshold);

    return new PathTracer(maxDepth, rrDepth, rrThreshold);
}

}  // namespace renoster
