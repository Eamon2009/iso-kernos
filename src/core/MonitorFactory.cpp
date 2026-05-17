#include "core/MonitorFactory.hpp"

#if defined(__linux__)
#include "platform/linux/LinuxMonitor.hpp"
#elif defined(_WIN32)
#include "platform/windows/WindowsMonitor.hpp"
#elif defined(__APPLE__)
#include "platform/macos/MacOSMonitor.hpp"
#endif

namespace iso_kernos {

std::unique_ptr<IMonitor> MonitorFactory::create()
{
#if defined(__linux__)
    return std::make_unique<LinuxMonitor>();
#elif defined(_WIN32)
    return std::make_unique<WindowsMonitor>();
#elif defined(__APPLE__)
    return std::make_unique<MacOSMonitor>();
#else
    return nullptr;
#endif
}

} // namespace iso_kernos
