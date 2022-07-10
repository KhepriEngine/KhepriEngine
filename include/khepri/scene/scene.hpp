#pragma once

#include "scene_object.hpp"

#include <memory>
#include <set>

namespace khepri::scene {

/**
 * \brief A scene
 *
 * A scene is a collection of scene objects and represents an interactive space.
 */
class Scene
{
public:
    Scene()          = default;
    virtual ~Scene() = default;

    Scene(const Scene&) = delete;
    Scene(Scene&&)      = delete;
    Scene& operator=(const Scene&) = delete;
    Scene& operator=(Scene&&) = delete;

    /// Returns the objects in the scene
    [[nodiscard]] const auto& objects() const noexcept
    {
        return m_objects;
    }

    /**
     * Adds an object to the scene.
     *
     * Does nothing if the object is already added.
     */
    void add_object(const std::shared_ptr<SceneObject>& object)
    {
        m_objects.insert(object);
    }

    /**
     * Remvoes an object from the scene.
     *
     * Does nothing if the object is not in the scene.
     */
    void remove_object(const std::shared_ptr<SceneObject>& object)
    {
        m_objects.erase(object);
    }

private:
    std::set<std::shared_ptr<SceneObject>> m_objects;
};

} // namespace khepri::scene
