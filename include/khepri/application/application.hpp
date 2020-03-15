#pragma once

#include "console_logger.hpp"
#include "window.hpp"

#include <filesystem>
#include <string>

namespace khepri::application {

/**
 * \brief Base class for an application framework.
 *
 * This utility class aims to simplify the creation of an application context.
 * When started with #run(), it creates a console for logging, setts up debug settings, error
 * handling, creates the window, and so on.
 * After setting up the context, it calls #do_run() where the user of this class can implement
 * the main application logic.
 */
class Application
{
public:
    /**
     * Constructs an application context
     * \param[in] application_name name of the application, used to show in the window it creates
     */
    explicit Application(std::string_view application_name);
    virtual ~Application();

    /**
     * \brief Sets up and runs the application
     *
     * After setting up the application, it calls #do_run().
     *
     * \returns true if the application ran successfully, false if an error or exception occurred.
     */
    bool run() noexcept;

    Application(const Application&)  = delete;
    Application(const Application&&) = delete;
    Application& operator=(const Application&) = delete;
    Application& operator=(const Application&&) = delete;

protected:
    /**
     * \brief Called when the application has been set up.
     * \param[in] window the window created by the application.
     * \param[in] working_path the working directory when the application was started.
     */
    virtual void do_run(Window& window, const std::filesystem::path& working_path) = 0;

private:
    static void terminate_handler();

#ifndef NDEBUG
    // In debug mode we create a console to log
    ConsoleLogger m_console;
#endif

    std::string            m_application_name;
    std::terminate_handler m_previous_terminate_handler{};
};

} // namespace khepri::application
