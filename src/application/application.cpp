#if defined(_MSC_VER)
#include <Windows.h>
#endif

#include <khepri/application/application.hpp>
#include <khepri/log/log.hpp>

#include <array>

#if !defined(_MSC_VER)
#include <cstdlib>
#include <unistd.h>
#endif

#include <exception>

namespace khepri::application {
namespace {
constexpr log::Logger LOG("application");

std::string get_current_directory()
{
#ifdef _MSC_VER
    std::array<CHAR, MAX_PATH> curdir{};
    GetCurrentDirectoryA(MAX_PATH, curdir.data());
    return curdir.data();
#else
    char*       curdir_ = getcwd(nullptr, 0);
    std::string curdir  = curdir_;
    std::free(curdir_);
    return curdir;
#endif
}
} // namespace

Application::Application(std::string_view application_name) : m_application_name(application_name)
{}

Application::~Application() = default;

bool Application::run() noexcept
{
    // Set the terminate handler
    m_previous_terminate_handler = std::set_terminate([]() {
        LOG.critical("Unhandled exception");
        std::abort();
    });

#if defined(_MSC_VER) && !defined(NDEBUG)
    // In debug mode we turn on memory checking
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    const auto curdir = get_current_directory();
    LOG.info("Application starting up in \"{}\"", curdir);

    Window window(m_application_name);
    try {
        do_run(window, curdir);
    } catch (std::exception& e) {
        LOG.error("Caught unhandled exception: {}", e.what());
#ifdef _MSC_VER
        MessageBoxA(nullptr, e.what(), nullptr, MB_OK);
#endif
        return false;
    } catch (...) {
        LOG.error("Caught unhandled unknown exception");
        return false;
    }
    LOG.info("Application shutting down");

    std::set_terminate(m_previous_terminate_handler);
    return true;
}

} // namespace khepri::application
