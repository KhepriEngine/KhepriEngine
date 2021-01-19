#pragma once

#include <khepri/math/size.hpp>

#include <GLFW/glfw3.h>

#include <functional>
#include <string>
#include <vector>

namespace khepri::application {

/**
 * \brief A user-visible window
 *
 * A window is the primary means of interaction by the user with the application.
 * It can provide native window handles for the renderer and receive and handle
 * input events.
 */
class Window final
{
public:
    /// Callback for "window size changed" events
    using SizeListener = std::function<void()>;

    /**
     * Constructs the window
     * \param[in] title the title of the window, as visible in the title bar.
     */
    explicit Window(const std::string& title);
    Window(const Window&) = delete;
    Window(Window&&)      = delete;
    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;
    ~Window();

    /**
     * Returns the native handle of this window.
     */
    [[nodiscard]] void* native_handle() const;

    /**
     * Returns the size of the render area.
     */
    [[nodiscard]] Size render_size() const;

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
     * Adds a listener for "window size changed" events.
     * If invoked, call #render_size() to obtain the new render size.
     */
    void add_size_listener(const SizeListener& listener);

    /**
     * \brief observer and handle new events on the process's event queue.
     *
     * Every process has a single event queue that all user input events are posted to.
     * This method handles those events. The application should ensure that this method is
     * called regularly to guarantee responsiveness.
     */
    static void poll_events();

private:
    static void framebuffer_size_changed(GLFWwindow* windoww, int width, int height);

    GLFWwindow*               m_window;
    std::vector<SizeListener> m_size_listeners;
};

} // namespace khepri::application