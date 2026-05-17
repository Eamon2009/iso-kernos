#include "core/Config.hpp"

#include "utils/FileReader.hpp"
#include "utils/StringUtils.hpp"

#include <cstdlib>
#include <sstream>

namespace iso_kernos {
namespace {

bool parseInt(const std::string& value, int& out)
{
    char* end = nullptr;
    const long parsed = std::strtol(value.c_str(), &end, 10);
    if (end == value.c_str() || *end != '\0') {
        return false;
    }
    out = static_cast<int>(parsed);
    return true;
}

bool parseDouble(const std::string& value, double& out)
{
    char* end = nullptr;
    const double parsed = std::strtod(value.c_str(), &end);
    if (end == value.c_str() || *end != '\0') {
        return false;
    }
    out = parsed;
    return true;
}

bool parseMode(const std::string& value, DisplayMode& mode)
{
    if (value == "dashboard") {
        mode = DisplayMode::Dashboard;
    } else if (value == "compact") {
        mode = DisplayMode::Compact;
    } else if (value == "detailed") {
        mode = DisplayMode::Detailed;
    } else if (value == "json") {
        mode = DisplayMode::Json;
    } else {
        return false;
    }
    return true;
}

} // namespace

Config loadConfigFile(const std::string& path, Config config)
{
    const std::string content = readTextFile(path);
    std::istringstream input(content);
    std::string line;
    while (std::getline(input, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';' || line[0] == '[') {
            continue;
        }
        const auto separator = line.find('=');
        if (separator == std::string::npos) {
            continue;
        }
        const std::string key = trim(line.substr(0, separator));
        const std::string value = trim(line.substr(separator + 1));
        if (key == "refresh_rate_ms") {
            parseInt(value, config.refreshRateMs);
        } else if (key == "color_enabled") {
            config.colorEnabled = (value == "true" || value == "1");
        } else if (key == "cpu_threshold") {
            parseDouble(value, config.cpuThreshold);
        } else if (key == "memory_threshold") {
            parseDouble(value, config.memoryThreshold);
        }
    }
    return config;
}

bool applyCommandLine(int argc, char** argv, Config& config, std::string& error)
{
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        auto requireValue = [&](std::string& value) -> bool {
            if (i + 1 >= argc) {
                error = "Missing value for " + arg;
                return false;
            }
            value = argv[++i];
            return true;
        };

        if (arg == "-h" || arg == "--help") {
            error = "help";
            return false;
        }
        if (arg == "-v" || arg == "--version") {
            error = "version";
            return false;
        }
        if (arg == "--no-color") {
            config.colorEnabled = false;
            continue;
        }
        if (arg == "--test-mode") {
            config.testMode = true;
            continue;
        }
        std::string value;
        if (arg == "-r" || arg == "--refresh") {
            if (!requireValue(value) || !parseInt(value, config.refreshRateMs)) {
                error = "Invalid refresh rate";
                return false;
            }
        } else if (arg == "-m" || arg == "--mode") {
            if (!requireValue(value) || !parseMode(value, config.mode)) {
                error = "Invalid display mode";
                return false;
            }
        } else if (arg == "-c" || arg == "--config") {
            if (!requireValue(value)) {
                return false;
            }
            config = loadConfigFile(value, config);
        } else if (arg == "-l" || arg == "--log") {
            if (!requireValue(config.logFile)) {
                return false;
            }
        } else if (arg == "--duration") {
            if (!requireValue(value) || !parseInt(value, config.durationSeconds)) {
                error = "Invalid duration";
                return false;
            }
        } else if (arg == "--threshold-cpu") {
            if (!requireValue(value) || !parseDouble(value, config.cpuThreshold)) {
                error = "Invalid CPU threshold";
                return false;
            }
        } else if (arg == "--threshold-mem") {
            if (!requireValue(value) || !parseDouble(value, config.memoryThreshold)) {
                error = "Invalid memory threshold";
                return false;
            }
        } else if (arg == "--compact") {
            config.mode = DisplayMode::Compact;
        } else if (arg == "--json") {
            config.mode = DisplayMode::Json;
        } else {
            error = "Unknown option: " + arg;
            return false;
        }
    }
    return true;
}

std::string usageText()
{
    return "Usage: iso-kernos [OPTIONS]\n"
           "  -h, --help              Show this help message\n"
           "  -v, --version           Show version information\n"
           "  -r, --refresh <ms>      Refresh rate in milliseconds\n"
           "  -m, --mode <mode>       dashboard|compact|detailed|json\n"
           "  -c, --config <file>     Read an INI-style config file\n"
           "  -l, --log <file>        Append JSON metrics to a log file\n"
           "      --duration <sec>    Stop after a fixed duration\n"
           "      --test-mode         Collect once and exit successfully\n"
           "      --no-color          Disable ANSI color output\n";
}

} // namespace iso_kernos
