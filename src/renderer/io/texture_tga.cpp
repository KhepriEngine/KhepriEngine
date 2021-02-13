#include <khepri/io/exceptions.hpp>
#include <khepri/io/stream.hpp>
#include <khepri/renderer/texture_desc.hpp>

#include <cassert>
#include <cstdint>

namespace khepri::renderer::io {
namespace {

enum
{
    targa_image_none         = 0,
    targa_image_color_mapped = 1,
    targa_image_rgb          = 2,
    targa_image_grayscale    = 3,

    targa_image_rle = 8, // Run-length encoding modifier
};

struct Header
{
    std::uint8_t  image_id_length;
    std::uint8_t  color_map_type;
    std::uint8_t  image_type;
    std::uint16_t color_map_start;
    std::uint16_t color_map_length;
    std::uint8_t  color_map_bpp;
    std::uint16_t image_x;
    std::uint16_t image_y;
    std::uint16_t image_width;
    std::uint16_t image_height;
    std::uint8_t  image_bpp;
    std::uint8_t  image_descriptor;
};

void verify(bool condition)
{
    if (!condition) {
        throw khepri::io::InvalidFormatError();
    }
}

Header read_header(khepri::io::Stream& stream)
{
    Header hdr{};
    hdr.image_id_length  = stream.read_byte();
    hdr.color_map_type   = stream.read_byte();
    hdr.image_type       = stream.read_byte();
    hdr.color_map_start  = stream.read_short();
    hdr.color_map_length = stream.read_short();
    hdr.color_map_bpp    = stream.read_byte();
    hdr.image_x          = stream.read_short();
    hdr.image_y          = stream.read_short();
    hdr.image_width      = stream.read_short();
    hdr.image_height     = stream.read_short();
    hdr.image_bpp        = stream.read_byte();
    hdr.image_descriptor = stream.read_byte();
    return hdr;
}

bool is_valid_header(const Header& header)
{
    if (header.color_map_type >= 2) {
        // Reserved values
        return false;
    }

    if ((header.image_type & ~targa_image_rle) > targa_image_grayscale) {
        // Invalid values
        return false;
    }

    if (header.color_map_type == 1) {
        // Validate color map data
        if (header.color_map_bpp != 16 && header.color_map_bpp != 24 &&
            header.color_map_bpp != 32) {
            return false;
        }
    } else if (header.image_type == targa_image_color_mapped) {
        // Color-mapped images require a color map
        return false;
    }

    if (header.image_bpp != 8 && header.image_bpp != 16 && header.image_bpp != 24 &&
        header.image_bpp != 32) {
        return false;
    }

    if (header.image_descriptor != 0) {
        return false;
    }

    return true;
}

} // namespace

bool is_texture_tga(khepri::io::Stream& stream)
{
    assert(stream.readable() && stream.seekable());
    try {
        auto header = read_header(stream);
        return is_valid_header(header);
    } catch (const khepri::io::Error&) {
    }
    return false;
}

TextureDesc load_texture_tga(khepri::io::Stream& stream)
{
    assert(stream.readable() && stream.seekable());

    const auto header = read_header(stream);
    verify(is_valid_header(header));

    verify(header.image_type != targa_image_none);
    verify(header.image_type == targa_image_rgb);

    // 16-bits Targa isn't supported as of now
    verify(header.image_bpp != 16);

    // Skip over the Image ID section
    stream.seek(header.image_id_length, khepri::io::seek_origin::current);

    // Skip the color map, if any; we don't support color mapped TGA files
    if (header.color_map_type != 0) {
        stream.seek(header.color_map_length * header.color_map_bpp / 8U,
                    khepri::io::seek_origin::current);
    }

    // Read the image in B8G8R8A8 format
    std::vector<std::uint8_t> data(header.image_width * header.image_height * 4);

    // We have to repack the image data to get the wanted format
    const std::size_t pixel_count = header.image_width * header.image_height;

    std::vector<std::uint8_t> raw_data(pixel_count * header.image_bpp / 8U);
    stream.read(raw_data.data(), raw_data.size());

    const gsl::span<std::uint8_t> src = raw_data;
    for (std::size_t y = 1; y <= header.image_height; ++y) {
        // Note: targa image data are stored upside-down (bottom scanline first)
        auto dest = gsl::span<std::uint8_t>(
            &data[(header.image_height - y) * header.image_width * 4], header.image_width * 4);

        switch (header.image_bpp) {
        case 32:
            for (std::size_t x = 0, d = 0, s = 0; x < header.image_width; ++x, d += 4, s += 4) {
                dest[d + 0] = src[s + 2]; // red
                dest[d + 1] = src[s + 1]; // green
                dest[d + 2] = src[s + 0]; // blue
                dest[d + 3] = src[s + 3]; // alpha
            }
            break;
        case 24:
            for (std::size_t x = 0, d = 0, s = 0; x < header.image_width; ++x, d += 4, s += 3) {
                dest[d + 0] = src[s + 2]; // red
                dest[d + 1] = src[s + 1]; // green
                dest[d + 2] = src[s + 0]; // blue
                dest[d + 3] = 255;        // alpha
            }
            break;
        default:
            assert(false);
            break;
        }
    }

    std::vector<TextureDesc::Subresource> subresources{
        {0, data.size(), header.image_width * header.image_bpp / 8U, data.size()}};

    return TextureDesc(TextureDimension::texture_2d, header.image_width, header.image_height, 0, 1,
                       PixelFormat::r8g8b8a8_unorm_srgb, std::move(subresources), std::move(data));
}

} // namespace khepri::renderer::io
