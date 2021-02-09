#pragma once

#include <khepri/io/exceptions.hpp>
#include <khepri/renderer/io/texture.hpp>
#include <khepri/renderer/texture_desc.hpp>

#include <algorithm>
#include <cassert>
#include <optional>

namespace khepri::renderer::io {
namespace {

using khepri::renderer::PixelFormat;

constexpr auto BITS_PER_BYTE = 8UL;

enum : unsigned long
{
    ddsf_caps        = 1,
    ddsf_height      = 2,
    ddsf_width       = 4,
    ddsf_pitch       = 8,
    ddsf_pixelformat = 0x1000,
    ddsf_mipmapcount = 0x20000,
    ddsf_linearsize  = 0x80000,
    ddsf_depth       = 0x800000,
};

enum : unsigned long
{
    ddscaps2_cubemap           = 0x200,
    ddscaps2_cubemap_positivex = 0x400,
    ddscaps2_cubemap_negativex = 0x800,
    ddscaps2_cubemap_positivey = 0x1000,
    ddscaps2_cubemap_negativey = 0x2000,
    ddscaps2_cubemap_positivez = 0x4000,
    ddscaps2_cubemap_negativez = 0x8000,
    ddscaps2_volume            = 0x200000,
};

enum : unsigned long
{
    ddpf_alphapixels     = 1,
    ddpf_alpha           = 2,
    ddpf_fourcc          = 4,
    ddpf_paletteindexed8 = 0x20,
    ddpf_rgb             = 0x40,
    ddpf_yuv             = 0x200,
    ddpf_luminance       = 0x20000,
    ddpf_bumpdudv        = 0x00080000,
};

struct DdsPixelFormat
{
    unsigned long flags;
    unsigned long fourcc;
    unsigned long rgb_bitcount;
    unsigned long r_mask;
    unsigned long g_mask;
    unsigned long b_mask;
    unsigned long a_mask;
};

constexpr unsigned long DDS_MAGIC            = 0x20534444; // "DDS " in little endian
constexpr unsigned long DDS_HEADER_SIZE      = 124;
constexpr unsigned long DDS_PIXELFORMAT_SIZE = 32;
constexpr unsigned long DDS_REQUIRED_FLAGS =
    ddsf_caps | ddsf_height | ddsf_width | ddsf_pixelformat;

void verify(bool condition)
{
    if (!condition) {
        throw khepri::io::InvalidFormatError();
    }
}

constexpr std::uint32_t fourcc(char ch0, char ch1, char ch2, char ch3) noexcept
{
    return (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch0)) << (BITS_PER_BYTE * 0)) |
           (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch1)) << (BITS_PER_BYTE * 1)) |
           (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch2)) << (BITS_PER_BYTE * 2)) |
           (static_cast<std::uint32_t>(static_cast<std::uint8_t>(ch3)) << (BITS_PER_BYTE * 3));
}

std::optional<PixelFormat> pixel_format(const DdsPixelFormat& ddpf)
{
    if ((ddpf.flags & ddpf_fourcc) != 0) {
        // Note: there is no distinction anymore between pre-multiplied and post-multiplied alpha,
        // so DXT2/3 and DXT4/5 are treated the same.
        switch (ddpf.fourcc) {
        case fourcc('D', 'X', 'T', '1'):
            return PixelFormat::bc1_unorm;

        case fourcc('D', 'X', 'T', '2'):
        case fourcc('D', 'X', 'T', '3'):
            return PixelFormat::bc2_unorm;

        case fourcc('D', 'X', 'T', '4'):
        case fourcc('D', 'X', 'T', '5'):
            return PixelFormat::bc3_unorm;
        }
    }
    // Unsupported or unknown format
    return {};
}

auto create_subresources(unsigned long width, unsigned long height, unsigned long depth,
                         unsigned long mip_levels, PixelFormat pixel_format)
{
    size_t data_offset = 0;
    auto   mip_width   = width;
    auto   mip_height  = height;
    auto   mip_depth   = depth;

    std::vector<struct TextureDesc::Subresource> subresources(mip_levels);
    for (size_t mip = 0; mip < mip_levels; ++mip) {
        auto& subresource = subresources[mip];

        switch (pixel_format) {
        case PixelFormat::bc1_unorm:
        case PixelFormat::bc2_unorm:
        case PixelFormat::bc3_unorm: {
            // Block compression
            auto bpe = (pixel_format == PixelFormat::bc1_unorm) ? BITS_PER_BYTE : 2 * BITS_PER_BYTE;
            auto blocks_w = (mip_width > 0) ? std::max(1UL, (mip_width + 3UL) / 4UL) : 0;
            auto blocks_h = (mip_height > 0) ? std::max(1UL, (mip_height + 3UL) / 4UL) : 0;

            subresource.stride       = blocks_w * bpe;
            subresource.depth_stride = subresource.stride * blocks_h;
            break;
        }

        default:
            assert(false);
            break;
        }

        subresource.data_offset = data_offset;
        subresource.data_size   = subresource.depth_stride * mip_depth;

        data_offset += subresource.data_size;
        mip_width  = std::max(1UL, mip_width / 2UL);
        mip_height = std::max(1UL, mip_height / 2UL);
        mip_depth  = std::max(1UL, mip_depth / 2UL);
    }
    return subresources;
}

} // namespace

TextureDesc load_texture(khepri::io::Stream& stream)
{
    auto dimension = TextureDimension::texture_2d;

    auto magic = stream.read_uint();
    verify(magic == DDS_MAGIC);

    auto size   = stream.read_uint();
    auto flags  = stream.read_uint();
    auto height = stream.read_uint();
    auto width  = stream.read_uint();

    // Ignore pitch/linear size, it's unreliable. We must calculate it ourselves.
    stream.read_uint();

    auto depth = stream.read_uint();
    if ((flags & ddsf_depth) != 0) {
        // 3D (volume) texture
        depth     = std::max(1UL, depth);
        dimension = TextureDimension::texture_3d;
    } else {
        depth = 1;
    }

    auto mip_levels = stream.read_uint();
    if ((flags & ddsf_mipmapcount) != 0) {
        mip_levels = std::max(1UL, mip_levels);
    } else {
        mip_levels = 1;
    }

    // Reserved data
    constexpr auto num_reserved_uints = 11;
    for (int i = 0; i < num_reserved_uints; ++i) {
        stream.read_uint();
    }

    // Pixel format
    auto           pf_size = stream.read_uint();
    DdsPixelFormat ddpf{};
    ddpf.flags        = stream.read_uint();
    ddpf.fourcc       = stream.read_uint();
    ddpf.rgb_bitcount = stream.read_uint();
    ddpf.r_mask       = stream.read_uint();
    ddpf.g_mask       = stream.read_uint();
    ddpf.b_mask       = stream.read_uint();
    ddpf.a_mask       = stream.read_uint();

    stream.read_uint(); // Caps. Ignored, contains no relevant data.
    auto caps2 = stream.read_uint();
    stream.read_uint(); // Caps3. Unused
    stream.read_uint(); // Caps4. Unused.
    stream.read_uint(); // Reserved

    verify(size == DDS_HEADER_SIZE);
    verify((flags & DDS_REQUIRED_FLAGS) == DDS_REQUIRED_FLAGS);
    verify(pf_size == DDS_PIXELFORMAT_SIZE);

    const bool dx10_header =
        (((ddpf.flags & ddpf_fourcc) != 0) && (ddpf.fourcc == fourcc('D', 'X', '1', '0')));
    if (dx10_header) {
        // DX10 extensions, unsupported as of yet
        throw khepri::io::InvalidFormatError();
    }

    if ((caps2 & ddscaps2_cubemap) != 0) {
        // Cubemap texture, unsupported as of yet
        throw khepri::io::InvalidFormatError();
    }

    auto format = pixel_format(ddpf);
    if (!format) {
        // Unsupported or unknown format
        throw khepri::io::InvalidFormatError();
    }

    if (width == 0 || height == 0) {
        // Invalid format
        throw khepri::io::InvalidFormatError();
    }

    auto subresources = create_subresources(width, height, depth, mip_levels, *format);

    size_t data_size = 0;
    for (const auto& subresource : subresources) {
        data_size += subresource.data_size;
    }

    std::vector<std::uint8_t> data(data_size);
    if (stream.read(data.data(), data_size) != data_size) {
        throw khepri::io::InvalidFormatError();
    }

    const size_t depth_array_size = (dimension == TextureDimension::texture_3d) ? depth : 0;

    return TextureDesc(dimension, width, height, depth_array_size, mip_levels, *format,
                       std::move(subresources), std::move(data));
}

} // namespace khepri::renderer::io
