#include <khepri/application/window.hpp>
#include <khepri/log/log.hpp>

#ifdef _MSC_VER
#define GLFW_EXPOSE_NATIVE_WIN32
#else
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>
#include <gsl/gsl-lite.hpp>

namespace khepri::application {
namespace {
constexpr log::Logger LOG("window");

constexpr auto WINDOW_WIDTH  = 1024;
constexpr auto WINDOW_HEIGHT = 768;

Window* get_window(GLFWwindow* glfw_window)
{
    auto* data = glfwGetWindowUserPointer(glfw_window);
    return reinterpret_cast<Window*>(data); // NOLINT
}
} // namespace

Window::Window(const std::string& title)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_changed);
    glfwSetCursorPosCallback(m_window, cursor_position_callback);
    glfwSetMouseButtonCallback(m_window, mouse_button_callback);
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
#ifdef _MSC_VER
    return glfwGetWin32Window(m_window);
#else
    return glfwGetCocoaWindow(m_window);
#endif
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

void Window::add_cursor_position_listener(const CursorPositionListener& listener)
{
    m_cursor_position_listeners.push_back(listener);
}

void Window::add_mouse_button_listener(const MouseButtonListener& listener)
{
    m_mouse_button_listeners.push_back(listener);
}

void Window::poll_events()
{
    glfwPollEvents();
}

void Window::framebuffer_size_changed(GLFWwindow* w, int /*width*/, int /*height*/)
{
    auto* window = get_window(w);
    if (window != nullptr) {
        for (const auto& listener : window->m_size_listeners) {
            listener();
        }
    }
}

void Window::cursor_position_callback(GLFWwindow* w, double xpos, double ypos)
{
    auto* window = get_window(w);
    if (window != nullptr) {
        window->m_cursor_pos = {static_cast<long>(xpos), static_cast<long>(ypos)};
        for (const auto& listener : window->m_cursor_position_listeners) {
            listener(window->m_cursor_pos);
        }
    }
}

void Window::mouse_button_callback(GLFWwindow* w, int button, int action, int /*mods*/)
{
    auto* window = get_window(w);
    if (window != nullptr) {
        MouseButton mb{};
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            mb = MouseButton::left;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            mb = MouseButton::right;
            break;
        default:
            // We don't care about this button
            return;
        }

        const auto mba =
            (action == GLFW_PRESS) ? MouseButtonAction::pressed : MouseButtonAction::released;

        for (const auto& listener : window->m_mouse_button_listeners) {
            listener(window->m_cursor_pos, mb, mba);
        }
    }
}

} // namespace khepri::application