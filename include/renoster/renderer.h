#ifndef RENOSTER_RENDERER_H_
#define RENOSTER_RENDERER_H_

#include "renoster/camera.h"
#include "renoster/export.h"
#include "renoster/film.h"
#include "renoster/integrator.h"
#include "renoster/sampler.h"
#include "renoster/scene.h"

namespace renoster {

class RENO_API Renderer
{
public:
    Renderer(Camera * camera, Film * film, Integrator * integrator,
             Sampler * sampler);

    void Render(const Scene & scene);

private:
    void RenderThread(const Scene & scene, int threadId);

    Camera * _camera;
    Film * _film;
    Integrator * _integrator;
    Sampler * _sampler;
};

} // namespace renoster

#endif // RENOSTER_RENDERER_H_
