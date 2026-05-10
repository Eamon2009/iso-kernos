#pragma once

#include <string>

struct CpuStats
{
      double usagePercent;
};

struct RamStats
{
      double usagePercent;
      long long totalMB;
      long long usedMB;
};

struct GpuStats
{
      double enginePercent;
      double vramUsedMB;
      double vramTotalMB;
      std::string name;
};

class HardwareMonitor
{
public:
      HardwareMonitor();
      ~HardwareMonitor();

      CpuStats getCpuStats();

      RamStats getRamStats();

      GpuStats getGpuStats();

private:
#ifdef _WIN32

      long long prevIdle = 0;
      long long prevKernel = 0;
      long long prevUser = 0;
      bool firstCpuCall = true;

      void *dxgiFactory = nullptr;
#endif

#ifdef __linux__

      long long prevIdle = 0;
      long long prevTotal = 0;
      bool firstCpuCall = true;
#endif
};