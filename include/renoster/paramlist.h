#ifndef RENOSTER_PARAMLIST_H_
#define RENOSTER_PARAMLIST_H_

#include <memory>
#include <string>
#include <vector>

#include "renoster/color.h"
#include "renoster/export.h"
#include "renoster/normal.h"
#include "renoster/point.h"
#include "renoster/vector.h"

namespace renoster {

class RENO_API ParameterList {
public:
    ParameterList();

    ~ParameterList();

    ParameterList(const ParameterList & params);

    ParameterList & operator=(const ParameterList & params);

    void SetBools(const std::string & name, std::vector<bool> values);
    std::vector<bool> GetBools(const std::string & name) const;
    std::vector<bool> GetBools(const std::string & name);
    bool GetBool(const std::string & name, const bool * defValue) const;

    void SetInts(const std::string & name, std::vector<int> values);
    std::vector<int> GetInts(const std::string & name) const;
    std::vector<int> GetInts(const std::string & name);
    int GetInt(const std::string & name, const int * defValue) const;

    void SetFloats(const std::string & name, std::vector<float> values);
    std::vector<float> GetFloats(const std::string & name) const;
    std::vector<float> GetFloats(const std::string & name);
    float GetFloat(const std::string & name, const float * defValue) const;

    void SetStrings(const std::string & name, std::vector<std::string> values);
    std::vector<std::string> GetStrings(const std::string & name) const;
    std::vector<std::string> GetStrings(const std::string & name);
    std::string GetString(const std::string & name,
                          const std::string * defValue) const;

    void SetColors(const std::string & name, std::vector<Color> values);
    std::vector<Color> GetColors(const std::string & name) const;
    std::vector<Color> GetColors(const std::string & name);
    Color GetColor(const std::string & name, const Color * defValue) const;

    void SetPoint2fs(const std::string & name, std::vector<Point2f> values);
    std::vector<Point2f> GetPoint2fs(const std::string & name) const;
    std::vector<Point2f> GetPoint2fs(const std::string & name);
    Point2f GetPoint2f(const std::string & name,
                       const Point2f * defValue) const;

    void SetPoint3fs(const std::string & name, std::vector<Point3f> values);
    std::vector<Point3f> GetPoint3fs(const std::string & name) const;
    std::vector<Point3f> GetPoint3fs(const std::string & name);
    Point3f GetPoint3f(const std::string & name,
                       const Point3f * defValue) const;

    void SetVector2fs(const std::string & name, std::vector<Vector2f> values);
    std::vector<Vector2f> GetVector2fs(const std::string & name) const;
    std::vector<Vector2f> GetVector2fs(const std::string & name);
    Vector2f GetVector2f(const std::string & name,
                         const Vector2f * defValue) const;

    void SetVector3fs(const std::string & name, std::vector<Vector3f> values);
    std::vector<Vector3f> GetVector3fs(const std::string & name) const;
    std::vector<Vector3f> GetVector3fs(const std::string & name);
    Vector3f GetVector3f(const std::string & name,
                         const Vector3f * defValue) const;

    void SetNormal3fs(const std::string & name, std::vector<Normal3f> values);
    std::vector<Normal3f> GetNormal3fs(const std::string & name) const;
    std::vector<Normal3f> GetNormal3fs(const std::string & name);
    Normal3f GetNormal3f(const std::string & name,
                         const Normal3f * defValue) const;

    void Clear();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

}  // namespace renoster

#endif  // RENOSTER_PARAMLIST_H_
