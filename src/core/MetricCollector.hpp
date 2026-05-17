#pragma once

#include "core/IMonitor.hpp"

namespace iso_kernos {

class MetricCollector {
public:
    explicit MetricCollector(IMonitor& monitor);

    SystemMetrics collect();

private:
    IMonitor& monitor_;
};

} // namespace iso_kernos
