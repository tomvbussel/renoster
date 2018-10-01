#include "renoster/display.h"

#include "OpenImageIO/imageio.h"

namespace renoster {

class ImageDisplay : public Display {
public:
    ImageDisplay(const std::string & filename)
        : filename_(filename) {}

    bool Open(const Vector2i & resolution);

    bool WriteData(float * pixels);

    bool Close();

    std::string GetError();

private:
    std::string filename_;
    std::unique_ptr<OIIO::ImageOutput> out_;
};

bool ImageDisplay::Open(const Vector2i & resolution) {
    out_.reset(OIIO::ImageOutput::create(filename_));
    if (!out_) {
        return false;
    }
    OIIO::ImageSpec spec(resolution.x(), resolution.y(), 3,
                         OIIO::TypeDesc::FLOAT);
    return out_->open(filename_, spec);
}

bool ImageDisplay::WriteData(float * pixels) {
    if (!out_) {
        return false;
    }
    return out_->write_image(OIIO::TypeDesc::FLOAT, pixels);
}

bool ImageDisplay::Close() {
    if (out_) {
        return out_->close();
    } else {
        return false;
    }
}

std::string ImageDisplay::GetError() {
    if (out_) {
        return out_->geterror();
    } else {
        return "Not opened";
    }
}

extern "C"
RENO_API
Display * CreateDisplay(ParameterList & params) {
    std::string defFilename = "output.exr";
    std::string filename = params.GetString("filename", &defFilename);

    return new ImageDisplay(filename);
}

} // namespace renoster
