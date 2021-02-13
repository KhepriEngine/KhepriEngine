#include <khepri/io/exceptions.hpp>
#include <khepri/renderer/io/texture.hpp>

namespace khepri::renderer::io {

// DirectDraw surface
extern bool        is_texture_dds(khepri::io::Stream& stream);
extern TextureDesc load_texture_dds(khepri::io::Stream& stream);

// TrueVision TGA
extern bool        is_texture_tga(khepri::io::Stream& stream);
extern TextureDesc load_texture_tga(khepri::io::Stream& stream);

namespace {

struct FormatFuncs
{
    // Checks if the stream contains this format
    bool (*check_func)(khepri::io::Stream&);

    // Loads a texture in this format from stream
    TextureDesc (*load_func)(khepri::io::Stream&);
};

} // namespace

TextureDesc load_texture(khepri::io::Stream& stream)
{
    if (!stream.readable() || !stream.seekable()) {
        throw ArgumentError();
    }

    static const std::array<FormatFuncs, 2> formats = {
        {{is_texture_dds, load_texture_dds}, {is_texture_tga, load_texture_tga}}};
    // Check each supported format
    for (const auto& format : formats) {
        stream.seek(0, khepri::io::seek_origin::begin);
        if (format.check_func(stream)) {
            stream.seek(0, khepri::io::seek_origin::begin);
            return format.load_func(stream);
        }
    }
    throw khepri::io::InvalidFormatError();
}

} // namespace khepri::renderer::io
