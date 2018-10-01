#include "renoster/scene.h"

#include "renoster/accel/builder.h"
#include "renoster/accel/splitter.h"

namespace renoster {

Scene::Scene(std::vector<Primitive *> geometries,
             std::vector<Primitive *> lights)
    : _lights(std::move(lights))
{
    // Build a BVH over the instanced geometries in the scene
    _bvh = std::make_unique<BVH>();
    ObjectSplitter splitter;
    size_t minLeafSize = 1;
    BVHBuilder<Primitive, ObjectSplitter> builder(_bvh.get(), splitter,
                                                  minLeafSize);
    PrimitiveContext pCtx;
    builder.Build(pCtx, geometries);

    // Build a distribution over the instanced lights in the scene
    if (!_lights.empty()) {
        std::vector<float> prob(_lights.size(), 1.f);
        _lightDistrib = Distribution1D(std::move(prob));
    }
}

bool Scene::Intersect(const Ray3f & ray, ShadingPoint * sp) const
{
    PrimitiveContext ctx;
    return _bvh->Intersect<Primitive, PrimitiveContext>(ctx, ray, sp);
}

bool Scene::Occluded(const Ray3f & ray) const
{
    PrimitiveContext ctx;
    return _bvh->Occluded<Primitive, PrimitiveContext>(ctx, ray);
}

Color Scene::SampleDirect(Sampler & sampler, const ShadingPoint & ref,
                          ShadingPoint * pos, float * pdf) const
{
    if (_lights.empty()) {
        *pdf = 0.f;
        return Color(0.f);
    }

    // Select a light
    float lightPdf;
    float u = sampler.Get1D();
    size_t index = _lightDistrib.SampleDiscrete(u, &lightPdf, &u);

    // Sample a point on the light
    float directPdf;
    PrimitiveContext ctx;
    Color L = _lights[index]->SampleDirect(ctx, sampler, ref, pos, &directPdf);

    *pdf = lightPdf * directPdf;
    return L;
}

Color Scene::EvaluateDirect(const ShadingPoint & ref, const ShadingPoint & pos,
                            float * pdf) const
{
    if (_lights.empty()) {
        *pdf = 0.f;
        return Color(0.f);
    }

    // Get the pdf for selecting the light
    size_t lightId = pos.primitive->GetLightId();
    if (lightId == -1) {
        *pdf = 0.f;
        return Color(0.f);
    }
    float lightPdf = _lightDistrib.PdfDiscrete(lightId);

    // Evaluate the light at the shading point
    float directPdf;
    PrimitiveContext ctx;
    Color L = pos.primitive->EvaluateDirect(ctx, ref, pos, &directPdf);

    *pdf = lightPdf * directPdf;
    return L;
}

Color Scene::SampleEmission(Sampler & sampler, ShadingPoint * pos,
                            float * pdf) const
{
    if (_lights.empty()) {
        *pdf = 0.f;
        return Color(0.f);
    }

    // Select a light
    float lightPdf;
    float u = sampler.Get1D();
    size_t index = _lightDistrib.SampleDiscrete(u, &lightPdf, &u);

    // Sample a point on the light
    float directPdf;
    PrimitiveContext ctx;
    Color L = _lights[index]->SampleEmission(ctx, sampler, pos, &directPdf);

    *pdf = lightPdf * directPdf;
    return L;
}

Color Scene::EvaluateEmission(const ShadingPoint & pos, float * pdf) const
{
    if (_lights.empty()) {
        *pdf = 0.f;
        return Color(0.f);
    }

    // Get the pdf for selecting the light
    size_t lightId = pos.primitive->GetLightId();
    if (lightId == -1) {
        *pdf = 0.f;
        return Color(0.f);
    }
    float lightPdf = _lightDistrib.PdfDiscrete(lightId);

    // Evaluate the light at the shading point
    float directPdf;
    PrimitiveContext ctx;
    Color L = pos.primitive->EvaluateEmission(ctx, pos, &directPdf);

    *pdf = lightPdf * directPdf;
    return L;
}

} // namespace renoster
