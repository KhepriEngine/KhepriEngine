#pragma once

#include <khepri/math/point.hpp>
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
    /// Identifies a mouse button
    enum class MouseButton
    {
        /// The left mouse button
        left,
        /// The right mouse button
        right
    };

    /// Identifies a mouse button action
    enum class MouseButtonAction
    {
        /// The mouse button was pressed
        pressed,
        /// The mouse button was released
        released
    };

    /// Callback for "window size changed" events
    using SizeListener = std::function<void()>;

    /// Callback for "cursor position changed" events
    using CursorPositionListener = std::function<void(const khepri::Point& pos)>;

    /// Callback for "mouse button" events
    using MouseButtonListener =
        std::function<void(const khepri::Point& pos, MouseButton, MouseButtonAction)>;

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
     * Adds a listener for "cursor posiion changed" events.
     * The cursor's position relative to the window's render area are passed along.
     */
    void add_cursor_position_listener(const CursorPositionListener& listener);

    /**
     * Adds a listener for "mouse button" events.
     * The cursor's position relative to the window's render area, the mouse button, and the button
     * action are passed along.
     */
    void add_mouse_button_listener(const MouseButtonListener& listener);

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
    static void cursor_position_callback(GLFWwindow* windoww, double xpos, double ypos);
    static void mouse_button_callback(GLFWwindow* windoww, int button, int action, int mods);

    GLFWwindow*                         m_window;
    std::vector<SizeListener>           m_size_listeners;
    std::vector<CursorPositionListener> m_cursor_position_listeners;
    std::vector<MouseButtonListener>    m_mouse_button_listeners;

    khepri::Point m_cursor_pos{0, 0};
};

} // namespace khepri::application