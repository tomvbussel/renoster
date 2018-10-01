#include "renoster/plugin.h"

#include <iostream>

#include <iterator>
#include <map>
#include <vector>

#include <dlfcn.h>

#include "renoster/camera.h"
#include "renoster/display.h"
#include "renoster/geometry.h"
#include "renoster/integrator.h"
#include "renoster/light.h"
#include "renoster/log.h"
#include "renoster/pixelfilter.h"
#include "renoster/sampler.h"

#include "util/filesystem.h"

namespace renoster {

static std::vector<std::string> PluginSearchPath;

void SetPluginSearchPath(const std::string & searchPath)
{
    SplitSearchPath(searchPath, PluginSearchPath);
}

void AppendToPluginSearchPath(const std::string & searchPath)
{
    std::vector<std::string> newPaths;
    SplitSearchPath(searchPath, newPaths);
    std::move(newPaths.begin(), newPaths.end(), std::back_inserter(PluginSearchPath));
}

namespace { // anonymous

class Plugin {
public:
    Plugin(const std::string & filename)
        : _filename(filename) {}

    ~Plugin() {
        if (_handle) {
            dlclose(_handle);
        }
    }

    virtual bool Open() {
        _handle = dlopen(_filename.c_str(), RTLD_LAZY);
        if (!_handle) {
            Error("Plugin::Open(): %s", dlerror());
            return false;
        }
        return true;
    }

    void * GetSymbol(const std::string & symbolName) {
        assert(_handle);

        void * symbol = dlsym(_handle, symbolName.c_str());
        if (!symbol) {
            Error("Plugin::GetSymbol(): Could not get symbol: %s", symbolName);
        }
        return symbol;
    }

    bool Close() {
        dlclose(_handle);
        _handle = nullptr;
        return true;
    }

protected:
    std::string _filename;
    void * _handle = nullptr;
};

class CameraPlugin : public Plugin {
public:
    CameraPlugin(const std::string & filename)
        : Plugin(filename) {}

    bool Open() {
        if (!Plugin::Open()) {
            return false;
        }
        
        _createFunc = (CreateCameraFunc) GetSymbol("CreateCamera");
        if (!_createFunc) {
            return false;
        }

        return true;
    }

    Camera * CreateCamera(const ParameterList & params,
                          const Transform & WorldToCamera,
                          const Transform & CameraToWorld) const {
        return _createFunc(params, WorldToCamera, CameraToWorld);
    }

private:
    using CreateCameraFunc = Camera * (*)(const ParameterList & params,
                                          const Transform & WorldToCamera,
                                          const Transform & CameraToWorld);

    CreateCameraFunc _createFunc = nullptr;
};

class DisplayPlugin : public Plugin {
public:
    DisplayPlugin(const std::string & filename)
        : Plugin(filename) {}

    bool Open() {
        if (!Plugin::Open()) {
            return false;
        }
        
        _createFunc = (CreateDisplayFunc) GetSymbol("CreateDisplay");
        if (!_createFunc) {
            return false;
        }

        return true;
    }

    Display * CreateDisplay(ParameterList & params) const {
        return _createFunc(params);
    }

private:
    using CreateDisplayFunc = Display * (*)(ParameterList & params);

    CreateDisplayFunc _createFunc = nullptr;
};

class GeometryPlugin : public Plugin {
public:
    GeometryPlugin(const std::string & filename)
        : Plugin(filename) {}

    bool Open() {
        if (!Plugin::Open()) {
            return false;
        }

        _createFunc = (CreateGeometryFunc) GetSymbol("CreateGeometry");
        if (!_createFunc) {
            return false;
        }

        return true;
    }

    Geometry * CreateGeometry(const ParameterList & params) const {
        return _createFunc(params);
    }

private:
    using CreateGeometryFunc = Geometry * (*)(const ParameterList & params);

    CreateGeometryFunc _createFunc = nullptr;
};

class GeometryLightPlugin : public Plugin {
public:
    GeometryLightPlugin(const std::string & filename)
        : Plugin(filename) {}

    bool Open() {
        if (!Plugin::Open()) {
            return false;
        }

        _createFunc = (CreateGeometryLightFunc) GetSymbol("CreateGeometryLight");
        if (!_createFunc) {
            return false;
        }

        return true;
    }

    GeometryLight * CreateGeometryLight(
            const ParameterList & params,
            const std::shared_ptr<Geometry> & geometry) {
        return _createFunc(params, geometry);
    }

private:
    using CreateGeometryLightFunc
        = GeometryLight * (*)(const ParameterList & params,
                              const std::shared_ptr<Geometry> & geometry);

    CreateGeometryLightFunc _createFunc = nullptr;
};

class IntegratorPlugin : public Plugin {
public:
    IntegratorPlugin(const std::string & filename)
        : Plugin(filename) {}

    bool Open() {
        if (!Plugin::Open()) {
            return false;
        }

        _createFunc = (CreateIntegratorFunc) GetSymbol("CreateIntegrator");
        if (!_createFunc) {
            return false;
        }
        
        return true;
    }

    Integrator * CreateIntegrator(ParameterList & params) const {
        return _createFunc(params);
    }

private:
    using CreateIntegratorFunc = Integrator * (*)(ParameterList & params);

    CreateIntegratorFunc _createFunc = nullptr;
};

class LightPlugin : public Plugin {
public:
    LightPlugin(const std::string & filename)
        : Plugin(filename) {}

    bool Open() {
        if (!Plugin::Open()) {
            return false;
        }

        _createFunc = (CreateLightFunc) GetSymbol("CreateLight");
        if (!_createFunc) {
            return false;
        }
        
        return true;
    }

    Light * CreateLight(const ParameterList & params) const {
        return _createFunc(params);
    }

private:
    using CreateLightFunc = Light * (*)(const ParameterList & params);

    CreateLightFunc _createFunc = nullptr;
};

class MaterialPlugin : public Plugin {
public:
    MaterialPlugin(const std::string & filename)
        : Plugin(filename) {}

    bool Open() {
        if (!Plugin::Open()) {
            return false;
        }

        _createFunc = (CreateMaterialFunc) GetSymbol("CreateMaterial");
        if (!_createFunc) {
            return false;
        }

        return true;
    }

    Material * CreateMaterial(const ParameterList & params) const {
        return _createFunc(params);
    }

private:
    using CreateMaterialFunc = Material * (*)(const ParameterList & params);

    CreateMaterialFunc _createFunc;
};

class PixelFilterPlugin : public Plugin {
public:
    PixelFilterPlugin(const std::string & filename)
        : Plugin(filename) {}

    bool Open() {
        if (!Plugin::Open()) {
            return false;
        }

        _createFunc = (CreatePixelFilterFunc) GetSymbol("CreatePixelFilter");
        if (!_createFunc) {
            return false;
        }
        
        return true;
    }

    PixelFilter * CreatePixelFilter(ParameterList & params) const {
        return _createFunc(params);
    }

private:
    using CreatePixelFilterFunc = PixelFilter * (*)(ParameterList & params);

    CreatePixelFilterFunc _createFunc = nullptr;
};

class SamplerPlugin : public Plugin {
public:
    SamplerPlugin(const std::string & filename)
        : Plugin(filename) {}

    bool Open() {
        if (!Plugin::Open()) {
            return false;
        }

        _createFunc = (CreateSamplerFunc) GetSymbol("CreateSampler");
        if (!_createFunc) {
            return false;
        }

        return true;
    }

    Sampler * CreateSampler(ParameterList & params) const {
        return _createFunc(params);
    }

private:
    using CreateSamplerFunc = Sampler * (*)(ParameterList & params);

    CreateSamplerFunc _createFunc = nullptr;
};

template <typename T> using PluginMap
    = std::map<std::string, std::unique_ptr<T>>;
using CameraPluginMap = PluginMap<CameraPlugin>;
using DisplayPluginMap = PluginMap<DisplayPlugin>;
using GeometryPluginMap = PluginMap<GeometryPlugin>;
using GeometryLightPluginMap = PluginMap<GeometryLightPlugin>;
using IntegratorPluginMap = PluginMap<IntegratorPlugin>;
using LightPluginMap = PluginMap<LightPlugin>;
using MaterialPluginMap = PluginMap<MaterialPlugin>;
using PixelFilterPluginMap = PluginMap<PixelFilterPlugin>;
using SamplerPluginMap = PluginMap<SamplerPlugin>;

static CameraPluginMap cameraPlugins;
static DisplayPluginMap displayPlugins;
static GeometryPluginMap geometryPlugins;
static GeometryLightPluginMap geoLightPlugins;
static IntegratorPluginMap integratorPlugins;
static LightPluginMap lightPlugins;
static MaterialPluginMap materialPlugins;
static PixelFilterPluginMap pixelFilterPlugins;
static SamplerPluginMap samplerPlugins;

template <typename P>
void CatalogPlugin(const std::string & name, PluginMap<P> & catalog)
{
    const std::string filename = name + ".so";

    auto filepath = FindSearchPath(filename, PluginSearchPath);
    if (filepath.empty()) {
        return;
    }

    Info("Found plugin: %s", filepath);
    auto plugin = std::make_unique<P>(filepath);

    if (!plugin->Open()) {
        Error("Could not open plugin %s", name);
        return;
    }

    catalog[name] = std::move(plugin);
}

template <typename P>
P * GetPlugin(const std::string & name, PluginMap<P> & pluginMap)
{
    if (name.empty()) {
        Error("GetPlugin() called with empty plugin name");
        return nullptr;
    }

    auto found = pluginMap.find(name);
    if (found == std::end(pluginMap)) {
        CatalogPlugin<P>(name, pluginMap);
        found = pluginMap.find(name);
    }

    if (found != std::end(pluginMap)) {
        return found->second.get();
    } else {
        Error("CreatePlugin() could not find a plugin with name %s",
              name);
        return nullptr;
    }
}

} // anonymous namespace

std::unique_ptr<Camera> CreateCamera(const std::string & name,
                                     const ParameterList & params,
                                     const Transform & WorldToCamera,
                                     const Transform & CameraToWorld)
{
    std::unique_ptr<Camera> camera;
    CameraPlugin * plugin = GetPlugin<CameraPlugin>(name, cameraPlugins);
    if (!plugin) {
        Error("CreateCamera()");
        return camera;
    }

    camera = std::unique_ptr<Camera>(plugin->CreateCamera(
                params, WorldToCamera, CameraToWorld));

    return camera;
}

std::unique_ptr<Display> CreateDisplay(const std::string & name,
                                       ParameterList & params)
{
    std::unique_ptr<Display> display;
    DisplayPlugin * plugin = GetPlugin<DisplayPlugin>(name, displayPlugins);
    if (!plugin) {
        Error("CreateDisplay()");
        return display;
    }

    display = std::unique_ptr<Display>(plugin->CreateDisplay(params));

    return display;
}

std::unique_ptr<Geometry> CreateGeometry(const std::string & name,
                                         const ParameterList & params)
{
    std::unique_ptr<Geometry> geometry;
    GeometryPlugin * plugin = GetPlugin<GeometryPlugin>(name, geometryPlugins);
    if (!plugin) {
        Error("CreateGeometry()");
        return geometry;
    }

    geometry = std::unique_ptr<Geometry>(plugin->CreateGeometry(params));

    return geometry;
}

std::unique_ptr<GeometryLight> CreateGeometryLight(
        const std::string & name, const ParameterList & params,
        const std::shared_ptr<Geometry> & geometry)
{
    std::unique_ptr<GeometryLight> geoLight;
    GeometryLightPlugin * plugin =
            GetPlugin<GeometryLightPlugin>(name, geoLightPlugins);
    if (!plugin) {
        Error("CreateGeometryLight()");
        return geoLight;
    }

    geoLight = std::unique_ptr<GeometryLight>(
            plugin->CreateGeometryLight(params, geometry));

    return geoLight;
}

std::unique_ptr<Integrator> CreateIntegrator(const std::string & name,
                                             ParameterList & params)
{
    std::unique_ptr<Integrator> integrator;
    IntegratorPlugin * plugin = GetPlugin<IntegratorPlugin>(name, integratorPlugins);
    if (!plugin) {
        Error("CreateIntegrator()");
        return integrator;
    }

    integrator = std::unique_ptr<Integrator>(plugin->CreateIntegrator(params));

    return integrator;
}

std::unique_ptr<Light> CreateLight(const std::string & name,
                                   const ParameterList & params)
{
    std::unique_ptr<Light> light;
    LightPlugin * plugin = GetPlugin<LightPlugin>(name, lightPlugins);
    if (!plugin) {
        Error("CreateLight()");
        return light;
    }

    light = std::unique_ptr<Light>(plugin->CreateLight(params));

    return light;
}

std::unique_ptr<Material> CreateMaterial(const std::string & name,
                                         const ParameterList & params)
{
    std::unique_ptr<Material> material;
    MaterialPlugin * plugin = GetPlugin<MaterialPlugin>(name, materialPlugins);
    if (!plugin) {
        Error("CreateMaterial()");
        return material;
    }

    material = std::unique_ptr<Material>(plugin->CreateMaterial(params));

    return material;
}

std::unique_ptr<PixelFilter> CreatePixelFilter(const std::string & name,
                                               ParameterList & params)
{
    std::unique_ptr<PixelFilter> filter;
    PixelFilterPlugin * plugin = GetPlugin<PixelFilterPlugin>(name, pixelFilterPlugins);
    if (!plugin) {
        Error("CreatePixelFilter()");
        return filter;
    }

    filter = std::unique_ptr<PixelFilter>(plugin->CreatePixelFilter(params));

    return filter;
}

std::unique_ptr<Sampler> CreateSampler(const std::string & name,
                                       ParameterList & params)
{
    std::unique_ptr<Sampler> sampler;
    SamplerPlugin * plugin = GetPlugin<SamplerPlugin>(name, samplerPlugins);
    if (!plugin) {
        Error("CreateSampler()");
        return sampler;
    }

    sampler = std::unique_ptr<Sampler>(plugin->CreateSampler(params));

    return sampler;
}

} // namespace renoster
