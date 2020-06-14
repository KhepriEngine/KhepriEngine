#pragma once

#include <khepri/io/serialize.hpp>
#include <khepri/renderer/model.hpp>

namespace khepri::io {

/// Specialization of #khepri::io::SerializeTraits for #khepri::renderer::Model::Mesh::Vertex
template <>
struct SerializeTraits<renderer::Model::Mesh::Vertex>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const renderer::Model::Mesh::Vertex& value)
    {
        s.write(value.position);
        s.write(value.normal);
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static renderer::Model::Mesh::Vertex deserialize(Deserializer& d)
    {
        renderer::Model::Mesh::Vertex vertex;
        vertex.position = d.read<Vector3>();
        vertex.normal   = d.read<Vector3>();
        return vertex;
    }
};

/// Specialization of #khepri::io::SerializeTraits for #khepri::renderer::Model::Mesh
template <>
struct SerializeTraits<renderer::Model::Mesh>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const renderer::Model::Mesh& value)
    {
        s.write(value.vertices);
        s.write(value.indices);
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static renderer::Model::Mesh deserialize(Deserializer& d)
    {
        auto vertices = d.read<std::vector<renderer::Model::Mesh::Vertex>>();
        auto indices  = d.read<std::vector<renderer::Model::Index>>();
        return renderer::Model::Mesh{std::move(vertices), std::move(indices)};
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
        auto meshes = d.read<std::vector<renderer::Model::Mesh>>();
        return renderer::Model(std::move(meshes));
    }
};

} // namespace khepri::io
