#pragma once

#include <khepri/io/stream.hpp>
#include <khepri/renderer/texture_desc.hpp>

namespace khepri ::renderer ::io {

/**
 * Loads a texture description from a stream.
 *
 * Only the DDS and TARGA formats are supported by this function.
 *
 * @throw khepri::argument_error the stream is not readable and seekable.
 * @throw khepri::io::invalid_format the stream is not a valid texture.
 */
TextureDesc load_texture(khepri::io::Stream& stream);

} // namespace khepri::renderer::io
