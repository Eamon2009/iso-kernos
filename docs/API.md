# API

The primary extension point is `iso_kernos::IMonitor`.

```cpp
class IMonitor {
public:
    virtual ~IMonitor() = default;
    virtual CPUMetrics getCPU() = 0;
    virtual MemoryMetrics getMemory() = 0;
    virtual GPUMetrics getGPU() = 0;
    virtual DiskMetrics getDisk();
    virtual NetworkMetrics getNetwork();
    virtual BatteryMetrics getBattery();
};
```

Unavailable metrics use `-1` numeric values and `"Unknown"` text values.
