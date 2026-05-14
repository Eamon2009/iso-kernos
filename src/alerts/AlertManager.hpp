#pragma once

#include "core/Config.hpp"
#include "core/MetricTypes.hpp"

#include <string>
#include <vector>

namespace iso_kernos {

struct Alert {
    std::string metric;
    double value = -1.0;
    double threshold = -1.0;
};

class AlertManager {
public:
    explicit AlertManager(Config config);

    std::vector<Alert> evaluate(const SystemMetrics& metrics) const;

private:
    Config config_;
};

} // namespace iso_kernos
