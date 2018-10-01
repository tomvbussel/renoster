#ifndef RENOSTER_RENO_H_
#define RENOSTER_RENO_H_

#include <string>

#include "renoster/export.h"
#include "renoster/paramlist.h"

namespace renoster {

RENO_API void RenoAttributeBegin();
RENO_API void RenoAttributeEnd();
RENO_API void RenoBegin();
RENO_API void RenoCamera(const std::string & name, ParameterList & params);
RENO_API void RenoDisplay(const std::string & name, ParameterList & params);
RENO_API void RenoEnd();
RENO_API void RenoFilm(ParameterList & params);
RENO_API void RenoGeometry(const std::string & name, ParameterList & params);
RENO_API void RenoGeometryLight(const std::string & name, ParameterList & params);
RENO_API void RenoIdentity();
RENO_API void RenoIntegrator(const std::string & name, ParameterList & params);
RENO_API void RenoLight(const std::string & name, ParameterList & params);
RENO_API void RenoLookAt(float ex, float ey, float ez, float lx, float ly,
                         float lz, float ux, float uy, float uz);
RENO_API void RenoObjectBegin(const std::string & handle);
RENO_API void RenoObjectEnd();
RENO_API void RenoOrthographic(float zNear, float zFar);
RENO_API void RenoMaterial(const std::string & name, ParameterList & params);
RENO_API void RenoPerspective(float fov, float zNear, float zFar);
RENO_API void RenoPixelFilter(const std::string & name, ParameterList & params);
RENO_API void RenoRotate(float angle, float dx, float dy, float dz);
RENO_API void RenoSampler(const std::string & name, ParameterList & params);
RENO_API void RenoScale(float sx, float sy, float sz);
RENO_API void RenoTransformBegin();
RENO_API void RenoTransformEnd();
RENO_API void RenoTranslate(float dx, float dy, float dz);
RENO_API void RenoWorldBegin();
RENO_API void RenoWorldEnd();

}  // namespace renoster

#endif  // RENOSTER_RENO_H_
