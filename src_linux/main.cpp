#include "Monitor.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <ctime>

static const int W = 76;

static std::string repeat(char c, int n) { return std::string(n, c); }

static std::string fmtDouble(double v, int prec = 1)
{
      if (v < 0.0)
            return "N/A";
      std::ostringstream s;
      s << std::fixed << std::setprecision(prec) << v << "%";
      return s.str();
}

static std::string fmtMiB(double v)
{
      if (v < 0.0)
            return "N/A";
      std::ostringstream s;
      s << std::fixed << std::setprecision(0) << v << " MiB";
      return s.str();
}

static std::string fmtLL(long long v, const char *unit)
{
      if (v < 0)
            return "N/A";
      return std::to_string(v) + unit;
}

static std::string currentDateTime()
{
      std::time_t t = std::time(nullptr);
      char buf[64];
      std::strftime(buf, sizeof(buf), "%a %b %d %Y  %H:%M:%S", std::localtime(&t));
      return buf;
}

static void topRow(const std::string &lbl, const std::string &val,
                   const std::string &lbl2, const std::string &val2)
{
      std::cout << "| "
                << std::left << std::setw(18) << lbl
                << std::setw(20) << val
                << std::setw(18) << lbl2
                << std::setw(16) << val2
                << "|\n";
}

static void singleRow(const std::string &lbl, const std::string &val)
{
      int pad = W - 4 - (int)lbl.size() - (int)val.size();
      if (pad < 1)
            pad = 1;
      std::cout << "| " << lbl << std::string(pad, ' ') << val << " |\n";
}

static void divider(char l, char fill, char r)
{
      std::cout << l << repeat(fill, W - 2) << r << '\n';
}

static void sectionHeader(const std::string &title)
{
      divider('+', '-', '+');
      int pad = W - 4 - (int)title.size();
      if (pad < 0)
            pad = 0;
      std::cout << "| " << title << std::string(pad, ' ') << " |\n";
      divider('+', '=', '+');
}

int main()
{
      HardwareMonitor monitor;

      for (;;)
      {
            CpuStats cpu = monitor.getCpuStats();
            RamStats ram = monitor.getRamStats();
            GpuStats gpu = monitor.getGpuStats();

            std::cout << "\033[2J\033[H";

            divider('+', '-', '+');
            {
                  std::string title = "Hardware Monitor v1.0";
                  std::string dt = currentDateTime();
                  int gap = W - 4 - (int)title.size() - (int)dt.size();
                  if (gap < 1)
                        gap = 1;
                  std::cout << "| " << title << std::string(gap, ' ') << dt << " |\n";
            }
            divider('+', '-', '+');
            {
                  std::string left = "Driver API : OS Native (no external libs)";
                  std::string right = "Poll: 500 ms";
                  int gap = W - 4 - (int)left.size() - (int)right.size();
                  if (gap < 1)
                        gap = 1;
                  std::cout << "| " << left << std::string(gap, ' ') << right << " |\n";
            }
            divider('+', '=', '+');

            sectionHeader("CPU");
            topRow("Utilisation", fmtDouble(cpu.usagePercent), "", "");
            divider('+', '-', '+');

            sectionHeader("Memory (RAM)");
            topRow("Utilisation", fmtDouble(ram.usagePercent),
                   "Total", fmtLL(ram.totalMB, " MiB"));
            topRow("Used", fmtLL(ram.usedMB, " MiB"),
                   "Free", fmtLL(ram.totalMB >= 0 && ram.usedMB >= 0 ? ram.totalMB - ram.usedMB : -1LL, " MiB"));
            divider('+', '-', '+');

            sectionHeader("GPU  |  " + gpu.name);
            topRow("Engine Util", fmtDouble(gpu.enginePercent),
                   "VRAM Used", fmtMiB(gpu.vramUsedMB));
            topRow("VRAM Total", fmtMiB(gpu.vramTotalMB),
                   "VRAM Free", fmtMiB(gpu.vramTotalMB >= 0 && gpu.vramUsedMB >= 0 ? gpu.vramTotalMB - gpu.vramUsedMB : -1.0));
            divider('+', '=', '+');

            singleRow("Press Ctrl-C to exit", "");
            divider('+', '-', '+');
            std::cout << '\n';
      }

      return 0;
}