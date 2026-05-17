#pragma once

#include "core/IMonitor.hpp"

#include <memory>

namespace iso_kernos {

class MonitorFactory {
public:
    static std::unique_ptr<IMonitor> create();
};

} // namespace iso_kernos
