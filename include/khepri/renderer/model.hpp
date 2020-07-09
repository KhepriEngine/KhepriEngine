#pragma once

#include <khepri/math/sphere.hpp>
#include <khepri/physics/collision_mesh.hpp>
#include <khepri/renderer/mesh.hpp>

#include <vector>

namespace khepri::renderer {

/**
 * \brief A game model
 *
 * A model is a collection of data structures related to its meshes that make up a single game
 * entity
 */
class Model
{
public:
    /// Constructs a model from meshes
    Model(std::vector<Mesh> meshes);

    /// Returns the meshes in this model
    [[nodiscard]] const auto& meshes() const noexcept
    {
        return m_meshes;
    }

    /// Returns the bounding sphere of this model
    [[nodiscard]] const auto& bounding_sphere() const noexcept
    {
        return m_bounding_sphere;
    }

    /// Returns the collision mesh of this model
    [[nodiscard]] const auto& collision_mesh() const noexcept
    {
        return m_collision_mesh;
    }

private:
    std::vector<Mesh>      m_meshes;
    Sphere                 m_bounding_sphere;
    physics::CollisionMesh m_collision_mesh;
};

} // namespace khepri::renderer
