#include "alerts/Logger.hpp"

#include "display/Formatter.hpp"

#include <fstream>

namespace iso_kernos {

void appendJsonLog(const std::string& path, const SystemMetrics& metrics)
{
    if (path.empty()) {
        return;
    }
    std::ofstream out(path, std::ios::app);
    if (out) {
        out << toJson(metrics) << '\n';
    }
}

} // namespace iso_kernos
