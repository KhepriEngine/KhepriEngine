#include <khepri/io/exceptions.hpp>
#include <khepri/io/file.hpp>
#include <khepri/renderer/exceptions.hpp>
#include <khepri/renderer/io/ksf.hpp>
#include <khepri/renderer/shader.hpp>
#include <khepri/version.hpp>

#include <cxxopts.hpp>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

namespace {
constexpr auto PROGRAM_NAME = "glsl2ksf";

void check_required(const cxxopts::ParseResult& result, const std::vector<std::string>& required)
{
    for (const auto& r : required) {
        if (result.count(r) == 0) {
            throw std::runtime_error("missing option '" + r + "'");
        }
    }
}

std::string read_file(const std::filesystem::path& path)
{
    khepri::io::File shader_file(path, khepri::io::open_mode::read);

    const auto size = shader_file.seek(0, khepri::io::seek_origin::end);
    shader_file.seek(0, khepri::io::seek_origin::begin);
    std::vector<char> buffer(size);
    if (shader_file.read(buffer.data(), buffer.size()) != buffer.size()) {
        throw khepri::io::Error("unable to read stream");
    }
    return {buffer.data(), buffer.size()};
}
} // namespace

int main(int argc, char* argv[])
{
    try {
        using namespace std::literals;

        cxxopts::Options options(PROGRAM_NAME,
                                 "Compiles a GLSL shader file to a Khepri shader file (.ksf)");
        options.positional_help("INPUT OUTPUT");

        options.add_options()("h,help", "display this help and exit")(
            "i,input", "Input file", cxxopts::value<std::filesystem::path>())(
            "o,output", "Output file",
            cxxopts::value<std::filesystem::path>())("V,version", "display the version and exit");

        options.parse_positional({"input", "output"});
        auto result = options.parse(argc, argv);
        if (result.count("help") != 0) {
            std::cout << options.help({"", "Group"}) << "\n";
            return 0;
        }

        if (result.count("version") != 0) {
            std::cout << PROGRAM_NAME << " version " << to_string(khepri::version()) << '\n';
            return 0;
        }

        check_required(result, {"input", "output"});
        const auto input_path  = result["input"].as<std::filesystem::path>();
        const auto output_path = result["output"].as<std::filesystem::path>();

        // Read the file
        const auto shader_source = read_file(input_path);

        // Compile to a shader
        const auto shader = khepri::renderer::Shader::compile(shader_source, input_path.string());

        // Write out the compiled shader
        khepri::io::File output(output_path, khepri::io::open_mode::read_write);
        khepri::renderer::io::write_ksf(shader, output);
    } catch (const khepri::renderer::ShaderCompileError& e) {
        std::cerr << "error: " << e.what() << ":\n" << e.error_message() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}