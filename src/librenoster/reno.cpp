#include "renoster/reno.h"

#include <iostream>

#include <memory>
#include <stack>
#include <vector>

#include "renoster/camera.h"
#include "renoster/display.h"
#include "renoster/film.h"
#include "renoster/geometry.h"
#include "renoster/integrator.h"
#include "renoster/light.h"
#include "renoster/log.h"
#include "renoster/material.h"
#include "renoster/pixelfilter.h"
#include "renoster/plugin.h"
#include "renoster/primitive.h"
#include "renoster/renderer.h"
#include "renoster/sampler.h"
#include "renoster/scene.h"
#include "renoster/transform.h"

namespace renoster {

enum class RenoState {
    kUninitialized,
    kOptions,
    kWorld
};

struct Options {
    std::unique_ptr<Camera> camera;
    std::unique_ptr<Display> display;
    std::unique_ptr<Film> film;
    std::unique_ptr<PixelFilter> filter;
    std::unique_ptr<Integrator> integrator;
    std::unique_ptr<Sampler> sampler;

    void Clear() {
        display.reset();
        film.reset();
        filter.reset();
        integrator.reset();
        sampler.reset();
    }
};

struct Attributes {
    // Material
    std::shared_ptr<Material> material;
    
    // Light source
    std::string lightName;
    ParameterList lightParams;
};

struct World {
    std::vector<std::unique_ptr<Primitive>> primitives;
    std::vector<Primitive *> geometries;
    std::vector<Primitive *> lights;

    void Clear() {
        primitives.clear();
        geometries.clear();
        lights.clear();
    }
};

static RenoState state;
static Options options;
static std::stack<Transform> transformStack;
static Transform curTransform;
static std::stack<Attributes> attributesStack;
static Attributes curAttributes;
static World world;

void RenoBegin()
{
    if (state != RenoState::kUninitialized) {
        Error("RenoBegin()");
        return;
    }

    // TODO: Create defaults for options
    
    curTransform = Identity();

    state = RenoState::kOptions;
}

void RenoEnd()
{
    if (state != RenoState::kOptions) {
        Error("RenoEnd()");
        return;
    }

    options.Clear();
    
    state = RenoState::kUninitialized;
}

void RenoWorldBegin()
{
    if (state != RenoState::kOptions) {
        Error("RenoWorldBegin()");
        return;
    }

    curTransform = Identity();
    state = RenoState::kWorld;
}

void RenoWorldEnd()
{
    if (state != RenoState::kWorld) {
        Error("RenoWorldEnd()");
        return;
    }

    // Prepare for rendering
    options.film->RenderBegin(options.filter.get(), options.display.get());
    CameraEnvironment camEnv{options.film->GetScreenWindow()};
    options.camera->RenderBegin(camEnv);

    // Render the current scene
    Renderer renderer(options.camera.get(), options.film.get(),
                      options.integrator.get(), options.sampler.get());
    Scene scene(world.geometries, world.lights);
    renderer.Render(scene);

    // Finish rendering
    options.camera->RenderEnd();
    options.film->RenderEnd();

    world.Clear();
    state = RenoState::kOptions;
}

void RenoAttributeBegin()
{
    if (state != RenoState::kWorld) {
        Error("RenoAttributeBegin()");
        return;
    }

    transformStack.push(curTransform);
    attributesStack.push(curAttributes);
}

void RenoAttributeEnd()
{
    if (state != RenoState::kWorld) {
        Error("RenoAttributeEnd()");
        return;
    }

    if (transformStack.empty() || attributesStack.empty()) {
        Error("RenoAttributeEnd()");
        return;
    }

    curTransform = transformStack.top();
    transformStack.pop();
    curAttributes = attributesStack.top();
    attributesStack.pop();
}

void RenoObjectBegin(const std::string & handle)
{
    Error("RenoObjectBegin() is not implemented");
}

void RenoObjectEnd() {
    Error("RenoObjectEnd() is not implemented");
}

void RenoTransformBegin() {
    if (state != RenoState::kWorld) {
        Error("RenoTransformBegin()");
        return;
    }

    transformStack.push(curTransform);
}

void RenoTransformEnd() {
    if (state != RenoState::kWorld) {
        Error("RenoTransformEnd()");
        return;
    }

    if (transformStack.empty()) {
        Error("RenoTransformEnd()");
        return;
    }

    curTransform = transformStack.top();
    transformStack.pop();
}

void RenoIdentity()
{
    curTransform = Identity();
}

void RenoLookAt(float ex, float ey, float ez, float lx, float ly, float lz,
                float ux, float uy, float uz)
{
    curTransform = curTransform * LookAt(Point3f(ex, ey, ez),
                                         Point3f(lx, ly, lz),
                                         Vector3f(ux, uy, uz));
}

void RenoOrthographic(float zNear, float zFar)
{
    curTransform = curTransform * Orthographic(zNear, zFar);
}

void RenoPerspective(float fov, float zNear, float zFar)
{
    curTransform = curTransform * Perspective(fov, zNear, zFar);
}

void RenoRotate(float angle, float dx, float dy, float dz)
{
    curTransform = curTransform * Rotate(angle, Vector3f(dx, dy, dz));
}

void RenoScale(float sx, float sy, float sz)
{
    curTransform = curTransform * Scale(sx, sy, sz);
}

void RenoTranslate(float dx, float dy, float dz)
{
    curTransform = curTransform * Translate(dx, dy, dz);
}

void RenoCamera(const std::string & name, ParameterList & params)
{
    if (state != RenoState::kOptions) {
        Error("RenoCamera()");
        return;
    }

    // Create a camera
    Transform WorldToCamera = curTransform;
    Transform CameraToWorld = Inverse(curTransform);
    options.camera = CreateCamera(name, params, WorldToCamera, CameraToWorld);
}

void RenoDisplay(const std::string & name, ParameterList & params)
{
    if (state != RenoState::kOptions) {
        Error("RenoDisplay()");
        return;
    }

    options.display = CreateDisplay(name, params);
}

void RenoFilm(ParameterList & params)
{
    if (state != RenoState::kOptions) {
        Error("RenoFilm()");
        return;
    }

    options.film = CreateFilm(params);
}

void RenoGeometry(const std::string & name, ParameterList & params)
{
    if (state != RenoState::kWorld) {
        Error("RenoGeometry()");
        return;
    }

    //
    std::shared_ptr<Geometry> geometry = CreateGeometry(name, params);

    //
    std::shared_ptr<GeometryLight> light;
    if (!curAttributes.lightName.empty()) {
        light = CreateGeometryLight(curAttributes.lightName,
                                    curAttributes.lightParams,
                                    geometry);
    }

    //
    Transform WorldToObject = Inverse(curTransform);
    Transform ObjectToWorld = curTransform;
    auto primitive = std::make_unique<GeometricPrimitive>(
            geometry, light, curAttributes.material, WorldToObject, ObjectToWorld);
    world.geometries.push_back(primitive.get());
    if (light) {
        primitive->SetLightId(world.lights.size());
        world.lights.push_back(primitive.get());
    }
    world.primitives.push_back(std::move(primitive));
}

void RenoGeometryLight(const std::string & name, ParameterList & params)
{
    if (state != RenoState::kWorld) {
        Error("RenoGeometryLight()");
        return;
    }

    curAttributes.lightName = name;
    curAttributes.lightParams = params;
}

void RenoIntegrator(const std::string & name, ParameterList & params)
{
    if (state != RenoState::kOptions) {
        Error("RenoIntegrator()");
        return;
    }

    options.integrator = CreateIntegrator(name, params);
}

void RenoMaterial(const std::string & name, ParameterList & params)
{
    if (state != RenoState::kWorld) {
        Error("RenoMaterial()");
        return;
    }

    curAttributes.material = CreateMaterial(name, params);
}

void RenoLight(const std::string & name, ParameterList & params)
{
    if (state != RenoState::kWorld) {
        Error("RenoLight()");
        return;
    }

    Transform WorldToLight = Inverse(curTransform);
    Transform LightToWorld = curTransform;
    std::shared_ptr<Light> light = CreateLight(name, params);
    auto lightPrim = std::make_unique<LightPrimitive>(
            light, WorldToLight, LightToWorld);
    world.lights.push_back(lightPrim.get());
    world.primitives.push_back(std::move(lightPrim));
}

void RenoPixelFilter(const std::string & name, ParameterList & params)
{
    if (state != RenoState::kOptions) {
        Error("RenoPixelFilter()");
        return;
    }

    options.filter = CreatePixelFilter(name, params);
}

void RenoSampler(const std::string & name, ParameterList & params)
{
    if (state != RenoState::kOptions) {
        Error("RenoSampler()");
        return;
    }

    options.sampler = CreateSampler(name, params);
}

} // namespace renoster
