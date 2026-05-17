#include "core/MetricCollector.hpp"

namespace iso_kernos {

MetricCollector::MetricCollector(IMonitor& monitor) : monitor_(monitor) {}

SystemMetrics MetricCollector::collect()
{
    SystemMetrics metrics;
    metrics.cpu = monitor_.getCPU();
    metrics.memory = monitor_.getMemory();
    metrics.gpu = monitor_.getGPU();
    metrics.disk = monitor_.getDisk();
    metrics.network = monitor_.getNetwork();
    metrics.battery = monitor_.getBattery();
    return metrics;
}

} // namespace iso_kernos
