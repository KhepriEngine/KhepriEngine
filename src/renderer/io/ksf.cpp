#pragma once

#include <khepri/io/container_stream.hpp>
#include <khepri/io/exceptions.hpp>
#include <khepri/io/serialize.hpp>
#include <khepri/log/log.hpp>
#include <khepri/math/serialize.hpp>
#include <khepri/renderer/exceptions.hpp>
#include <khepri/renderer/io/ksf.hpp>
#include <khepri/renderer/io/serialize.hpp>

namespace khepri::renderer::io {

namespace {
constexpr log::Logger LOG("ksf");

constexpr khepri::io::ContainerStream::content_type_id CONTENT_ID_KSF = 0xd086def7;

void require(bool condition)
{
    if (!condition) {
        throw khepri::io::InvalidFormatError();
    }
}
} // namespace

Shader load_ksf(khepri::io::Stream& stream)
{
    if (!stream.readable() || !stream.seekable()) {
        throw ArgumentError();
    }

    khepri::io::ContainerStream container(stream, CONTENT_ID_KSF,
                                          khepri::io::ContainerStream::open_mode::read);

    auto size = container.seek(0, khepri::io::seek_origin::end);
    container.seek(0, khepri::io::seek_origin::begin);
    std::vector<std::uint8_t> buffer(size);
    if (container.read(buffer.data(), buffer.size()) != buffer.size()) {
        throw khepri::io::Error("unable to read stream");
    }

    try {
        khepri::io::Deserializer deserializer(buffer);
        return deserializer.read<Shader>();
    } catch (const khepri::io::Error&) {
        throw khepri::io::InvalidFormatError();
    }
}

void write_ksf(const Shader& shader, khepri::io::Stream& stream)
{
    if (!stream.writable() || !stream.seekable()) {
        throw ArgumentError();
    }

    khepri::io::ContainerStream container(stream, CONTENT_ID_KSF,
                                          khepri::io::ContainerStream::open_mode::write);

    khepri::io::Serializer serializer;
    serializer.write(shader);
    auto data = serializer.data();
    if (container.write(data.data(), data.size()) != data.size()) {
        throw khepri::io::Error("unable to write stream");
    }
    container.close();
}

} // namespace khepri::renderer::io
