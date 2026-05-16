#pragma once

#include "core/IMonitor.hpp"

namespace iso_kernos {

class WindowsMonitor final : public IMonitor {
public:
    WindowsMonitor();
    ~WindowsMonitor() override;

    CPUMetrics getCPU() override;
    MemoryMetrics getMemory() override;
    GPUMetrics getGPU() override;
    BatteryMetrics getBattery() override;

private:
    long long previousIdle_ = 0;
    long long previousKernel_ = 0;
    long long previousUser_ = 0;
    bool firstCpuCall_ = true;
    void* dxgiFactory_ = nullptr;
};

} // namespace iso_kernos
