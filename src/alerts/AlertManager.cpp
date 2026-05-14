#include "alerts/AlertManager.hpp"

namespace iso_kernos {

AlertManager::AlertManager(Config config) : config_(std::move(config)) {}

std::vector<Alert> AlertManager::evaluate(const SystemMetrics& metrics) const
{
    std::vector<Alert> alerts;
    if (metrics.cpu.usagePercent >= config_.cpuThreshold) {
        alerts.push_back({"cpu", metrics.cpu.usagePercent, config_.cpuThreshold});
    }
    if (metrics.memory.usagePercent >= config_.memoryThreshold) {
        alerts.push_back({"memory", metrics.memory.usagePercent, config_.memoryThreshold});
    }
    return alerts;
}

} // namespace iso_kernos
