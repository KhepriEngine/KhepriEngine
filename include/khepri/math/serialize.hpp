#pragma once

#include "vector2.hpp"
#include "vector3.hpp"
#include "vector4.hpp"

#include <khepri/io/serialize.hpp>

namespace khepri::io {

/// Specialization of #khepri::io::SerializeTraits for #khepri::Vector2
template <>
struct SerializeTraits<Vector2>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const Vector2& value)
    {
        s.write(value.x);
        s.write(value.y);
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static Vector2 deserialize(Deserializer& d)
    {
        Vector2 v;
        v.x = d.read<Vector2::ComponentType>();
        v.y = d.read<Vector2::ComponentType>();
        return v;
    }
};

/// Specialization of #khepri::io::SerializeTraits for #khepri::Vector3
template <>
struct SerializeTraits<Vector3>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const Vector3& value)
    {
        s.write(value.x);
        s.write(value.y);
        s.write(value.z);
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static Vector3 deserialize(Deserializer& d)
    {
        Vector3 v;
        v.x = d.read<Vector3::ComponentType>();
        v.y = d.read<Vector3::ComponentType>();
        v.z = d.read<Vector3::ComponentType>();
        return v;
    }
};

/// Specialization of #khepri::io::SerializeTraits for #khepri::Vector4
template <>
struct SerializeTraits<Vector4>
{
    /// \see #khepri::io::SerializeTraits::serialize
    static void serialize(Serializer& s, const Vector4& value)
    {
        s.write(value.x);
        s.write(value.y);
        s.write(value.z);
        s.write(value.w);
    }

    /// \see #khepri::io::SerializeTraits::deserialize
    static Vector4 deserialize(Deserializer& d)
    {
        Vector4 v;
        v.x = d.read<Vector4::ComponentType>();
        v.y = d.read<Vector4::ComponentType>();
        v.z = d.read<Vector4::ComponentType>();
        v.w = d.read<Vector4::ComponentType>();
        return v;
    }
};

} // namespace khepri::io
