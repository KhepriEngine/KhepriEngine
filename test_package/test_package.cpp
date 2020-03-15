#include <khepri/application/application.hpp>

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
    return 0;
}
