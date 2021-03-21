#if defined(_MSC_VER)
#include <Windows.h>
#ifndef NDEBUG
#include <dbghelp.h>
#include <debugapi.h>
#endif
#endif

#include <khepri/application/application.hpp>
#include <khepri/log/log.hpp>

#include <fmt/ranges.h>
#include <gsl/gsl-lite.hpp>

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

#ifdef _MSC_VER
void print_native_exception(HANDLE hProcess, EXCEPTION_POINTERS* exc_ptrs)
{
    if (exc_ptrs == nullptr || exc_ptrs->ExceptionRecord == nullptr) {
        LOG.error("Caught unknown native exception");
    } else {
        const auto* exceptr = exc_ptrs->ExceptionRecord;
        auto*       context = exc_ptrs->ContextRecord;

        gsl::span<const ULONG_PTR> params(&exceptr->ExceptionInformation[0],
                                          exceptr->NumberParameters);

        LOG.error("Caught native exception: code={:#x}, flags={}, address={:p}, params={{{:#x}}}",
                  exceptr->ExceptionCode, exceptr->ExceptionFlags, exceptr->ExceptionAddress,
                  fmt::join(params, ", "));
#ifndef NDEBUG
        DWORD machine_type = 0;
#if defined(_M_IX86)
        machine_type = IMAGE_FILE_MACHINE_I386;
#elif defined(_M_IA64)
        machine_type = IMAGE_FILE_MACHINE_IA64;
#elif defined(_M_AMD64) || defined(_M_X64)
        machine_type = IMAGE_FILE_MACHINE_AMD64;
#endif
        LOG.error("Stack trace:");

        STACKFRAME frame{};
        while (StackWalk64(machine_type, hProcess, GetCurrentThread(), &frame, context, nullptr,
                           SymFunctionTableAccess64, SymGetModuleBase64, nullptr) != FALSE) {
            std::array<char, sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)> buffer{};
            auto* pSymbol         = reinterpret_cast<SYMBOL_INFO*>(buffer.data()); // NOLINT
            pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            pSymbol->MaxNameLen   = MAX_SYM_NAME;

            DWORD64         displacement = 0;
            IMAGEHLP_LINE64 line;
            line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

            if (SymFromAddr(hProcess, frame.AddrPC.Offset, &displacement, pSymbol) != FALSE) {
                auto symbol_name = std::string_view(&pSymbol->Name[0], pSymbol->NameLen);
                if (SymGetLineFromAddr64(hProcess, frame.AddrPC.Offset, nullptr, &line) != FALSE) {
                    LOG.error(" - {:#018x} {} + {} ({}:{})", frame.AddrPC.Offset, symbol_name,
                              displacement, line.FileName, line.LineNumber);
                } else {
                    LOG.error(" - {:#018x} {} + {}", frame.AddrPC.Offset, symbol_name,
                              displacement);
                }
            } else {
                LOG.error(" - {:#018x}", frame.AddrPC.Offset);
            }
        }
#endif
    }
}

template <typename TCallable>
void handle_native_exceptions(TCallable callable)
{
#ifndef NDEBUG
    // Don't catch exceptions in debug builds if a debugger is attached -- the debugger will catch
    // them instead
    if (IsDebuggerPresent()) {
        callable();
        return;
    }
#endif

    HANDLE hProcess = GetCurrentProcess();
#ifndef NDEBUG
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_DEFERRED_LOADS);
    SymInitialize(hProcess, nullptr, TRUE);
#endif
    __try {
        callable();
        // NOLINTNEXTLINE -- GetExceptionInformation is a macro and confuses clang-tidy
    } __except (print_native_exception(hProcess, GetExceptionInformation()),
                EXCEPTION_EXECUTE_HANDLER) {
    }
#ifndef NDEBUG
    SymCleanup(hProcess);
#endif
}
#else
template <typename TCallable>
void handle_native_exceptions(TCallable callable)
{
    callable();
}
#endif

template <typename TCallable>
void handle_language_exceptions(TCallable callable)
{
    try {
        callable();
    } catch (std::exception& e) {
        LOG.error("Caught unhandled exception: {}", e.what());
#ifdef _MSC_VER
        MessageBoxA(nullptr, e.what(), nullptr, MB_OK);
#endif
    } catch (...) {
        LOG.error("Caught unhandled unknown exception");
    }
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
    handle_native_exceptions([&] { handle_language_exceptions([&] { do_run(window, curdir); }); });
    LOG.info("Application shutting down");

    std::set_terminate(m_previous_terminate_handler);
    return true;
}

} // namespace khepri::application
