#ifndef RENOSTER_CAMERA_H_
#define RENOSTER_CAMERA_H_

#include <memory>

#include "renoster/bounds.h"
#include "renoster/export.h"
#include "renoster/paramlist.h"
#include "renoster/ray.h"
#include "renoster/sampler.h"
#include "renoster/transform.h"

namespace renoster {

struct CameraEnvironment {
    Bounds2f screen;
};

class RENO_API Camera {
public:
    Camera(const Transform & WorldToCamera, const Transform & CameraToWorld);

    virtual ~Camera() = default;

    ///
    virtual void RenderBegin(CameraEnvironment & env) = 0;

    ///
    virtual void RenderEnd() = 0;

    ///
    virtual float GenerateRay(Sampler & sampler, const Point2f & pScreen,
                              float time, Ray3f * ray) const = 0;

protected:
    Transform _WorldToCamera;
    Transform _CameraToWorld;
};

RENO_API std::unique_ptr<Camera> CreateCamera(const std::string & name,
                                              const ParameterList & params,
                                              const Transform & WorldToCamera,
                                              const Transform & CameraToWorld);

}  // namespace renoster

#endif  // RENOSTER_CAMERA_H_
