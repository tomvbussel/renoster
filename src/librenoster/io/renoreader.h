#ifndef RENOSTER_IO_RENOREADER_H_
#define RENOSTER_IO_RENOREADER_H_

#include <stack>

namespace renoster {

class AreaLight;
class Camera;
class Film;
class Geometry;
class Integrator;
class Light;
class Material;
class ParameterList;
class Pattern;

class RenoReader
{
public:
    RenoReader();

    void Begin(
            const std::string & name);

    void End();

    void WorldBegin();

    void WorldEnd();

    void TransformBegin();

    void TransformEnd();

    void AttributeBegin();

    void AttributeEnd();

    void Identity();

    void Transform(
            float matrix[16]);

    void Perspective(
            float fov);

    void LookAt(
            float ex,
            float ey,
            float ez,
            float lx,
            float ly,
            float lz,
            float ux,
            float uy,
            float uz);

    void Rotate(
            float angle,
            float dx,
            float dy,
            float dz);

    void Scale(
            float sx,
            float sy,
            float sz);

    void Translate(
            float dx,
            float dy,
            float dz);

    void AreaLightSource(
            const std::string &type,
            ParameterList &params);

    void Camera(
            const std::string & type,
            ParameterList & params);

    void Film(
            const std::string & type,
            ParameterList & params);

    void Integrator(
            const std::string & type,
            ParameterList & params);

    void LightSource(const std::string & type,
            ParameterList & params);

    void Material(
            const std::string & type,
            ParameterList & params);

    void Geometry(
            const std::string & type,
            ParameterList & params);

    void Sampler(
            const std::string & type,
            ParameterList & params);

    void Pattern(
            const std::string &type,
            const std::string &name,
            ParameterList &params);

private:
    void Read(const std::string & path);

    enum class ReaderState
    {
        Unitialized,
        Options,
        World
    } m_state;

    struct Options
    {
    } m_options;

    struct Attributes
    {
    } m_curAttributes;

    struct World
    {
    } m_world;

    std::stack<Attributes> m_attributesStack;
    Transform m_curTransform;
    std::stack<Transform> m_transformStack;
};

} // namespace renoster;

#endif // RENOSTER_IO_RENOREADER_H_
