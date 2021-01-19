#include "shader_stream_factory.hpp"

#include <khepri/exceptions.hpp>
#include <khepri/renderer/camera.hpp>
#include <khepri/renderer/diligent/renderer.hpp>

#include <EngineFactoryD3D11.h>
#include <MapHelper.hpp>

using namespace Diligent;

namespace khepri::renderer::diligent {
namespace {
SHADER_TYPE to_shader_type(ShaderType shader_type) noexcept
{
    switch (shader_type) {
    case ShaderType::vertex:
        return SHADER_TYPE_VERTEX;
    case ShaderType::pixel:
        return SHADER_TYPE_PIXEL;
    default:
        break;
    }
    assert(false);
    return SHADER_TYPE_VERTEX;
}

const char* get_entry_point(ShaderType shader_type) noexcept
{
    switch (shader_type) {
    case ShaderType::vertex:
        return "vs_main";
    case ShaderType::pixel:
        return "ps_main";
    default:
        break;
    }
    assert(false);
    return nullptr;
}
} // namespace

struct Renderer::RenderMesh
{
    Mesh::Index            index_count = 0;
    RefCntAutoPtr<IBuffer> vertex_buffer;
    RefCntAutoPtr<IBuffer> index_buffer;
};

struct Renderer::RenderPipeline
{
    Diligent::RefCntAutoPtr<Diligent::IPipelineState>         pipeline;
    Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> shader_resource_binding;
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

    // Create constants buffer for vertex shader
    BufferDesc desc;
    desc.Name           = "VS constants CB";
    desc.Size           = sizeof(Matrix);
    desc.Usage          = USAGE_DYNAMIC;
    desc.BindFlags      = BIND_UNIFORM_BUFFER;
    desc.CPUAccessFlags = CPU_ACCESS_WRITE;
    m_device->CreateBuffer(desc, nullptr, &m_constants_vs);
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

ShaderId Renderer::create_shader(const std::filesystem::path& path, ShaderType shader_type,
                                 const FileLoader& loader)
{
    RefCntAutoPtr<ShaderStreamFactory> factory(MakeNewRCObj<ShaderStreamFactory>()(loader));

    const std::string strpath = path.string();

    ShaderCreateInfo ci;
    ci.Desc.Name                  = strpath.c_str();
    ci.Desc.ShaderType            = to_shader_type(shader_type);
    ci.FilePath                   = strpath.c_str();
    ci.pShaderSourceStreamFactory = factory;
    ci.SourceLanguage             = SHADER_SOURCE_LANGUAGE_HLSL;
    ci.EntryPoint                 = get_entry_point(shader_type);

    RefCntAutoPtr<IShader> shader;
    m_device->CreateShader(ci, &shader);

    auto it = std::find(m_shaders.begin(), m_shaders.end(), nullptr);
    if (it == m_shaders.end()) {
        it = m_shaders.emplace(m_shaders.end());
    }
    *it = std::move(shader);
    return it - m_shaders.begin();
}

void Renderer::destroy_shader(ShaderId shader)
{
    assert(shader <= m_shaders.size());
    m_shaders[shader] = {};
}

PipelineId Renderer::create_render_pipeline(const PipelineDesc& desc)
{
    if ((desc.stage.vertex_shader >= m_shaders.size()) ||
        (desc.stage.pixel_shader >= m_shaders.size())) {
        throw ArgumentError();
    }

    GraphicsPipelineStateCreateInfo ci;
    ci.PSODesc.PipelineType                          = PIPELINE_TYPE_GRAPHICS;
    ci.GraphicsPipeline.NumRenderTargets             = 1;
    ci.GraphicsPipeline.RTVFormats[0]                = m_swapchain->GetDesc().ColorBufferFormat;
    ci.GraphicsPipeline.DSVFormat                    = m_swapchain->GetDesc().DepthBufferFormat;
    ci.GraphicsPipeline.DepthStencilDesc.DepthEnable = false;

    std::array<LayoutElement, 2> layout{
        LayoutElement{0, 0, 3, VT_FLOAT32, false, offsetof(Mesh::Vertex, position),
                      sizeof(Mesh::Vertex)},
        LayoutElement{1, 0, 3, VT_FLOAT32, false, offsetof(Mesh::Vertex, normal),
                      sizeof(Mesh::Vertex)}};
    ci.GraphicsPipeline.InputLayout.LayoutElements = layout.data();
    ci.GraphicsPipeline.InputLayout.NumElements    = static_cast<Uint32>(layout.size());

    ci.pVS = m_shaders[desc.stage.vertex_shader];
    ci.pPS = m_shaders[desc.stage.pixel_shader];

    RenderPipeline pipeline;
    m_device->CreateGraphicsPipelineState(ci, &pipeline.pipeline);

    pipeline.pipeline->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")
        ->Set(m_constants_vs);
    pipeline.pipeline->CreateShaderResourceBinding(&pipeline.shader_resource_binding, true);

    auto it = std::find_if(m_pipelines.begin(), m_pipelines.end(), [](const auto& p) {
        return p.pipeline == nullptr && p.shader_resource_binding == nullptr;
    });
    if (it == m_pipelines.end()) {
        it = m_pipelines.emplace(m_pipelines.end());
    }
    *it = std::move(pipeline);
    return it - m_pipelines.begin();
}

void Renderer::destroy_render_pipeline(PipelineId pipeline)
{
    assert(pipeline <= m_pipelines.size());
    m_pipelines[pipeline] = {};
}

RenderableMeshId Renderer::create_renderable_mesh(const Mesh& mesh)
{
    RenderMesh render_mesh;
    render_mesh.index_count = static_cast<Mesh::Index>(mesh.indices.size());

    {
        BufferData data{mesh.vertices.data(),
                        static_cast<Uint32>(mesh.vertices.size() * sizeof(Mesh::Vertex))};
        BufferDesc desc;
        desc.Size      = data.DataSize;
        desc.BindFlags = BIND_VERTEX_BUFFER;
        desc.Usage     = USAGE_IMMUTABLE;
        m_device->CreateBuffer(desc, &data, &render_mesh.vertex_buffer);
    }

    {
        BufferData data{mesh.indices.data(),
                        static_cast<Uint32>(mesh.indices.size() * sizeof(Mesh::Index))};
        BufferDesc desc;
        desc.Size      = data.DataSize;
        desc.BindFlags = BIND_INDEX_BUFFER;
        desc.Usage     = USAGE_IMMUTABLE;
        m_device->CreateBuffer(desc, &data, &render_mesh.index_buffer);
    }

    auto it = std::find_if(m_meshes.begin(), m_meshes.end(), [](const auto& m) {
        return m.index_count == 0 && m.vertex_buffer == nullptr && m.index_buffer == nullptr;
    });
    if (it == m_meshes.end()) {
        it = m_meshes.emplace(m_meshes.end());
    }
    *it = std::move(render_mesh);
    return it - m_meshes.begin();
}

void Renderer::destroy_renderable_mesh(RenderableMeshId mesh_id)
{
    assert(mesh_id <= m_meshes.size());
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

void Renderer::render_meshes(PipelineId pipeline_id, gsl::span<const RenderableMeshInstance> meshes,
                             const Camera& camera)
{
    assert(pipeline_id < m_pipelines.size());
    assert(m_pipelines[pipeline_id].pipeline != nullptr);
    assert(m_pipelines[pipeline_id].shader_resource_binding != nullptr);
    auto& pipeline = m_pipelines[pipeline_id];

    m_context->SetPipelineState(pipeline.pipeline);
    m_context->CommitShaderResources(pipeline.shader_resource_binding,
                                     RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    for (const auto& mesh_info : meshes) {
        assert(mesh_info.mesh_id < m_meshes.size());
        auto& mesh = m_meshes[mesh_info.mesh_id];

        IBuffer* vertex_buffer = mesh.vertex_buffer.RawPtr();
        m_context->SetVertexBuffers(0, 1, &vertex_buffer, nullptr,
                                    RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                                    SET_VERTEX_BUFFERS_FLAG_RESET);
        m_context->SetIndexBuffer(mesh.index_buffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        {
            MapHelper<Matrix> constants(m_context, m_constants_vs, MAP_WRITE, MAP_FLAG_DISCARD);
            *constants = mesh_info.transform * camera.matrices().view_proj;
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
