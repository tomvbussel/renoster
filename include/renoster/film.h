#ifndef RENOSTER_FILM_H_
#define RENOSTER_FILM_H_

#include <memory>
#include <mutex>
#include <vector>

#include "renoster/bounds.h"
#include "renoster/color.h"
#include "renoster/export.h"
#include "renoster/filmaccumulator.h"
#include "renoster/filtertable.h"
#include "renoster/paramlist.h"
#include "renoster/point.h"
#include "renoster/vector.h"

namespace renoster {

class Display;
class Film;
class PixelFilter;
class Sampler;

enum class FilmSampleMode {
    kConvolution,
    kImportance
};

struct Pixel {
    Color contribSum = Color(0.f);
    float weightSum = 0.f;
};

/// FilmTile represents a part of the
class RENO_API FilmTile {
public:
    FilmTile(int tileId, const Point2i & index, const Bounds2i & pixelBounds,
             const Bounds2i & sampleBounds, const PixelFilter * filter,
             const FilterTable * filterTable, FilmSampleMode sampleMode);

    Point2f Sample(const Point2i & pixel, Sampler * sampler, float * pdf);

    void AddSample(const Point2i & pixel, const Point2f & pSample,
                   const FilmAccumulator & accum);

    const Bounds2i & GetSampleBounds() const {
        return _sampleBounds;
    }

    const Bounds2i & GetPixelBounds() const {
        return _pixelBounds;
    }

    int GetTileId() const {
        return _tileId;
    }

private:
    Pixel & GetPixel(const Point2i & p);

    ///
    int _tileId;

    /// The index of the tile
    Point2i _index;

    /// The pixels we need to store values for.
    /// This includes some pixels on other tiles.
    Bounds2i _pixelBounds;

    /// The pixels we need to sample
    Bounds2i _sampleBounds;

    /// Pixel filter of the film
    const PixelFilter * _filter;

    /// Filter table of the film
    const FilterTable * _filterTable;

    /// Sample mode of the film
    FilmSampleMode _sampleMode;

    /// Pixels stored in scanlines
    std::vector<Pixel> _pixels;

    friend class Film;
};

class RENO_API TileGenerator {
public:
    enum class Order {
        kHorizontal,
        kVertical,
        kSpiral
    };

    TileGenerator(Order order, const Vector2i & nTiles);

    bool GenerateNextTile(Point2i & tileIndex);

private:
    Order _order;
    Vector2i _nTiles;
    int _index;
    std::mutex _mutex;
};

class RENO_API Film {
public:
    Film(const Vector2i & resolution, float pixelAspectRatio,
         const Bounds2f & cropWindow, float frameAspectRatio,
         const Bounds2f & screenWindow, const Vector2i & tileSize,
         int filterTableSize, FilmSampleMode sampleMode);

    void RenderBegin(PixelFilter * filter, Display * display);

    void RenderEnd();

    std::unique_ptr<FilmTile> GetNextTile() const;

    void MergeFilmTile(std::unique_ptr<FilmTile> tile);

    Point2f RasterToScreen(const Point2f & p) const;

    const Bounds2f & GetScreenWindow() const {
        return _screenWindow;
    }

private:
    void OutputToDisplay();

    Pixel & GetPixel(const Point2i & p);

    // Film Settings
    Vector2i _resolution;
    float _pixelAspectRatio;
    Bounds2f _cropWindow;
    float _frameAspectRatio;
    Bounds2f _screenWindow;
    Vector2i _tileSize;
    int _filterTableSize;
    FilmSampleMode _sampleMode;

    /// Number of tiles in each direction
    Vector2i _nTiles;
    
    // Cropped pixel bounds
    Bounds2i _pixelBounds;

    // Pixels stored in scanlines
    std::vector<Pixel> _pixels;

    // Filter and Display
    // Set in RenderBegin()
    PixelFilter * _filter;
    Display * _display;

    //
    Bounds2i _sampleBounds;

    // Tile Generator
    std::unique_ptr<TileGenerator> _tileGen;
    std::unique_ptr<FilterTable> _filterTable;

    /// Mutex for merging tiles
    std::mutex _mutex;
};

RENO_API std::unique_ptr<Film> CreateFilm(ParameterList & params);

} // namespace renoster

#endif // RENOSTER_FILM_H_
