#include "renoster/filtertable.h"

namespace renoster {

FilterTable::FilterTable(const PixelFilter * filter, int tableSize)
    : _filter(filter), _tableSize(tableSize)
{
    _radius = _filter->GetRadius();

    _invRadius = Vector2f(1.f / _radius.x(), 1.f / _radius.y());

    _table.resize(4 * _tableSize * _tableSize);
    size_t offset = 0;
    for (int y = -_tableSize; y < _tableSize; ++y)
    {
        for (int x = -_tableSize; x < _tableSize; ++x)
        {
            Point2f p((x + 0.5f) * _invRadius.x() / _tableSize,
                      (y + 0.5f) * _invRadius.y() / _tableSize);
            _table[offset++] = _filter->Evaluate(p);
        }
    }

    std::vector<float> absTable(_table.size());
    for (size_t i = 0; i < absTable.size(); ++i)
    {
        absTable[i] = std::abs(_table[i]);
    }
    _pTable = Distribution2D(std::move(absTable), Point2i(2 * _tableSize));
}

float FilterTable::Evaluate(const Point2f & p) const
{
    // Map to [-1, 1] x [-1, 1]
    Point2f pFilter(p.x() / _invRadius.x(), p.y() / _invRadius.y());

    // Map to [0, 2 * tS] x [0, 2 * tS]
    Point2f pTable = (pFilter + Vector2f(1.f)) * _tableSize;

    int ix = std::min((int)std::floor(pTable.x()), 2 * _tableSize - 1);
    int iy = std::min((int)std::floor(pTable.y()), 2 * _tableSize - 1);

    return _table[iy * 2 * _tableSize + ix];
}

float FilterTable::Sample(Point2f & p, const Point2f & uv, float * pdf) const
{
    Point2f pTable = _pTable.SampleContinuous(uv, pdf);
    Point2f pUnscaled = (pTable / _tableSize) - Vector2f(1.f);
    p = Point2f(pUnscaled.x() * _radius.x(), pUnscaled.y() * _radius.y());

    if (pdf)
    {
        *pdf *= _tableSize * _tableSize / (_radius.x() * _radius.y());
    }

    return _table[std::floor(pTable.y()) * _tableSize + std::floor(pTable.y())];
}

} // namespace renoster
