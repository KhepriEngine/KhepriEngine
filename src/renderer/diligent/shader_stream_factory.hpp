#pragma once

#include <khepri/renderer/renderer.hpp>

#include <ObjectBase.hpp>
#include <Shader.h>
#include <cassert>
#include <iterator>

namespace khepri::renderer::diligent {

class ShaderStreamFactory final
    : public Diligent::ObjectBase<Diligent::IShaderSourceInputStreamFactory>
{
    using IObject      = Diligent::IObject;
    using INTERFACE_ID = Diligent::INTERFACE_ID;

    class MemoryStream final : public Diligent::ObjectBase<Diligent::IFileStream>
    {
    public:
        MemoryStream(Diligent::IReferenceCounters* pRefCounters, std::vector<std::uint8_t> data)
            : Diligent::ObjectBase<Diligent::IFileStream>(pRefCounters), m_data(std::move(data))
        {}

        void DILIGENT_CALL_TYPE ReadBlob(Diligent::IDataBlob* data) override
        {
            assert(data != nullptr);
            assert(m_pos == 0);
            data->Resize(m_data.size());
            std::copy(m_data.begin(), m_data.end(), static_cast<uint8_t*>(data->GetDataPtr()));
        }

        bool DILIGENT_CALL_TYPE Read(void* data, size_t size) override
        {
            auto read = std::min(m_data.size() - m_pos, size);
            std::copy(&m_data[m_pos], &m_data[m_pos + read], static_cast<uint8_t*>(data));
            m_pos += read;
            return read == size;
        }

        bool DILIGENT_CALL_TYPE Write(const void* /*data*/, size_t /*size*/) override
        {
            // Not supported
            return false;
        }

        size_t DILIGENT_CALL_TYPE GetSize() override
        {
            return m_data.size();
        }

        bool DILIGENT_CALL_TYPE IsValid() override
        {
            return true;
        }

        IMPLEMENT_QUERY_INTERFACE_IN_PLACE(Diligent::IID_FileStream,
                                           Diligent::ObjectBase<Diligent::IFileStream>);

    private:
        std::vector<std::uint8_t> m_data;
        size_t                    m_pos = 0;
    };

public:
    explicit ShaderStreamFactory(Diligent::IReferenceCounters* pRefCounters,
                                 Renderer::FileLoader          loader)
        : Diligent::ObjectBase<Diligent::IShaderSourceInputStreamFactory>(pRefCounters)
        , m_loader(std::move(loader))
    {}

    void DILIGENT_CALL_TYPE CreateInputStream(const char*             name,
                                              Diligent::IFileStream** ppStream) final
    {
        CreateInputStream2(name, Diligent::CREATE_SHADER_SOURCE_INPUT_STREAM_FLAG_NONE, ppStream);
    }

    void DILIGENT_CALL_TYPE CreateInputStream2(
        const char*             Name, Diligent::CREATE_SHADER_SOURCE_INPUT_STREAM_FLAGS /*Flags*/,
        Diligent::IFileStream** ppStream) final
    {
        assert(ppStream != nullptr);
        auto data = m_loader(Name);
        *ppStream = (data) ? Diligent::MakeNewRCObj<MemoryStream>()(std::move(*data)) : nullptr;
    }

    IMPLEMENT_QUERY_INTERFACE_IN_PLACE(
        Diligent::IID_IShaderSourceInputStreamFactory,
        Diligent::ObjectBase<Diligent::IShaderSourceInputStreamFactory>);

private:
    Renderer::FileLoader m_loader;
};

} // namespace khepri::renderer::diligent
