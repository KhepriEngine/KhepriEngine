#include <khepri/application/window.hpp>
#include <khepri/log/log.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
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
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_changed);
    LOG.info("Created window: {}", (void*)m_window);
}

Window::~Window()
{
    glfwSetWindowUserPointer(m_window, nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void* Window::native_handle() const
{
    return glfwGetWin32Window(m_window);
}

Size Window::render_size() const
{
    int width  = 0;
    int height = 0;
    glfwGetFramebufferSize(m_window, &width, &height);
    return {static_cast<unsigned long>(width), static_cast<unsigned long>(height)};
}

bool Window::should_close() const
{
    return glfwWindowShouldClose(m_window) == GLFW_TRUE;
}

void Window::add_size_listener(const SizeListener& listener)
{
    m_size_listeners.push_back(listener);
}

void Window::poll_events()
{
    glfwPollEvents();
}

void Window::framebuffer_size_changed(GLFWwindow* w, int /*width*/, int /*height*/)
{
    void* data = glfwGetWindowUserPointer(w);
    if (data != nullptr) {
        auto* window = reinterpret_cast<Window*>(data); // NOLINT
        for (const auto& listener : window->m_size_listeners) {
            listener();
        }
    }
}

} // namespace khepri::application
