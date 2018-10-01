#include <cmath>

#include "renoster/camera.h"
#include "renoster/export.h"
#include "renoster/mathutil.h"
#include "renoster/paramlist.h"

namespace renoster {

class PinholeCamera : public Camera {
public:
    PinholeCamera(const Transform & WorldToCamera,
                  const Transform & CameraToWorld, float fov);

    float GenerateRay(Sampler & sampler, const Point2f & pScreen, float time,
                      Ray3f * ray) const;

    void RenderBegin(CameraEnvironment & env);

    void RenderEnd();

private:
    float _fov;
    float _zoom;
};

PinholeCamera::PinholeCamera(const Transform & WorldToCamera,
                             const Transform & CameraToWorld, float fov)
    : Camera(WorldToCamera, CameraToWorld),
    _fov(fov)
{
    _zoom = std::tan(0.5f * _fov * DegToRad);
}

float PinholeCamera::GenerateRay(Sampler & /* sampler */,
                                 const Point2f & pScreen, float time,
                                 Ray3f * ray) const
{
    ray->o() = Point3f(0.f, 0.f, 0.f);
    ray->d() = Normalize(Vector3f(pScreen.x() * _zoom,
                                  pScreen.y() * _zoom,
                                  1.f));
    ray->time() = time;
    ray->tMin() = Epsilon;
    ray->tMax() = Infinity;

    *ray = _CameraToWorld(*ray);

    return 1.f;
}

void PinholeCamera::RenderBegin(CameraEnvironment &)
{
}

void PinholeCamera::RenderEnd()
{
}

extern "C"
RENO_EXPORT
Camera * CreateCamera(const ParameterList & params,
                      const Transform & WorldToCamera,
                      const Transform & CameraToWorld)
{
    float defaultFov = 90.f;
    float fov = params.GetFloat("fov", &defaultFov);

    return new PinholeCamera(WorldToCamera, CameraToWorld, fov);
};

} // namespace renoster
