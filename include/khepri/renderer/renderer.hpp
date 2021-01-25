#pragma once

#include "camera.hpp"
#include "material.hpp"
#include "material_id.hpp"
#include "mesh.hpp"
#include "mesh_id.hpp"
#include "mesh_instance.hpp"
#include "shader_id.hpp"
#include "texture.hpp"
#include "texture_id.hpp"

#include <khepri/math/matrix.hpp>
#include <khepri/math/size.hpp>

#include <gsl/gsl-lite.hpp>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string_view>
#include <vector>

namespace khepri::renderer {

/**
 * \brief Interface for renderers
 *
 * This interface provides a technology-independent interface to various renderers.
 */
class Renderer
{
public:
    /**
     * Callback used to load files on demand.
     * \param the path of the file.
     * \return the file's content or std::none if the file cannot be found or read.
     */
    using FileLoader =
        std::function<std::optional<std::vector<std::uint8_t>>(const std::filesystem::path&)>;

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
     * \brief Creates a shader by compiling a shader source file.
     *
     * A shader is a pair of vertex and pixel shaders loaded from the same file.
     * The vertex shader's entry point is called @c vs_main and the pixel shader's entry point is
     * called @c ps_main.
     *
     * \param path the path to the source file of the shader
     * \param loader the loader used to load files
     *
     * In case the shader source files contain includes of other files, @a loader is invoked
     * multiple times to load the needed files.
     */
    virtual ShaderId create_shader(const std::filesystem::path& path, const FileLoader& loader) = 0;

    /**
     * \brief Destroys a shader.
     *
     * \param[in] shader the ID of the shader to destroy.
     */
    virtual void destroy_shader(ShaderId shader) = 0;

    /**
     * \brief Creates a material to be used when rendering meshes.
     *
     * After creating a material, specify the returned ID in a #khepri::renderer::MeshInstance to
     * render that mesh with the created material.
     *
     * \param material the material to create.
     */
    virtual MaterialId create_material(const Material& material) = 0;

    /**
     * \brief Destroys a material.
     *
     * \param[in] material the material to destroy.
     */
    virtual void destroy_material(MaterialId material) = 0;

    /**
     * \brief Creates a texture from binary data.
     *
     * Textures can be referenced by meshes for binding to shader arguments during rendering.
     *
     * \param[in] texture the texture data.
     *
     * \return the ID of the newly created texture.
     */
    virtual TextureId create_texture(const Texture& texture) = 0;

    /**
     * \brief Destroys a texture.
     *
     * \param[in] texture the ID of the texture to destroy.
     */
    virtual void destroy_texture(TextureId texture) = 0;

    /**
     * \brief Creates a mesh from raw mesh data.
     *
     * The returned value can be used in calls to the renderer to identify the created
     * mesh. To free the resources allocated for the mesh, call #destroy_mesh.
     *
     * \param[in] mesh the raw mesh data to create a mesh from.
     *
     * \return the ID of the created mesh.
     */
    virtual MeshId create_mesh(const Mesh& mesh) = 0;

    /**
     * \brief Destroys a mesh.
     *
     * Created meshes consume resources. Meshes that are no longer intended to be used, should be
     * destroyed to free those resources. After destroying a mesh, the ID becomes invalid and can be
     * reused by the renderer for a new mesh.
     *
     * \param[in] mesh_id the ID of the mesh to destroy.
     *
     */
    virtual void destroy_mesh(MeshId mesh_id) = 0;

    /**
     * Clears the render target.
     */
    virtual void clear() = 0;

    /**
     * Presents all the rendered objects.
     */
    virtual void present() = 0;

    /**
     * Renders a collection of meshe instances.
     *
     * \param[in] meshes a collection of mesh instances to render.
     * \param[in] camera the camera to render them with.
     */
    virtual void render_meshes(gsl::span<const MeshInstance> meshes, const Camera& camera) = 0;
};

} // namespace khepri::renderer
