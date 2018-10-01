#include "renoster/renderer.h"

#include <iostream>
#include "renoster/log.h"

#include <memory>
#include <thread>
#include <vector>

#include "renoster/camera.h"
#include "renoster/filmaccumulator.h"

namespace renoster {

Renderer::Renderer(Camera * camera, Film * film, Integrator * integrator,
                   Sampler * sampler)
    : _camera(camera),
    _film(film),
    _integrator(integrator),
    _sampler(sampler)
{
}

void Renderer::RenderThread(const Scene & scene, int threadId)
{
    FilmAccumulator accum;
    Allocator alloc;

    while(auto tile = _film->GetNextTile()) {
        Bounds2i tileBounds = tile->GetSampleBounds();

        int tileId = tile->GetTileId();
        std::unique_ptr<Sampler> tileSampler = _sampler->Clone(tileId);

        IntegratorContext ctx(scene, *tileSampler, alloc);

        for (Point2i pixel : tileBounds) {
            tileSampler->StartPixel(pixel);

            while(tileSampler->StartNextSample()) {

                // Sample either the pixel, or the pixel's filter
                float pdf;
                Point2f pSample = tile->Sample(pixel, tileSampler.get(), &pdf);

                // Generate a ray for the pixel sample
                Point2f pScreen = _film->RasterToScreen(pSample);
                Ray3f ray;
                float time = tileSampler->Get1D();
                float weight = _camera->GenerateRay(*tileSampler, pScreen,
                                                    time, &ray);

                // Include film and camera sample weights
                Color result;
                accum.GetValue(result);
                result *= weight / pdf;
                accum.WriteValue(result);

                // Integrate the ray
                _integrator->Integrate(ctx, ray, &accum);

                // Add contribution to tile
                tile->AddSample(pixel, pSample, accum);
                accum.Reset();

                alloc.Reset();
            }
        }

        _film->MergeFilmTile(std::move(tile));
    }
}

void Renderer::Render(const Scene & scene)
{
    int numThreads = 1;//8;
    
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(&Renderer::RenderThread, this, std::ref(scene), i);
    }
    for (int i = 0; i < numThreads; ++i) {
        threads[i].join();
    }
}

} // namespace renoster
