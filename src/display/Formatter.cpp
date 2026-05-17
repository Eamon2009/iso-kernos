#include "display/Formatter.hpp"

#include <iomanip>
#include <sstream>

namespace iso_kernos {
namespace {

std::string formatNumber(double value, const std::string& suffix, int precision)
{
    if (value < 0.0) {
        return "N/A";
    }
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << value << suffix;
    return out.str();
}

std::string jsonNumber(double value)
{
    if (value < 0.0) {
        return "null";
    }
    std::ostringstream out;
    out << std::fixed << std::setprecision(2) << value;
    return out.str();
}

std::string jsonNumber(long long value)
{
    if (value < 0) {
        return "null";
    }
    return std::to_string(value);
}

} // namespace

std::string formatPercent(double value, int precision)
{
    return formatNumber(value, "%", precision);
}

std::string formatMiB(double value, int precision)
{
    return formatNumber(value, " MiB", precision);
}

std::string formatBytesPerSecond(double value)
{
    if (value < 0.0) {
        return "N/A";
    }
    static const char* units[] = {" B/s", " KiB/s", " MiB/s", " GiB/s"};
    int unit = 0;
    while (value >= 1024.0 && unit < 3) {
        value /= 1024.0;
        ++unit;
    }
    return formatNumber(value, units[unit], unit == 0 ? 0 : 1);
}

std::string toJson(const SystemMetrics& metrics)
{
    std::ostringstream out;
    out << "{"
        << "\"cpu\":{\"usage_percent\":" << jsonNumber(metrics.cpu.usagePercent)
        << ",\"load1\":" << jsonNumber(metrics.cpu.load1)
        << ",\"load5\":" << jsonNumber(metrics.cpu.load5)
        << ",\"load15\":" << jsonNumber(metrics.cpu.load15) << "},"
        << "\"memory\":{\"usage_percent\":" << jsonNumber(metrics.memory.usagePercent)
        << ",\"total_mib\":" << jsonNumber(metrics.memory.totalMiB)
        << ",\"used_mib\":" << jsonNumber(metrics.memory.usedMiB)
        << ",\"free_mib\":" << jsonNumber(metrics.memory.freeMiB) << "},"
        << "\"gpu\":{\"name\":\"" << metrics.gpu.name
        << "\",\"engine_percent\":" << jsonNumber(metrics.gpu.enginePercent)
        << ",\"vram_used_mib\":" << jsonNumber(metrics.gpu.vramUsedMiB)
        << ",\"vram_total_mib\":" << jsonNumber(metrics.gpu.vramTotalMiB) << "},"
        << "\"disk\":{\"read_bps\":" << jsonNumber(metrics.disk.readBytesPerSecond)
        << ",\"write_bps\":" << jsonNumber(metrics.disk.writeBytesPerSecond) << "},"
        << "\"network\":{\"rx_bps\":" << jsonNumber(metrics.network.receiveBytesPerSecond)
        << ",\"tx_bps\":" << jsonNumber(metrics.network.transmitBytesPerSecond) << "},"
        << "\"battery\":{\"percent\":" << jsonNumber(metrics.battery.percent)
        << ",\"status\":\"" << metrics.battery.status << "\"}"
        << "}";
    return out.str();
}

} // namespace iso_kernos
