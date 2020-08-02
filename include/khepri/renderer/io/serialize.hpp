#pragma once

#include <khepri/io/serialize.hpp>
#include <khepri/renderer/model.hpp>
#include <khepri/renderer/shader.hpp>

namespace khepri::io {

/// Specialization of #khepri::io::SerializeTraits for #khepri::renderer::Mesh::Vertex
template <>
struct SerializeTraits<renderer::Mesh::Vertex>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const renderer::Mesh::Vertex& value)
    {
        s.write(value.position);
        s.write(value.normal);
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static renderer::Mesh::Vertex deserialize(Deserializer& d)
    {
        renderer::Mesh::Vertex vertex;
        vertex.position = d.read<Vector3>();
        vertex.normal   = d.read<Vector3>();
        return vertex;
    }
};

/// Specialization of #khepri::io::SerializeTraits for #khepri::renderer::Mesh
template <>
struct SerializeTraits<renderer::Mesh>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const renderer::Mesh& value)
    {
        s.write(value.vertices);
        s.write(value.indices);
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static renderer::Mesh deserialize(Deserializer& d)
    {
        auto vertices = d.read<std::vector<renderer::Mesh::Vertex>>();
        auto indices  = d.read<std::vector<renderer::Mesh::Index>>();
        return renderer::Mesh{std::move(vertices), std::move(indices)};
    }
};

/// Specialization of #khepri::io::SerializeTraits for #khepri::renderer::Model
template <>
struct SerializeTraits<renderer::Model>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const renderer::Model& value)
    {
        s.write(value.meshes());
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static renderer::Model deserialize(Deserializer& d)
    {
        auto meshes = d.read<std::vector<renderer::Mesh>>();
        return renderer::Model(std::move(meshes));
    }
};

/// Specialization of #khepri::io::SerializeTraits for #khepri::renderer::Shader
template <>
struct SerializeTraits<renderer::Shader>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const renderer::Shader& value)
    {
        s.write(value.data());
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static renderer::Shader deserialize(Deserializer& d)
    {
        auto data = d.read<std::vector<std::uint32_t>>();
        return renderer::Shader(data);
    }
};

} // namespace khepri::io
