#pragma once

#include <khepri/renderer/vulkan/renderer.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

namespace khepri::application {

/**
 * \brief A user-visible window
 *
 * A window is the primary means of interaction by the user with the application.
 * It can create surfaces that the renderer can render to and receives and handles
 * input events.
 */
class Window final : public renderer::vulkan::Renderer::SurfaceProvider
{
public:
    /**
     * Constructs the window
     * \param[in] title the title of the window, as visible in the title bar.
     */
    explicit Window(const std::string& title);
    Window(const Window&) = delete;
    Window(Window&&)      = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;
    ~Window() override;

    /// \see SurfaceProvider::required_extensions
    [[nodiscard]] std::vector<std::string> required_extensions() const override;

    /// \see SurfaceProvider::create_surface
    [[nodiscard]] VkResult create_surface(VkInstance                   instance,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR*                surface) override;

    /// \see SurfaceProvider::get_render_size
    [[nodiscard]] Size get_render_size() const override;

    /**
     * \brief Returns true if the window should close.
     *
     * This will return true if the user, system or other event indicated that the window
     * should be closed. The application is responsible for checking this flag regularly
     * and act accordingly.
     *
     * \note If this is the only window in the application, it may want to shut down.
     */
    [[nodiscard]] bool should_close() const;

    /**
     * \brief observer and handle new events on the process's event queue.
     *
     * Every process has a single event queue that all user input events are posted to.
     * This method handles those events. The application should ensure that this method is
     * called regularly to guarantee responsiveness.
     */
    static void poll_events();

private:
    GLFWwindow* m_window;
};

} // namespace khepri::application