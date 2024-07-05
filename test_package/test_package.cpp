#include <khepri/application/exceptions.hpp>
#include <khepri/application/console_logger.hpp>
#include <khepri/log/log.hpp>
#include <khepri/math/vector2.hpp>
#include <khepri/version.hpp>

int main()
{
    khepri::application::ConsoleLogger logger;
    khepri::log::Logger log("TestLogger");
    khepri::Vector2f vector{1,2};
    khepri::application::ExceptionHandler("main").invoke([&] {
        log.info("Khepri version: {}", khepri::to_string(khepri::version()));
    });
    return 0;
}
