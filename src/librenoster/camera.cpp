#include "renoster/camera.h"

namespace renoster {

Camera::Camera(const Transform & WorldToCamera,
               const Transform & CameraToWorld)
    : _WorldToCamera(WorldToCamera),
    _CameraToWorld(CameraToWorld)
{
}

}  // namespace renoster
