# iso-kernos

<img width="774" height="474" alt="image" src="https://github.com/user-attachments/assets/db0384c7-52bd-4388-a0bc-9ac4840ae123" />


---

## Platform support

| Platform | CPU | RAM | GPU |
|---|---|---|---|
| Linux (AMD / Intel GPU) | `/proc/stat` delta | `/proc/meminfo` | `/sys/class/drm/cardN/device/gpu_busy_percent` + sysfs VRAM nodes |
| Linux (NVIDIA GPU) | same | same | `nvidia-smi` fallback via `popen` — no NVML required |
| Windows (MinGW / MSVC) | `GetSystemTimes` delta | `GlobalMemoryStatusEx` | DXGI + D3DKMT (same source as Task Manager) |

Unavailable metrics return `-1` and display as `N/A`. The program never crashes on missing hardware.

---

## How it works

**CPU** — Two snapshots of idle and total CPU ticks are taken 500 ms apart. Usage is computed as:

```
Usage% = 100 × (1 − ΔIdle / ΔTotal)
```

On Linux the ticks come from `/proc/stat`. On Windows `GetSystemTimes` returns idle, kernel (which includes idle), and user times as `FILETIME` values; idle is subtracted from kernel to isolate busy kernel time.

**RAM** — A single read with no sleep. On Linux, `MemTotal` and `MemAvailable` are parsed from `/proc/meminfo`; used = total − available. On Windows `GlobalMemoryStatusEx` fills a `MEMORYSTATUSEX` struct that contains `dwMemoryLoad`, `ullTotalPhys`, and `ullAvailPhys` directly.

**GPU** — Linux checks `/sys/class/drm/card0` through `card3` for `gpu_busy_percent` and VRAM byte counts exposed by open-source AMD and Intel drivers. If those sysfs nodes are absent, it falls back to `popen("nvidia-smi ...")` and parses the CSV. On Windows, `CreateDXGIFactory1` enumerates adapters; VRAM usage is queried via the `IDXGIAdapter3::QueryVideoMemoryInfo` vtable slot (called through a raw COM pointer to stay MinGW-compatible), and `D3DKMTQueryStatistics` (loaded dynamically from `gdi32.dll`) gives per-node engine utilisation averaged across all GPU nodes.

**Polling** — `getCpuStats()` owns the 500 ms sleep required for the CPU delta. RAM and GPU reads are instantaneous, so the overall loop cadence is naturally ≥ 500 ms with negligible CPU overhead.

---

## Project structure 

```
iso-kernos/
├── Monitor.hpp          # Struct definitions (CpuStats, RamStats, GpuStats)
│                        # and HardwareMonitor class declaration.
│                        # Included on both platforms — no OS headers here.
│
├── Monitor_win.cpp      # Windows-only implementation.
│                        # Uses GetSystemTimes, GlobalMemoryStatusEx,
│                        # DXGI + D3DKMT. All DXGI 1.4 types self-declared
│                        # so MinGW and MSVC both compile without extra SDKs.
│
└── main.cpp             # Entry point. Zero OS-specific code.
                         # Creates HardwareMonitor, calls the three stat
                         # methods, pretty-prints in nvidia-smi table style.
```

---

## Build

**Linux**
```bash
g++ -std=c++17 -O2 main.cpp Monitor_linux.cpp -o iso-kernos
./iso-kernos
```

**Windows — MinGW (g++)**
```powershell
g++ -std=c++17 -O2 main.cpp Monitor_win.cpp -o iso-kernos.exe -ldxgi
.\iso-kernos.exe
```

**Windows — MSVC**
```powershell
cl /std:c++17 /O2 main.cpp Monitor_win.cpp /link dxgi.lib kernel32.lib
.\iso-kernos.exe
```
 
