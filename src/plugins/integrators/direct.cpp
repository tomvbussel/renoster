#include "renoster/integrator.h"

#include "renoster/sampling.h"

namespace renoster {

class DirectLighting : public Integrator {
public:
    DirectLighting(int numLightSamples, int numBsdfSamples);

    void Integrate(IntegratorContext & ctx, const Ray3f & ray,
                   FilmAccumulator * accum) const;

private:
    int _numLightSamples;
    int _numBsdfSamples;
};

float AreaToSolidAngle(const ShadingPoint & ref, const ShadingPoint & pos,
                       float pdf)
{
    Vector3f wi = Normalize(pos.p - ref.p);
    pdf *= DistanceSquared(ref.p, pos.p) / std::abs(Dot(pos.ng, -wi));
    if (std::isinf(pdf)) pdf = 0.f;
    return pdf;
}

DirectLighting::DirectLighting(int numLightSamples, int numBsdfSamples)
    : _numLightSamples(numLightSamples),
    _numBsdfSamples(numBsdfSamples)
{
}

void DirectLighting::Integrate(IntegratorContext & ctx, const Ray3f & ray,
                               FilmAccumulator * accum) const
{
    ShadingPoint sp;
    if (!ctx.scene.Intersect(ray, &sp)) {
        return;
    }

    float pdfEmit;
    Color Le = ctx.scene.EvaluateEmission(sp, &pdfEmit);
    accum->AddSample(Le);

    sp.ComputeScatteringFunctions(ctx.alloc);
    if (!sp.bsdf) {
        return;
    }
    
    for (int i = 0; i < _numLightSamples; ++i) {
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
        
        float weight = MISPowerHeuristic(_numLightSamples, pdfLight,
                                         _numBsdfSamples, pdfBsdf);

        accum->AddSample(weight * f * Li / (pdfLight * _numLightSamples));
    }

    for (int i = 0; i < _numBsdfSamples; ++i) {
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

        float weight = MISPowerHeuristic(_numBsdfSamples, pdfBsdf,
                                         _numLightSamples, pdfLight);

        accum->AddSample(weight * f * Li / _numBsdfSamples);
    }
}

extern "C"
RENO_EXPORT
Integrator * CreateIntegrator(ParameterList & params)
{
    int defNumLightSamples = 1;
    int numLightSamples = params.GetInt("numLightSamples", &defNumLightSamples);

    int defNuMBSDFSamples = 1;
    int numBSDFSamples = params.GetInt("numBSDFSamples", &defNuMBSDFSamples);

    return new DirectLighting(numLightSamples, numBSDFSamples);
}

}  // namespace renoster
