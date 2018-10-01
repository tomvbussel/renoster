#include "renoster/film.h"

#include <cassert>
#include <iostream>

#include "renoster/bounds.h"
#include "renoster/display.h"
#include "renoster/log.h"
#include "renoster/pixelfilter.h"
#include "renoster/point.h"
#include "renoster/sampler.h"

namespace renoster {

FilmTile::FilmTile(int tileId, const Point2i & index,
                   const Bounds2i & pixelBounds, const Bounds2i & sampleBounds,
                   const PixelFilter * filter, const FilterTable * filterTable,
                   FilmSampleMode sampleMode)
    : _tileId(tileId),
    _index(index),
    _pixelBounds(pixelBounds),
    _sampleBounds(sampleBounds),
    _filter(filter),
    _filterTable(filterTable),
    _sampleMode(sampleMode)
{
    _pixels.resize(std::max(0, _pixelBounds.Volume()));
}

Point2f FilmTile::Sample(const Point2i & pixel, Sampler * sampler,
                         float * pdf)
{
    Point2f uv = sampler->Get2D();
    switch (_sampleMode) {
    case FilmSampleMode::kConvolution:
        if (pdf) {
            *pdf = 1.f; // Pixel has area 1
        }

        return pixel + uv;

    case FilmSampleMode::kImportance:
        Point2f pFilter;
        float pdfFilter;
        float weight = _filterTable->Sample(pFilter, uv, &pdfFilter);

        if (pdf) {
            *pdf = pdfFilter;
        }

        return pixel + pFilter + Vector2f(0.5f);
    }
}

void FilmTile::AddSample(const Point2i & pPixel, const Point2f & pSample,
                         const FilmAccumulator & accum)
{
    Color L;
    accum.GetValue(L);

    if (_sampleMode == FilmSampleMode::kConvolution) {
        Vector2f filterRadius = _filter->GetRadius();
        Point2i pMin = Point2i(Ceil(pPixel + filterRadius));
        Point2i pMax = Point2i(Floor(pPixel + filterRadius)) + Point2i(1);

        Bounds2i bPixels = Bounds2i(pMin, pMax);
        bPixels = Intersection(bPixels, _pixelBounds);
        for (Point2i p : bPixels) {
            Point2f pFilter(p.x() + 0.5f - pSample.x(),
                            p.y() + 0.5f - pSample.y());
            float weight = _filterTable->Evaluate(pFilter);

            Pixel & pixel = GetPixel(p);
            pixel.contribSum += weight * L;
            pixel.weightSum += weight;
        }
    } else {
        Pixel & pixel = GetPixel(pPixel);
        pixel.contribSum += L;
        pixel.weightSum += 1.f;
    }
}

Pixel & FilmTile::GetPixel(const Point2i & p)
{
    assert(_pixelBounds.Contains(p));
    Vector2i dims = _pixelBounds.Diagonal();
    Vector2i dTile = p - _pixelBounds.min();
    int offset = dTile.y() * dims.x() + dTile.x();
    return _pixels[offset];
}

TileGenerator::TileGenerator(Order order, const Vector2i & nTiles)
    : _order(order), _nTiles(nTiles), _index(0) {}

bool TileGenerator::GenerateNextTile(Point2i & tileIndex)
{
    int index;
    {
        std::lock_guard<std::mutex> guard(_mutex);
        if (_index >= _nTiles.x() * _nTiles.y()) {
            return false;
        }
        index = _index++;
    }

    switch (_order) {
    case Order::kHorizontal:
        tileIndex = Point2i(index % _nTiles.x(), index / _nTiles.x());
        return true;

    case Order::kVertical:
        tileIndex = Point2i(index / _nTiles.y(), index % _nTiles.y());
        return true;

    default:
        Error("");
        return false;
    }
}

Film::Film(const Vector2i & resolution, float pixelAspectRatio,
           const Bounds2f & cropWindow, float frameAspectRatio,
           const Bounds2f & screenWindow, const Vector2i & tileSize,
           int filterTableSize, FilmSampleMode sampleMode)
    : _resolution(resolution),
      _pixelAspectRatio(pixelAspectRatio),
      _cropWindow(cropWindow),
      _frameAspectRatio(frameAspectRatio),
      _screenWindow(screenWindow),
      _tileSize(tileSize),
      _filterTableSize(filterTableSize),
      _sampleMode(sampleMode)
{

    // Create the scanlines for the pixels
    _pixelBounds = Bounds2i(
            (Point2i)(Ceil(Point2f(_resolution.x() * _cropWindow.min().x(),
                                   _resolution.y() * _cropWindow.min().y()))),
            (Point2i)(Ceil(Point2f(_resolution.x() * _cropWindow.max().x(),
                                   _resolution.y() * _cropWindow.max().y())))
    );

    _pixels.resize(std::max(0, _pixelBounds.Volume()));
}

void Film::RenderBegin(PixelFilter * filter, Display * display)
{
    _filter = filter;
    _display = display;

    // Calculate the pixels which need to be sampled,
    // including the border created by the filter
    if (_sampleMode == FilmSampleMode::kConvolution) {
        Vector2f filterRadius = _filter->GetRadius();
        _sampleBounds = Bounds2i(
                (Point2i)Ceil(_pixelBounds.min() - filterRadius),
                (Point2i)Floor(_pixelBounds.max() + filterRadius)
        );
    } else {
        _sampleBounds = _pixelBounds;
    }

    // Calculate the number of tiles
    Vector2i sampleExtent = _sampleBounds.Diagonal();
    _nTiles = Vector2i(
            (sampleExtent.x() + _tileSize.x() - 1) / _tileSize.x(),
            (sampleExtent.y() + _tileSize.y() - 1) / _tileSize.y()
    );

    // Create the tile generator
    _tileGen = std::make_unique<TileGenerator>(
            TileGenerator::Order::kHorizontal, _nTiles);

    // Prepare a filter table
    _filterTable = std::make_unique<FilterTable>(_filter, _filterTableSize);
}

void Film::RenderEnd()
{
    OutputToDisplay();

    // Clear pixels
    for (Pixel & pixel : _pixels) {
        pixel.contribSum = Color(0.f);
        pixel.weightSum = 0.f;
    }

    // Make sample bounds invalid, to be safe
    _sampleBounds = Bounds2i();

    _filter = nullptr;
    _display = nullptr;

    _tileGen.reset();
    _filterTable.reset();
}

std::unique_ptr<FilmTile> Film::GetNextTile() const
{
    // Get a new tile
    Point2i tileIndex;
    if (!_tileGen->GenerateNextTile(tileIndex)) {
        return std::unique_ptr<FilmTile>();
    }

    // Calculate the pixels which need to be sampled
    Vector2i tileOffset(tileIndex.x() * _tileSize.x(),
                        tileIndex.y() * _tileSize.y());
    Bounds2i tileSampleBounds(
            _sampleBounds.min() + tileOffset,
            _sampleBounds.min() + tileOffset + _tileSize
    );
    Bounds2i b = tileSampleBounds;
    tileSampleBounds = Intersection(_sampleBounds, tileSampleBounds);

    // Calculate the pixels for which values need to be stored
    Bounds2i tilePixelBounds;
    if (_sampleMode == FilmSampleMode::kConvolution) {
        Vector2f halfPixel(0.5f);
        Vector2f filterRadius = _filter->GetRadius();
        tilePixelBounds = Bounds2i(
                (Point2i)Ceil(tileSampleBounds.min() - halfPixel - filterRadius),
                (Point2i)Floor(tileSampleBounds.max() - halfPixel + filterRadius) + Point2i(1)
        );
    } else {
        tilePixelBounds = tileSampleBounds;
    }
    tilePixelBounds = Intersection(_sampleBounds, tilePixelBounds);

    int tileId = _nTiles.x() * tileIndex.y() + tileIndex.x();
    return std::make_unique<FilmTile>(tileId, tileIndex, tilePixelBounds,
                                      tileSampleBounds, _filter,
                                      _filterTable.get(), _sampleMode);
}

void Film::MergeFilmTile(std::unique_ptr<FilmTile> tile)
{
    std::lock_guard<std::mutex> lock(_mutex);

    Bounds2i bPixels = Intersection(_pixelBounds, tile->_pixelBounds);
    for (Point2i p : bPixels) {
        Pixel & pixelTile = tile->GetPixel(p);
        Pixel & pixelFilm = GetPixel(p);

        pixelFilm.contribSum += pixelTile.contribSum;
        pixelFilm.weightSum += pixelTile.weightSum;
    }
}

Point2f Film::RasterToScreen(const Point2f & pRaster) const
{
    // Scale to [0, 1) x [0, 1)
    Point2f pNDC(pRaster.x() / _resolution.x(),
                 pRaster.y() / _resolution.y());

    // Scale to [0, 2 * aspect) x [0, 2)
    Vector2f d(_screenWindow.max().x() - _screenWindow.min().x(),
               _screenWindow.min().y() - _screenWindow.max().y());

    // Translate to [-aspect, aspect) x [-1, 1)
    return Point2f(pNDC.x() * d.x() + _screenWindow.min().x(),
                   pNDC.y() * d.y() + _screenWindow.max().y());
}

void Film::OutputToDisplay()
{
    assert(_display != nullptr);

    const int nChannels = 3;
    std::vector<float> pixels(_resolution.x() * _resolution.y() * nChannels);
    
    for (Point2i p : _pixelBounds) {
        Pixel & pixel = GetPixel(p);

        Color finalColor = pixel.contribSum;
        if (pixel.weightSum != 0.f) {
            // Error("%f", pixel.weightSum);
            finalColor /= pixel.weightSum;
        }

        size_t offset = p.y() * _resolution.x() + p.x();
        pixels[nChannels * offset] = finalColor.r();
        pixels[nChannels * offset + 1] = finalColor.g();
        pixels[nChannels * offset + 2] = finalColor.b();
    }

    _display->Open(_resolution);
    _display->WriteData(pixels.data());
    _display->Close();
}

Pixel & Film::GetPixel(const Point2i & pRaster)
{
    assert(_pixelBounds.Contains(pRaster));

    Vector2i dims = _pixelBounds.Diagonal();
    Vector2i d = pRaster - _pixelBounds.min();
    int offset = d.y() * dims.x() + d.x();
    return _pixels[offset];
}

std::unique_ptr<Film> CreateFilm(ParameterList & params)
{
    // Horizontal and Vertical Resolution
    Vector2i resolution;
    Vector2i defResolution(640, 480);
    resolution.x() = params.GetInt("xresolution", &defResolution.x());
    resolution.y() = params.GetInt("yresolution", &defResolution.y());

    // Pixel Aspect Ratio
    float defPixel = 1.f;
    float pixel = params.GetFloat("pixelaspectratio", &defPixel);

    // Crop Window
    Bounds2f crop;
    auto cw = params.GetFloats("cropwindow");
    if (cw.size() == 4) {
        crop.min().x() = cw[0];
        crop.max().x() = cw[1];
        crop.min().y() = cw[2];
        crop.max().y() = cw[3];
    } else if (cw.size() == 0) {
        crop.min().x() = 0.f;
        crop.max().x() = 1.f;
        crop.min().y() = 0.f;
        crop.max().y() = 1.f;
    } else {
        Error("");
    }

    // Frame aspect ratio
    float defFrame = float(resolution.x()) / float(resolution.y());
    float frame = params.GetFloat("frameaspectratio", &defFrame);

    // Screen window
    Bounds2f screen;
    auto sw = params.GetFloats("screenwindow");
    if (sw.size() == 4) {
        screen.min().x() = sw[0];
        screen.max().x() = sw[1];
        screen.min().y() = sw[2];
        screen.max().y() = sw[3];
    } else if (sw.size() == 0) {
        if (frame > 1.f) {
            screen.min().x() = -frame;
            screen.max().x() = frame;
            screen.min().y() = -1.f;
            screen.max().y() = 1.f;
        } else {
            screen.min().x() = -1.f;
            screen.max().x() = 1.f;
            screen.min().y() = -1.f / frame;
            screen.max().y() = 1.f / frame;
        }
    } else {
        Error("");
    }

    // Tile Size
    Vector2i tileSize;
    int defTileSize = 16;
    tileSize.x() = params.GetInt("xtile", &defTileSize);
    tileSize.y() = params.GetInt("ytile", &defTileSize);

    // Filter Table Size
    int defFilterTableSize = 16;
    int filterTableSize = params.GetInt("filterTableSize",
                                        &defFilterTableSize);

    // Sample mode
    FilmSampleMode mode;
    std::string defMode = "importance";
    std::string psm = params.GetString("pixelsamplemode", &defMode);
    if (psm == "importance") {
        mode = FilmSampleMode::kImportance;
    } else if (psm == "convolution") {
        mode = FilmSampleMode::kConvolution;
    } else {
        Error("");
    }

    return std::make_unique<Film>(resolution, pixel, crop, frame, screen,
                                  tileSize, filterTableSize, mode);
}

} // namespace renoster
