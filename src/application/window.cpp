#include <khepri/application/window.hpp>
#include <khepri/log/log.hpp>

#include <gsl/gsl-lite.hpp>

namespace khepri::application {
namespace {
constexpr log::Logger LOG("window");

constexpr auto WINDOW_WIDTH  = 640;
constexpr auto WINDOW_HEIGHT = 480;
} // namespace

Window::Window(const std::string& title)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title.c_str(), nullptr, nullptr);
    LOG.info("Created window: {}", (void*)m_window);
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

std::vector<std::string> Window::required_extensions() const
{
    if (glfwVulkanSupported() == GLFW_FALSE) {
        return {};
    }
    std::uint32_t count      = 0;
    const char**  extensions = glfwGetRequiredInstanceExtensions(&count);

    std::vector<std::string> result;
    for (const auto& extension : gsl::span<const char*>(extensions, count)) {
        result.emplace_back(extension);
    }
    return result;
}

Size Window::get_render_size() const
{
    int width  = 0;
    int height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    return {static_cast<unsigned long>(width), static_cast<unsigned long>(height)};
}

VkResult Window::create_surface(VkInstance instance, const VkAllocationCallbacks* allocator,
                                VkSurfaceKHR* surface)
{
    return glfwCreateWindowSurface(instance, m_window, allocator, surface);
}

bool Window::should_close() const
{
    return glfwWindowShouldClose(m_window) == GLFW_TRUE;
}

void Window::poll_events()
{
    glfwPollEvents();
}

} // namespace khepri::application
