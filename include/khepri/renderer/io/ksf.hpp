#pragma once

#include <khepri/io/stream.hpp>
#include <khepri/renderer/shader.hpp>

#include <memory>
#include <string>

namespace khepri::renderer::io {

/**
 * Loads a shader from a stream with Khepri Shader File data.
 * @throw khepri::ArgumentError the stream is not readable and seekable.
 */
Shader load_ksf(khepri::io::Stream& stream);

/**
 * Writes a shader to a stream as Khepri Shader File data.
 * @throw khepri::ArgumentError the stream is not writable and seekable.
 */
void write_ksf(const Shader& shader, khepri::io::Stream& stream);

} // namespace khepri::renderer::io
