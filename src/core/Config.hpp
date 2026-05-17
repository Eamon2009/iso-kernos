#pragma once

#include <string>

namespace iso_kernos {

enum class DisplayMode {
    Dashboard,
    Compact,
    Detailed,
    Json
};

struct Config {
    int refreshRateMs = 1000;
    int durationSeconds = -1;
    bool colorEnabled = true;
    bool testMode = false;
    DisplayMode mode = DisplayMode::Dashboard;
    std::string logFile;
    double cpuThreshold = 90.0;
    double memoryThreshold = 85.0;
};

Config loadConfigFile(const std::string& path, Config defaults = {});
bool applyCommandLine(int argc, char** argv, Config& config, std::string& error);
std::string usageText();

} // namespace iso_kernos
