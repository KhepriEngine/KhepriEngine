#include <khepri/application/exceptions.hpp>
#include <khepri/version.hpp>

#include <iostream>

int main()
{
    khepri::application::ExceptionHandler("main").invoke([&] {
        std::cout << "Khepri version: " << khepri::to_string(khepri::version()) << std::endl;
    });
    return 0;
}
