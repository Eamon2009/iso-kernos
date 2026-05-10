#include "Monitor.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <thread>
#include <cstdio>

static constexpr int POLL_MS = 500;

static long long readMemInfoKey(const char *key)
{
      std::ifstream f("/proc/meminfo");
      if (f.fail())
            return -1;
      std::string line, needle(key);
      while (std::getline(f, line))
      {
            if (line.rfind(needle, 0) == 0)
            {
                  std::istringstream iss(line.substr(needle.size()));
                  long long val = -1;
                  iss >> val;
                  return val;
            }
      }
      return -1;
}

static bool parseProcStat(long long &outIdle, long long &outTotal)
{
      std::ifstream f("/proc/stat");
      if (f.fail())
            return false;
      std::string tag;
      f >> tag;
      if (tag != "cpu")
            return false;
      long long user, nice, system, idle, iowait, irq, softirq, steal;
      f >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
      if (f.fail())
            return false;
      outIdle = idle + iowait;
      outTotal = user + nice + system + idle + iowait + irq + softirq + steal;
      return true;
}

static double tryDrmBusy()
{
      for (int i = 0; i < 4; ++i)
      {
            char path[128];
            std::snprintf(path, sizeof(path), "/sys/class/drm/card%d/device/gpu_busy_percent", i);
            std::ifstream f(path);
            if (f.fail())
                  continue;
            double val = -1.0;
            f >> val;
            if (!f.fail())
                  return val;
      }
      return -1.0;
}

static void tryDrmVram(double &usedMB, double &totalMB)
{
      usedMB = totalMB = -1.0;
      for (int i = 0; i < 4; ++i)
      {
            char pu[128], pt[128];
            std::snprintf(pu, sizeof(pu), "/sys/class/drm/card%d/device/mem_info_vram_used", i);
            std::snprintf(pt, sizeof(pt), "/sys/class/drm/card%d/device/mem_info_vram_total", i);
            std::ifstream fu(pu), ft(pt);
            if (fu.fail() || ft.fail())
                  continue;
            long long u = -1, t = -1;
            fu >> u;
            ft >> t;
            if (!fu.fail() && !ft.fail() && u >= 0 && t > 0)
            {
                  usedMB = u / (1024.0 * 1024.0);
                  totalMB = t / (1024.0 * 1024.0);
                  return;
            }
      }
}

static void tryNvidiaSmi(double &eng, double &um, double &tm, std::string &name)
{
      eng = um = tm = -1.0;
      name = "Unknown";
      FILE *pipe = ::popen(
          "nvidia-smi --query-gpu=utilization.gpu,memory.used,memory.total,name"
          " --format=csv,noheader,nounits 2>/dev/null",
          "r");
      if (!pipe)
            return;
      char buf[512] = {};
      if (std::fgets(buf, sizeof(buf), pipe))
      {
            std::istringstream ss(buf);
            std::string tok;
            int field = 0;
            while (std::getline(ss, tok, ','))
            {
                  auto s = tok.find_first_not_of(" \t\r\n");
                  if (s != std::string::npos)
                        tok = tok.substr(s);
                  switch (field)
                  {
                  case 0:
                        try
                        {
                              eng = std::stod(tok);
                        }
                        catch (...)
                        {
                        }
                        break;
                  case 1:
                        try
                        {
                              um = std::stod(tok);
                        }
                        catch (...)
                        {
                        }
                        break;
                  case 2:
                        try
                        {
                              tm = std::stod(tok);
                        }
                        catch (...)
                        {
                        }
                        break;
                  case 3:
                        name = tok;
                        break;
                  }
                  ++field;
            }
      }
      ::pclose(pipe);
}

HardwareMonitor::HardwareMonitor() = default;
HardwareMonitor::~HardwareMonitor() = default;

CpuStats HardwareMonitor::getCpuStats()
{
      CpuStats result{-1.0};
      long long idle1 = 0, total1 = 0;
      if (!parseProcStat(idle1, total1))
            return result;
      if (firstCpuCall)
      {
            prevIdle = idle1;
            prevTotal = total1;
            firstCpuCall = false;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(POLL_MS));
      long long idle2 = 0, total2 = 0;
      if (!parseProcStat(idle2, total2))
            return result;
      long long dIdle = idle2 - prevIdle;
      long long dTotal = total2 - prevTotal;
      prevIdle = idle2;
      prevTotal = total2;
      if (dTotal <= 0)
            return result;
      result.usagePercent = 100.0 * (1.0 - static_cast<double>(dIdle) / dTotal);
      return result;
}

RamStats HardwareMonitor::getRamStats()
{
      RamStats result{-1.0, -1, -1};
      long long totalKB = readMemInfoKey("MemTotal:");
      long long availKB = readMemInfoKey("MemAvailable:");
      if (totalKB <= 0 || availKB < 0)
            return result;
      long long usedKB = totalKB - availKB;
      result.totalMB = totalKB / 1024;
      result.usedMB = usedKB / 1024;
      result.usagePercent = 100.0 * static_cast<double>(usedKB) / totalKB;
      return result;
}

GpuStats HardwareMonitor::getGpuStats()
{
      GpuStats result{-1.0, -1.0, -1.0, "Unknown"};
      double eng = tryDrmBusy();
      double um = -1.0, tm = -1.0;
      tryDrmVram(um, tm);
      if (eng >= 0.0)
      {
            result.enginePercent = eng;
            result.vramUsedMB = um;
            result.vramTotalMB = tm;
            for (int i = 0; i < 4; ++i)
            {
                  char path[128];
                  std::snprintf(path, sizeof(path), "/sys/class/drm/card%d/device/uevent", i);
                  std::ifstream f(path);
                  if (f.fail())
                        continue;
                  std::string line;
                  while (std::getline(f, line))
                  {
                        if (line.rfind("DRIVER=", 0) == 0)
                        {
                              result.name = line.substr(7);
                              break;
                        }
                  }
                  if (result.name != "Unknown")
                        break;
            }
            return result;
      }
      tryNvidiaSmi(result.enginePercent, result.vramUsedMB, result.vramTotalMB, result.name);
      return result;
}