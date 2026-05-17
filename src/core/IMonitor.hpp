#pragma once

#include "core/MetricTypes.hpp"

namespace iso_kernos {

class IMonitor {
public:
    virtual ~IMonitor() = default;

    virtual CPUMetrics getCPU() = 0;
    virtual MemoryMetrics getMemory() = 0;
    virtual GPUMetrics getGPU() = 0;

    virtual DiskMetrics getDisk() { return {}; }
    virtual NetworkMetrics getNetwork() { return {}; }
    virtual BatteryMetrics getBattery() { return {}; }
};

} // namespace iso_kernos
