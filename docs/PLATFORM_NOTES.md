# Platform Notes

## Linux

CPU metrics are read from `/proc/stat` and `/proc/loadavg`. Memory metrics are read from `/proc/meminfo`. GPU metrics prefer DRM sysfs nodes and fall back to `nvidia-smi` when available.

## Windows

CPU metrics use `GetSystemTimes`; memory uses `GlobalMemoryStatusEx`; GPU name and total dedicated VRAM come from DXGI.

## macOS

The macOS monitor is scaffolded so the cross-platform factory and build layout are ready for native `sysctl`, Mach, and IOKit implementations.
