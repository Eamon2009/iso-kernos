#pragma once

#include "core/MetricTypes.hpp"

#include <string>

namespace iso_kernos {

void appendJsonLog(const std::string& path, const SystemMetrics& metrics);

} // namespace iso_kernos
