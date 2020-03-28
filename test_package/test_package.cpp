#include <khepri/application/application.hpp>
#include <khepri/version.hpp>

#include <iostream>

class TestApplication : khepri::application::Application
{
public:
    TestApplication() : Application("Test package"){};

protected:
    void do_run(khepri::application::Window&, const std::filesystem::path&) override {}
};

int main()
{
    TestApplication application;
    std::cout << "Khepri version: " << khepri::to_string(khepri::version()) << std::endl;
    return 0;
}
