#pragma once

#include "mesh.hpp"
#include "renderable_mesh_id.hpp"
#include "renderable_mesh_instance.hpp"
#include "shader.hpp"

#include <khepri/math/matrix.hpp>
#include <khepri/math/size.hpp>

#include <gsl/gsl-lite.hpp>

#include <memory>

namespace khepri::renderer {

class Camera;

using pipeline_id = std::size_t;

/**
 * Description of a pipeline stage
 */
struct StageDesc
{
    /// Vertex shader for this stage
    const Shader* vertex_shader;

    /// Fragment shader for this stage
    const Shader* fragment_shader;
};

/**
 * Describes a render pipeline.
 *
 * A render pipeline consists of a single render stage.
 */
struct PipelineDesc
{
    /**
     * Stage in this pipeline
     */
    StageDesc stage;
};

/**
 * \brief Interface for renderers
 *
 * This interface provides a technology-independent interface to various renderers.
 */
class Renderer
{
public:
    Renderer()          = default;
    virtual ~Renderer() = default;

    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&)      = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    /**
     * Returns the size of the rendering area
     */
    [[nodiscard]] virtual Size render_size() const noexcept = 0;

    /**
     * \brief Creates a render pipeline.
     *
     * Meshes are rendered by a render pipeline. A render pipeline contains render stages with
     * shaders and render targets and their dependencies.
     *
     * \param[in] desc a description of the pipeline to render.
     *
     * \return the ID of the newly created pipeline.
     */
    virtual pipeline_id create_render_pipeline(const PipelineDesc& desc) = 0;

    /**
     * \brief Destroys a render pipeline.
     *
     * \param[in] pipeline the ID of the render pipeline to destroy.
     */
    virtual void destroy_render_pipeline(pipeline_id pipeline) = 0;

    /**
     * \brief Creates a renderable mesh from a normal mesh.
     *
     * A renderable mesh is a mesh that has been optimized for rendering and has rendering resources
     * allocated. The returned value can be used in calls to the renderer to identify the created
     * mesh. To free the resources allocated for the renderable mesh, call #destroy_renderable_mesh.
     *
     * \param[in] mesh the normal mesh to create a renderable mesh from.
     *
     * \return the ID of the created renderable mesh.
     */
    virtual renderable_mesh_id create_renderable_mesh(const Mesh& mesh) = 0;

    /**
     * \brief Destroys a renderable mesh.
     *
     * Renderable meshes consume resources. Renderables meshes that are no longer intended to be
     * used, should be destroyed to free those resources. After the destroying a renderable mesh,
     * the ID becomes invalid and can be reused by the renderer for a new renderable mesh.
     *
     * \param[in] mesh_id the ID of the renderable mesh to destroy.
     *
     */
    virtual void destroy_renderable_mesh(renderable_mesh_id mesh_id) = 0;

    /**
     * Renders a collection of meshes.
     *
     * \param[in] pipeline the ID of the pipeline to use for rendering
     * \param[in] meshes a collection of meshes to render.
     * \param[in] camera the camera to render them with.
     */
    virtual void render_meshes(pipeline_id pipeline, gsl::span<const RenderableMeshInstance> meshes,
                               const Camera& camera) = 0;
};

} // namespace khepri::renderer
