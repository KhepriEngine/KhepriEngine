#include "shader_stream_factory.hpp"

#include <khepri/exceptions.hpp>
#include <khepri/log/log.hpp>
#include <khepri/renderer/camera.hpp>
#include <khepri/renderer/diligent/renderer.hpp>

#include <EngineFactoryD3D11.h>
#include <MapHelper.hpp>
#include <Sampler.h>
#include <Texture.h>

#include <unordered_map>

using namespace Diligent;

namespace khepri::renderer::diligent {
namespace {

constexpr khepri::log::Logger LOG("diligent");

struct InstanceConstantBuffer
{
    Matrix world;
    Matrix world_inv;
};

struct ViewConstantBuffer
{
    Matrix view_proj;
};

CULL_MODE to_cull_mode(Material::CullMode cull_mode) noexcept
{
    switch (cull_mode) {
    case Material::CullMode::none:
        return CULL_MODE_NONE;
    case Material::CullMode::back:
        return CULL_MODE_BACK;
    case Material::CullMode::front:
        return CULL_MODE_FRONT;
    default:
        break;
    }
    assert(false);
    return CULL_MODE_NONE;
}

RESOURCE_DIMENSION to_resource_dimension(TextureDimension dimension, bool is_array)
{
    switch (dimension) {
    case TextureDimension::texture_1d:
        return is_array ? RESOURCE_DIM_TEX_1D_ARRAY : RESOURCE_DIM_TEX_1D;
    case TextureDimension::texture_2d:
        return is_array ? RESOURCE_DIM_TEX_2D_ARRAY : RESOURCE_DIM_TEX_2D;
    case TextureDimension::texture_3d:
        // 3D textures cannot be arrays
        assert(!is_array);
        return RESOURCE_DIM_TEX_3D;
    case TextureDimension::texture_cubemap:
        return is_array ? RESOURCE_DIM_TEX_CUBE_ARRAY : RESOURCE_DIM_TEX_CUBE;
    }
    assert(false);
    return RESOURCE_DIM_UNDEFINED;
}

TEXTURE_FORMAT to_texture_format(PixelFormat format)
{
    switch (format) {
    case PixelFormat::bc1_unorm:
        return TEX_FORMAT_BC1_UNORM;
    case PixelFormat::bc2_unorm:
        return TEX_FORMAT_BC2_UNORM;
    case PixelFormat::bc3_unorm:
        return TEX_FORMAT_BC3_UNORM;
    }
    assert(false);
    return TEX_FORMAT_UNKNOWN;
}

// helper type for a variant visitor
template <class... Ts>
struct Overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

} // namespace

struct Renderer::ShaderData
{
    Diligent::RefCntAutoPtr<Diligent::IShader> vertex_shader;
    Diligent::RefCntAutoPtr<Diligent::IShader> pixel_shader;
};

struct Renderer::MeshData
{
    using Index = khepri::renderer::Mesh::Index;

    Index                  index_count{0};
    RefCntAutoPtr<IBuffer> vertex_buffer;
    RefCntAutoPtr<IBuffer> index_buffer;
};

struct Renderer::MaterialData
{
    struct Param
    {
        std::string                               name;
        khepri::renderer::Material::PropertyValue default_value;
        size_t                                    buffer_offset;
    };

    Diligent::RefCntAutoPtr<Diligent::IPipelineState>         pipeline;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> shader_resource_binding;
    Diligent::RefCntAutoPtr<Diligent::IBuffer>                param_buffer;
    std::vector<Param>                                        params;
};

struct Renderer::TextureData
{
    Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
    Diligent::ITextureView*                     shader_view{};
};

Renderer::Renderer(const NativeWindow& window)
{
    auto* factory = GetEngineFactoryD3D11();

    EngineD3D11CreateInfo engine_ci;
#ifndef NDEBUG
    engine_ci.SetValidationLevel(VALIDATION_LEVEL_2);
#endif
    factory->CreateDeviceAndContextsD3D11(engine_ci, &m_device, &m_context);

    SwapChainDesc      swapchain_desc;
    FullScreenModeDesc fullscreenmode_desc;
    factory->CreateSwapChainD3D11(m_device, m_context, swapchain_desc, fullscreenmode_desc, window,
                                  &m_swapchain);

    // Create constants buffers for vertex shader
    {
        BufferDesc desc;
        desc.Name           = "VS Instance Constants";
        desc.Size           = sizeof(InstanceConstantBuffer);
        desc.Usage          = USAGE_DYNAMIC;
        desc.BindFlags      = BIND_UNIFORM_BUFFER;
        desc.CPUAccessFlags = CPU_ACCESS_WRITE;
        m_device->CreateBuffer(desc, nullptr, &m_constants_instance);
    }

    {
        BufferDesc desc;
        desc.Name           = "VS View Constants";
        desc.Size           = sizeof(ViewConstantBuffer);
        desc.Usage          = USAGE_DYNAMIC;
        desc.BindFlags      = BIND_UNIFORM_BUFFER;
        desc.CPUAccessFlags = CPU_ACCESS_WRITE;
        m_device->CreateBuffer(desc, nullptr, &m_constants_view);
    }

    {
        SamplerDesc desc;
        desc.Name      = "LinearSampler";
        desc.MinFilter = FILTER_TYPE_LINEAR;
        desc.MagFilter = FILTER_TYPE_LINEAR;
        desc.MipFilter = FILTER_TYPE_LINEAR;
        desc.AddressU  = TEXTURE_ADDRESS_WRAP;
        desc.AddressV  = TEXTURE_ADDRESS_WRAP;
        m_device->CreateSampler(desc, &m_linear_sampler);
    }
}

Renderer::~Renderer() = default;

void Renderer::render_size(const Size& size)
{
    m_swapchain->Resize(size.width, size.height);
}

Size Renderer::render_size() const noexcept
{
    const auto& desc = m_swapchain->GetDesc();
    return {desc.Width, desc.Height};
}

ShaderId Renderer::create_shader(const std::filesystem::path& path, const FileLoader& loader)
{
    RefCntAutoPtr<ShaderStreamFactory> factory(MakeNewRCObj<ShaderStreamFactory>()(loader));

    const auto& create_shader_object = [&, this](const std::string& path, SHADER_TYPE shader_type,
                                                 const std::string& entrypoint) {
        ShaderCreateInfo ci;
        ci.Desc.Name                  = path.c_str();
        ci.Desc.ShaderType            = shader_type;
        ci.FilePath                   = path.c_str();
        ci.pShaderSourceStreamFactory = factory;
        ci.SourceLanguage             = SHADER_SOURCE_LANGUAGE_HLSL;
        ci.EntryPoint                 = entrypoint.c_str();
        Diligent::RefCntAutoPtr<Diligent::IShader> shader;
        m_device->CreateShader(ci, &shader);
        return shader;
    };

    ShaderData data = {create_shader_object(path.string(), SHADER_TYPE_VERTEX, "vs_main"),
                       create_shader_object(path.string(), SHADER_TYPE_PIXEL, "ps_main")};

    auto it = std::find_if(m_shaders.begin(), m_shaders.end(), [](const auto& s) {
        return s.vertex_shader == nullptr && s.pixel_shader == nullptr;
    });
    if (it == m_shaders.end()) {
        it = m_shaders.emplace(m_shaders.end());
    }
    *it = std::move(data);
    return it - m_shaders.begin();
}

void Renderer::destroy_shader(ShaderId shader_id)
{
    if (shader_id >= m_shaders.size()) {
        throw ArgumentError();
    }
    m_shaders[shader_id] = {};
}

std::vector<std::string>
Renderer::determine_dynamic_material_variables(const ShaderData&                      shader,
                                               const std::vector<Material::Property>& properties)
{
    // Any top-level material property with the same name as these is an error
    const std::unordered_map<std::string, SHADER_RESOURCE_TYPE> predefined_variables{
        {"InstanceConstants", SHADER_RESOURCE_TYPE_CONSTANT_BUFFER},
        {"ViewConstants", SHADER_RESOURCE_TYPE_CONSTANT_BUFFER},
        {"LinearSampler", SHADER_RESOURCE_TYPE_SAMPLER},
        {"Material", SHADER_RESOURCE_TYPE_CONSTANT_BUFFER},
    };

    // Collect all top-level shader resources, they must be matched by top-level material properties
    std::unordered_map<std::string, SHADER_RESOURCE_TYPE> shader_variables;
    for (const auto& shader : {shader.vertex_shader, shader.pixel_shader}) {
        Uint32 count = shader->GetResourceCount();
        for (Uint32 i = 0; i < count; ++i) {
            ShaderResourceDesc desc;
            shader->GetResourceDesc(i, desc);

            const auto it = predefined_variables.find(desc.Name);
            if (it == predefined_variables.end()) {
                // No predefined variable, remember it as a top-level shader variable
                shader_variables[desc.Name] = desc.Type;
            } else if (it->second != desc.Type) {
                LOG.error("type of property \"{}\" in material does not match shader variable type",
                          desc.Name);
                throw ArgumentError();
            }
        }
    }

    // Validate properties and collect all dynamic top-level materials
    std::vector<std::string> dynamic_variables;
    for (const auto& p : properties) {
        if (!std::holds_alternative<TextureId>(p.default_value)) {
            // Non-texture properties are fine, they are in a cbuffer and not top-level variables
            continue;
        }

        if (predefined_variables.find(p.name) != predefined_variables.end()) {
            // Invalid variable name
            throw ArgumentError();
        }

        // See if the property has a matching shader variable
        const auto it = shader_variables.find(p.name);
        if (it == shader_variables.end()) {
            LOG.error("missing shader variable for property \"{}\"", p.name);
            throw ArgumentError();
        }
        if (it->second != SHADER_RESOURCE_TYPE_TEXTURE_SRV) {
            LOG.error("mismatch for shader variable type for property \"{}\"", p.name);
            throw ArgumentError();
        }
        // We've seen this one, remove it
        shader_variables.erase(it);

        dynamic_variables.push_back(p.name);
    }
    dynamic_variables.emplace_back("Material");

    if (!shader_variables.empty()) {
        // Not all shader variables have been accounted for
        LOG.error("missing material property for shader variable \"{}\"",
                  shader_variables.begin()->first);
        throw ArgumentError();
    }
    return dynamic_variables;
}

MaterialId Renderer::create_material(const khepri::renderer::Material& material)
{
    if (material.shader >= m_shaders.size()) {
        throw ArgumentError();
    }

    auto& shader = m_shaders[material.shader];
    if (shader.vertex_shader == nullptr || shader.pixel_shader == nullptr) {
        throw ArgumentError();
    }

    const auto dynamic_variables =
        determine_dynamic_material_variables(shader, material.properties);

    GraphicsPipelineStateCreateInfo ci;
    ci.PSODesc.PipelineType                          = PIPELINE_TYPE_GRAPHICS;
    ci.GraphicsPipeline.NumRenderTargets             = 1;
    ci.GraphicsPipeline.RTVFormats[0]                = m_swapchain->GetDesc().ColorBufferFormat;
    ci.GraphicsPipeline.DSVFormat                    = m_swapchain->GetDesc().DepthBufferFormat;
    ci.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;
    ci.GraphicsPipeline.RasterizerDesc.CullMode      = to_cull_mode(material.cull_mode);

    constexpr auto                                 num_layout_elements = 5;
    std::array<LayoutElement, num_layout_elements> layout{
        LayoutElement{0, 0, 3, VT_FLOAT32, false, offsetof(Mesh::Vertex, position),
                      sizeof(Mesh::Vertex)},
        LayoutElement{1, 0, 3, VT_FLOAT32, false, offsetof(Mesh::Vertex, normal),
                      sizeof(Mesh::Vertex)},
        LayoutElement{2, 0, 3, VT_FLOAT32, false, offsetof(Mesh::Vertex, tangent),
                      sizeof(Mesh::Vertex)},
        LayoutElement{3, 0, 3, VT_FLOAT32, false, offsetof(Mesh::Vertex, binormal),
                      sizeof(Mesh::Vertex)},
        LayoutElement{4, 0, 2, VT_FLOAT32, false, offsetof(Mesh::Vertex, uv),
                      sizeof(Mesh::Vertex)}};
    ci.GraphicsPipeline.InputLayout.LayoutElements = layout.data();
    ci.GraphicsPipeline.InputLayout.NumElements    = static_cast<Uint32>(layout.size());

    ci.pVS = shader.vertex_shader;
    ci.pPS = shader.pixel_shader;

    // Mark all material properties as dynamic (the rest is static by default)
    std::vector<ShaderResourceVariableDesc> variables;
    for (const auto& var : dynamic_variables) {
        variables.emplace_back(SHADER_TYPE_VERTEX, var.c_str(),
                               SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC);
        variables.emplace_back(SHADER_TYPE_PIXEL, var.c_str(),
                               SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC);
    }

    ci.PSODesc.ResourceLayout.Variables    = variables.data();
    ci.PSODesc.ResourceLayout.NumVariables = static_cast<Uint32>(variables.size());

    MaterialData data;
    m_device->CreateGraphicsPipelineState(ci, &data.pipeline);

    if (auto* var =
            data.pipeline->GetStaticVariableByName(SHADER_TYPE_VERTEX, "InstanceConstants")) {
        var->Set(m_constants_instance);
    }
    if (auto* var = data.pipeline->GetStaticVariableByName(SHADER_TYPE_VERTEX, "ViewConstants")) {
        var->Set(m_constants_view);
    }
    if (auto* var = data.pipeline->GetStaticVariableByName(SHADER_TYPE_PIXEL, "LinearSampler")) {
        var->Set(m_linear_sampler);
    }
    data.pipeline->CreateShaderResourceBinding(&data.shader_resource_binding, true);

    const auto& property_size = [](const Material::PropertyValue& value) -> Uint32 {
        // Every type has its own size, except for textures, which don't take up space.
        return std::visit(Overloaded{[&](const TextureId& /*texture_id*/) -> Uint32 { return 0; },
                                     [&](const auto& value) -> Uint32 { return sizeof(value); }},
                          value);
    };

    Uint32 buffer_size = 0;
    data.params.reserve(material.properties.size());
    for (const auto& p : material.properties) {
        data.params.push_back({p.name, p.default_value, buffer_size});
        buffer_size += property_size(p.default_value);
        // Align next parameter to 16 bytes
        constexpr auto param_alignment = 16;
        buffer_size = (buffer_size + param_alignment - 1) / param_alignment * param_alignment;
    }

    // Create the material properties buffer
    if (buffer_size > 0) {
        BufferDesc desc;
        desc.Name           = "Material Constants";
        desc.Size           = buffer_size;
        desc.Usage          = USAGE_DYNAMIC;
        desc.BindFlags      = BIND_UNIFORM_BUFFER;
        desc.CPUAccessFlags = CPU_ACCESS_WRITE;
        m_device->CreateBuffer(desc, nullptr, &data.param_buffer);
    }

    auto it = std::find_if(m_materials.begin(), m_materials.end(),
                           [](const auto& m) { return m.pipeline == nullptr; });
    if (it == m_materials.end()) {
        it = m_materials.emplace(m_materials.end());
    }
    *it = std::move(data);
    return it - m_materials.begin();
}

void Renderer::destroy_material(MaterialId material_id)
{
    if (material_id >= m_materials.size()) {
        throw ArgumentError();
    }
    m_materials[material_id] = {};
}

TextureId Renderer::create_texture(const Texture& texture)
{
    TextureDesc desc;
    desc.Type      = to_resource_dimension(texture.dimension(), texture.array_size() > 0);
    desc.Width     = static_cast<Uint32>(texture.width());
    desc.Height    = static_cast<Uint32>(texture.height());
    desc.Format    = to_texture_format(texture.pixel_format());
    desc.MipLevels = static_cast<Uint32>(texture.mip_levels());
    desc.Usage     = USAGE_IMMUTABLE;
    desc.BindFlags = BIND_SHADER_RESOURCE;

    const std::size_t array_size        = std::max<std::size_t>(1, texture.array_size());
    const std::size_t subresource_count = array_size * texture.mip_levels();

    if (texture.dimension() == TextureDimension::texture_3d) {
        desc.Depth = static_cast<Uint32>(texture.depth()); // NOLINT - union access
    } else {
        desc.ArraySize = static_cast<Uint32>(array_size); // NOLINT - union access
    }

    std::vector<TextureSubResData> subresources(subresource_count);

    auto subresource = subresources.begin();
    for (std::size_t index = 0; index < array_size; ++index) {
        for (std::size_t mip = 0; mip < texture.mip_levels(); ++mip, ++subresource) {
            assert(subresource != subresources.end());
            const auto& src          = texture.subresource(texture.subresource_index(mip, index));
            subresource->pData       = texture.data().data() + src.data_offset;
            subresource->Stride      = static_cast<Uint32>(src.stride);
            subresource->DepthStride = static_cast<Uint32>(src.depth_stride);
        }
    }

    Diligent::TextureData texdata;
    texdata.pSubResources   = subresources.data();
    texdata.NumSubresources = static_cast<Uint32>(subresources.size());

    TextureData data;
    m_device->CreateTexture(desc, &texdata, &data.texture);
    data.shader_view = data.texture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

    auto it = std::find_if(m_textures.begin(), m_textures.end(),
                           [](const auto& t) { return t.texture == nullptr; });
    if (it == m_textures.end()) {
        it = m_textures.emplace(m_textures.end());
    }
    *it = std::move(data);
    return it - m_textures.begin();
}

void Renderer::destroy_texture(TextureId texture_id)
{
    if (texture_id >= m_textures.size()) {
        throw ArgumentError();
    }
    m_textures[texture_id] = {};
}

MeshId Renderer::create_mesh(const Mesh& mesh)
{
    MeshData data;
    data.index_count = static_cast<Mesh::Index>(mesh.indices.size());

    {
        using Vertex = khepri::renderer::Mesh::Vertex;
        BufferData bufdata{mesh.vertices.data(),
                           static_cast<Uint32>(mesh.vertices.size() * sizeof(Vertex))};
        BufferDesc desc;
        desc.Size      = bufdata.DataSize;
        desc.BindFlags = BIND_VERTEX_BUFFER;
        desc.Usage     = USAGE_IMMUTABLE;
        m_device->CreateBuffer(desc, &bufdata, &data.vertex_buffer);
    }

    {
        BufferData bufdata{mesh.indices.data(),
                           static_cast<Uint32>(mesh.indices.size() * sizeof(Mesh::Index))};
        BufferDesc desc;
        desc.Size      = bufdata.DataSize;
        desc.BindFlags = BIND_INDEX_BUFFER;
        desc.Usage     = USAGE_IMMUTABLE;
        m_device->CreateBuffer(desc, &bufdata, &data.index_buffer);
    }

    auto it = std::find_if(m_meshes.begin(), m_meshes.end(), [](const auto& m) {
        return m.index_count == 0 && m.vertex_buffer == nullptr && m.index_buffer == nullptr;
    });
    if (it == m_meshes.end()) {
        it = m_meshes.emplace(m_meshes.end());
    }
    *it = std::move(data);
    return it - m_meshes.begin();
}

void Renderer::destroy_mesh(MeshId mesh_id)
{
    if (mesh_id >= m_meshes.size()) {
        throw ArgumentError();
    }
    m_meshes[mesh_id] = {};
}

void Renderer::clear()
{
    const std::array<float, 4> color{0, 0, 0, 1};

    auto* rtv = m_swapchain->GetCurrentBackBufferRTV();
    auto* dsv = m_swapchain->GetDepthBufferDSV();

    m_context->SetRenderTargets(1, &rtv, dsv, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_context->ClearRenderTarget(rtv, color.data(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_context->ClearDepthStencil(dsv, CLEAR_DEPTH_FLAG | CLEAR_STENCIL_FLAG, 1.0F, 0,
                                 RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void Renderer::present()
{
    m_swapchain->Present();
}

void Renderer::apply_material_params(MaterialData&                        material,
                                     gsl::span<const MeshInstance::Param> params)
{
    const auto& set_variable = [&](const char* name, IDeviceObject* object) {
        auto& srb = *material.shader_resource_binding;
        if (auto* var = srb.GetVariableByName(SHADER_TYPE_VERTEX, name)) {
            var->Set(object);
        }
        if (auto* var = srb.GetVariableByName(SHADER_TYPE_PIXEL, name)) {
            var->Set(object);
        }
    };

    MapHelper<std::uint8_t> map_helper(m_context, material.param_buffer, MAP_WRITE,
                                       MAP_FLAG_DISCARD);

    for (const auto& param : material.params) {
        const auto* it = std::find_if(params.begin(), params.end(),
                                      [&](const auto& p) { return p.name == param.name; });

        // Use the value from the provided params if it exists, otherwise the material's default
        const auto& value = (it != params.end()) ? it->value : param.default_value;

        // NOLINTNEXTLINE - pointer arithmetic
        auto* param_data = static_cast<std::uint8_t*>(map_helper) + param.buffer_offset;

        std::visit(
            Overloaded{[&](const TextureId& texture_id) {
                           if (texture_id < m_textures.size()) {
                               set_variable(param.name.c_str(), m_textures[texture_id].shader_view);
                           }
                       },
                       [&](const auto& value) {
                           // NOLINTNEXTLINE - reinterpret_cast
                           *reinterpret_cast<std::decay_t<decltype(value)>*>(param_data) = value;
                       }},
            value);
    }

    set_variable("Material", material.param_buffer);
}

void Renderer::render_meshes(gsl::span<const MeshInstance> meshes, const Camera& camera)
{
    for (const auto& mesh : meshes) {
        if (mesh.material_id >= m_materials.size() || mesh.mesh_id >= m_meshes.size()) {
            throw ArgumentError();
        }
    }

    // Set the view-specific constants
    {
        MapHelper<ViewConstantBuffer> constants(m_context, m_constants_view, MAP_WRITE,
                                                MAP_FLAG_DISCARD);
        constants->view_proj = camera.matrices().view_proj;
    }

    for (const auto& mesh_info : meshes) {
        auto& material = m_materials[mesh_info.material_id];
        auto& mesh     = m_meshes[mesh_info.mesh_id];

        m_context->SetPipelineState(material.pipeline);

        apply_material_params(material, mesh_info.material_params);

        m_context->CommitShaderResources(material.shader_resource_binding,
                                         RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        IBuffer* vertex_buffer = mesh.vertex_buffer.RawPtr();
        m_context->SetVertexBuffers(0, 1, &vertex_buffer, nullptr,
                                    RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                    SET_VERTEX_BUFFERS_FLAG_RESET);
        m_context->SetIndexBuffer(mesh.index_buffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // Set instance-specific constants
        {
            MapHelper<InstanceConstantBuffer> constants(m_context, m_constants_instance, MAP_WRITE,
                                                        MAP_FLAG_DISCARD);
            constants->world     = mesh_info.transform;
            constants->world_inv = inverse(mesh_info.transform);
        }

        static_assert(sizeof(Mesh::Index) == sizeof(std::uint16_t));
        DrawIndexedAttribs draw_attribs;
        draw_attribs.NumIndices = mesh.index_count;
        draw_attribs.IndexType  = VT_UINT16;
#ifndef NDEBUG
        draw_attribs.Flags = DRAW_FLAG_VERIFY_ALL;
#endif
        m_context->DrawIndexed(draw_attribs);
    }
}

} // namespace khepri::renderer::diligent
