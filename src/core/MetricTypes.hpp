#pragma once

#include <string>

namespace iso_kernos {

struct CPUMetrics {
    double usagePercent = -1.0;
    double load1 = -1.0;
    double load5 = -1.0;
    double load15 = -1.0;
};

struct MemoryMetrics {
    double usagePercent = -1.0;
    long long totalMiB = -1;
    long long usedMiB = -1;
    long long freeMiB = -1;
};

struct GPUMetrics {
    double enginePercent = -1.0;
    double vramUsedMiB = -1.0;
    double vramTotalMiB = -1.0;
    std::string name = "Unknown";
};

struct DiskMetrics {
    double readBytesPerSecond = -1.0;
    double writeBytesPerSecond = -1.0;
};

struct NetworkMetrics {
    double receiveBytesPerSecond = -1.0;
    double transmitBytesPerSecond = -1.0;
};

struct BatteryMetrics {
    double percent = -1.0;
    std::string status = "Unknown";
};

struct SystemMetrics {
    CPUMetrics cpu;
    MemoryMetrics memory;
    GPUMetrics gpu;
    DiskMetrics disk;
    NetworkMetrics network;
    BatteryMetrics battery;
};

} // namespace iso_kernos
