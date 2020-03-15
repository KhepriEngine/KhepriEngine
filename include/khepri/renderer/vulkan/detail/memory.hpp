#pragma once

#include <vulkan/vulkan.h>

#include <utility>

namespace khepri::renderer::vulkan::detail {

template <typename THandle>
class UniqueVkInstance final
{
public:
    using destroy_func = void (*)(THandle, const VkAllocationCallbacks*);

    UniqueVkInstance() noexcept = default;

    UniqueVkInstance(const THandle& handle, const VkAllocationCallbacks* allocator,
                     destroy_func destroy_func) noexcept
        : m_handle(handle), m_allocator(allocator), m_destroy_func(destroy_func)
    {}

    ~UniqueVkInstance() noexcept
    {
        if (m_destroy_func != nullptr) {
            (*m_destroy_func)(m_handle, m_allocator);
        }
    }

    UniqueVkInstance(const UniqueVkInstance&) = delete;
    UniqueVkInstance& operator=(const UniqueVkInstance&) = delete;

    UniqueVkInstance(UniqueVkInstance&& other) noexcept
        : m_handle(VK_NULL_HANDLE)
        , m_allocator(other.m_allocator)
        , m_destroy_func(other.m_destroy_func)
    {
        std::swap(m_handle, other.m_handle);
    }

    UniqueVkInstance& operator=(UniqueVkInstance&& other) noexcept
    {
        m_allocator    = other.m_allocator;
        m_destroy_func = other.m_destroy_func;
        std::swap(m_handle, other.m_handle);
        return *this;
    }

    operator THandle() const noexcept
    {
        return m_handle;
    }

private:
    THandle                      m_handle{VK_NULL_HANDLE};
    const VkAllocationCallbacks* m_allocator{nullptr};
    destroy_func                 m_destroy_func{nullptr};
};

template <typename THandle, typename TOwner>
class UniqueVkHandle final
{
public:
    using destroy_func = void (*)(TOwner, THandle, const VkAllocationCallbacks*);

    UniqueVkHandle() noexcept = default;

    UniqueVkHandle(const THandle& handle, const TOwner& owner,
                   const VkAllocationCallbacks* allocator, destroy_func destroy_func) noexcept
        : m_handle(handle), m_owner(owner), m_allocator(allocator), m_destroy_func(destroy_func)
    {}

    ~UniqueVkHandle() noexcept
    {
        destroy();
    }

    UniqueVkHandle(const UniqueVkHandle&) = delete;
    UniqueVkHandle& operator=(const UniqueVkHandle&) = delete;

    UniqueVkHandle(UniqueVkHandle&& other) noexcept
        : m_handle(VK_NULL_HANDLE)
        , m_owner(other.m_owner)
        , m_allocator(other.m_allocator)
        , m_destroy_func(other.m_destroy_func)
    {
        std::swap(m_handle, other.m_handle);
    }

    UniqueVkHandle& operator=(UniqueVkHandle&& other) noexcept
    {
        destroy();
        m_owner        = other.m_owner;
        m_allocator    = other.m_allocator;
        m_destroy_func = other.m_destroy_func;
        m_handle       = other.m_handle;
        other.m_handle = VK_NULL_HANDLE;
        return *this;
    }

    operator THandle() const noexcept
    {
        return m_handle;
    }

private:
    void destroy() noexcept
    {
        if (m_handle != VK_NULL_HANDLE && m_destroy_func != nullptr) {
            (*m_destroy_func)(m_owner, m_handle, m_allocator);
        }
    }

    THandle                      m_handle{VK_NULL_HANDLE};
    TOwner                       m_owner{VK_NULL_HANDLE};
    const VkAllocationCallbacks* m_allocator{nullptr};
    destroy_func                 m_destroy_func{nullptr};
};

template <typename THandle, typename TOwner>
auto make_unique_vk_handle(
    const THandle& handle, const TOwner& owner, const VkAllocationCallbacks* allocator,
    typename UniqueVkHandle<THandle, TOwner>::destroy_func destroy_func) noexcept
{
    return UniqueVkHandle(handle, owner, allocator, destroy_func);
}

template <typename THandle, typename TOwner, typename TOther>
auto make_unique_vk_handle(
    const THandle& handle, const UniqueVkHandle<TOwner, TOther>& owner,
    const VkAllocationCallbacks*                           allocator,
    typename UniqueVkHandle<THandle, TOwner>::destroy_func destroy_func) noexcept
{
    return UniqueVkHandle(handle, static_cast<TOwner>(owner), allocator, destroy_func);
}

template <typename THandle, typename TOwner>
auto make_unique_vk_handle(
    const THandle& handle, const UniqueVkInstance<TOwner>& owner,
    const VkAllocationCallbacks*                           allocator,
    typename UniqueVkHandle<THandle, TOwner>::destroy_func destroy_func) noexcept
{
    return UniqueVkHandle(handle, static_cast<TOwner>(owner), allocator, destroy_func);
}

} // namespace khepri::renderer::vulkan::detail
