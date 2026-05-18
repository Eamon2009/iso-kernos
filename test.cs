using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace HardwareMonitoring
{
    public struct CpuStats
    {
        public double UsagePercent;
    }

    public struct RamStats
    {
        public double UsagePercent;
        public long TotalMB;
        public long UsedMB;
    }

    public struct GpuStats
    {
        public double EnginePercent;
        public double VramUsedMB;
        public double VramTotalMB;
        public string Name;
    }

    #region Windows API Structures and Enums

    [StructLayout(LayoutKind.Sequential)]
    struct FILETIME
    {
        public uint dwLowDateTime;
        public uint dwHighDateTime;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct MEMORYSTATUSEX
    {
        public uint dwLength;
        public uint dwMemoryLoad;
        public ulong ullTotalPhys;
        public ulong ullAvailPhys;
        public ulong ullTotalPageFile;
        public ulong ullAvailPageFile;
        public ulong ullTotalVirtual;
        public ulong ullAvailVirtual;
        public ulong ullAvailExtendedVirtual;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct LUID
    {
        public uint LowPart;
        public int HighPart;
    }

    enum D3DKMT_QUERYSTATISTICS_TYPE
    {
        D3DKMT_QUERYSTATISTICS_ADAPTER = 0,
        D3DKMT_QUERYSTATISTICS_NODE = 6
    }

    [StructLayout(LayoutKind.Sequential)]
    struct D3DKMT_QUERYSTATISTICS_NODE_INFORMATION
    {
        public D3DKMT_QUERYSTATISTICS_NODE_INFORMATION_GLOBAL GlobalInformation;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
        public byte[] Tail;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct D3DKMT_QUERYSTATISTICS_NODE_INFORMATION_GLOBAL
    {
        public long RunningTime;
        public ulong ContextSwitch;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 168)]
        public byte[] Pad;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct D3DKMT_QUERYSTATISTICS_ADAPTER_INFORMATION
    {
        public uint NodeCount;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 572)]
        public byte[] Pad;
    }

    [StructLayout(LayoutKind.Explicit)]
    struct D3DKMT_QUERYSTATISTICS_RESULT
    {
        [FieldOffset(0)]
        public D3DKMT_QUERYSTATISTICS_ADAPTER_INFORMATION AdapterInformation;
        [FieldOffset(0)]
        public D3DKMT_QUERYSTATISTICS_NODE_INFORMATION NodeInformation;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct D3DKMT_QUERYSTATISTICS_QUERY_NODE
    {
        public uint NodeId;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct D3DKMT_QUERYSTATISTICS
    {
        public D3DKMT_QUERYSTATISTICS_TYPE Type;
        public LUID AdapterLuid;
        public D3DKMT_QUERYSTATISTICS_QUERY_NODE QueryNode;
        public D3DKMT_QUERYSTATISTICS_RESULT QueryResult;
    }

    [StructLayout(LayoutKind.Sequential)]
    struct DXGI_QUERY_VIDEO_MEMORY_INFO
    {
        public ulong Budget;
        public ulong CurrentUsage;
        public ulong AvailableForReservation;
        public ulong CurrentReservation;
    }

    enum DXGI_MEMORY_SEGMENT_GROUP
    {
        DXGI_MEMORY_SEGMENT_GROUP_LOCAL = 0,
        DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL = 1
    }

    [StructLayout(LayoutKind.Sequential)]
    struct DXGI_ADAPTER_DESC1
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 128)]
        public string Description;
        public uint VendorId;
        public uint DeviceId;
        public uint SubSysId;
        public uint Revision;
        public IntPtr DedicatedVideoMemory;
        public IntPtr DedicatedSystemMemory;
        public IntPtr SharedSystemMemory;
        public LUID AdapterLuid;
        public uint Flags;
    }

    #endregion

    #region COM Interfaces

    [ComImport]
    [Guid("7b7166ec-21c7-44ae-b21a-c9ae321ae369")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    interface IDXGIFactory1
    {
        void EnumAdapters(uint Adapter, out IntPtr ppAdapter);
        void MakeWindowAssociation(IntPtr WindowHandle, uint Flags);
        void GetWindowAssociation(out IntPtr pWindowHandle);
        void CreateSwapChain(IntPtr pDevice, IntPtr pDesc, out IntPtr ppSwapChain);
        void CreateSoftwareAdapter(IntPtr Module, out IntPtr ppAdapter);
        void EnumAdapters1(uint Adapter, out IntPtr ppAdapter);
        [PreserveSig]
        int IsCurrent();
    }

    [ComImport]
    [Guid("29038f61-3839-4626-91fd-086879011a05")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    interface IDXGIAdapter1
    {
        void SetPrivateData(ref Guid Name, uint DataSize, IntPtr pData);
        void SetPrivateDataInterface(ref Guid Name, IntPtr pUnknown);
        void GetPrivateData(ref Guid Name, ref uint pDataSize, IntPtr pData);
        void GetParent(ref Guid riid, out IntPtr ppParent);
        void EnumOutputs(uint Output, out IntPtr ppOutput);
        [PreserveSig]
        int GetDesc1(out DXGI_ADAPTER_DESC1 pDesc);
    }

    [ComImport]
    [Guid("645967A4-1392-4310-A798-8053CE3E93FD")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    interface IDXGIAdapter3
    {
        // Inherits from IDXGIAdapter2, IDXGIAdapter1, IDXGIAdapter, IDXGIObject
        // We need to declare all parent methods first
        void SetPrivateData(ref Guid Name, uint DataSize, IntPtr pData);
        void SetPrivateDataInterface(ref Guid Name, IntPtr pUnknown);
        void GetPrivateData(ref Guid Name, ref uint pDataSize, IntPtr pData);
        void GetParent(ref Guid riid, out IntPtr ppParent);
        void EnumOutputs(uint Output, out IntPtr ppOutput);
        void GetDesc(IntPtr pDesc);
        void CheckInterfaceSupport(ref Guid InterfaceName, out long pUMDVersion);
        void GetDesc1(out DXGI_ADAPTER_DESC1 pDesc);
        void GetDesc2(IntPtr pDesc);
        void RegisterHardwareContentProtectionTeardownStatusEvent(IntPtr hEvent, out uint pdwCookie);
        void UnregisterHardwareContentProtectionTeardownStatus(uint dwCookie);
        void QueryVideoMemoryInfo(uint NodeIndex, DXGI_MEMORY_SEGMENT_GROUP MemorySegmentGroup, out DXGI_QUERY_VIDEO_MEMORY_INFO pVideoMemoryInfo);
        void SetVideoMemoryReservation(uint NodeIndex, DXGI_MEMORY_SEGMENT_GROUP MemorySegmentGroup, ulong Reservation);
        void RegisterVideoMemoryBudgetChangeNotificationEvent(IntPtr hEvent, out uint pdwCookie);
        void UnregisterVideoMemoryBudgetChangeNotification(uint dwCookie);
    }

    #endregion

    #region P/Invoke Declarations

    static class NativeMethods
    {
        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool GetSystemTimes(
            out FILETIME lpIdleTime,
            out FILETIME lpKernelTime,
            out FILETIME lpUserTime);

        [DllImport("kernel32.dll", SetLastError = true)]
        [return: MarshalAs(UnmanagedType.Bool)]
        public static extern bool GlobalMemoryStatusEx(ref MEMORYSTATUSEX lpBuffer);

        [DllImport("gdi32.dll", EntryPoint = "D3DKMTQueryStatistics")]
        public static extern int D3DKMTQueryStatistics(ref D3DKMT_QUERYSTATISTICS pData);

        [DllImport("dxgi.dll", EntryPoint = "CreateDXGIFactory1")]
        public static extern int CreateDXGIFactory1(ref Guid riid, out IntPtr ppFactory);
    }

    #endregion

    public class HardwareMonitor : IDisposable
    {
        private const int POLL_MS = 500;
        private static readonly Guid IID_IDXGIFactory1 = new Guid("7b7166ec-21c7-44ae-b21a-c9ae321ae369");

        private IntPtr dxgiFactory = IntPtr.Zero;
        private bool firstCpuCall = true;
        private long prevIdle, prevKernel, prevUser;

        public HardwareMonitor()
        {
            IntPtr factory;
            if (NativeMethods.CreateDXGIFactory1(ref IID_IDXGIFactory1, out factory) == 0)
            {
                dxgiFactory = factory;
            }
        }

        public void Dispose()
        {
            if (dxgiFactory != IntPtr.Zero)
            {
                Marshal.Release(dxgiFactory);
                dxgiFactory = IntPtr.Zero;
            }
        }

        private static long FileTimeToLong(FILETIME ft)
        {
            return ((long)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
        }

        private static double QueryD3dkmtEngineUtil(LUID adapterLuid)
        {
            try
            {
                D3DKMT_QUERYSTATISTICS qs = new D3DKMT_QUERYSTATISTICS
                {
                    Type = D3DKMT_QUERYSTATISTICS_TYPE.D3DKMT_QUERYSTATISTICS_ADAPTER,
                    AdapterLuid = adapterLuid
                };

                if (NativeMethods.D3DKMTQueryStatistics(ref qs) != 0)
                    return -1.0;

                uint nodeCount = qs.QueryResult.AdapterInformation.NodeCount;
                if (nodeCount == 0 || nodeCount > 64)
                    return -1.0;

                double total = 0.0;
                int valid = 0;

                for (uint n = 0; n < nodeCount; n++)
                {
                    D3DKMT_QUERYSTATISTICS qn = new D3DKMT_QUERYSTATISTICS
                    {
                        Type = D3DKMT_QUERYSTATISTICS_TYPE.D3DKMT_QUERYSTATISTICS_NODE,
                        AdapterLuid = adapterLuid,
                        QueryNode = new D3DKMT_QUERYSTATISTICS_QUERY_NODE { NodeId = n }
                    };

                    if (NativeMethods.D3DKMTQueryStatistics(ref qn) != 0)
                        continue;

                    long running = qn.QueryResult.NodeInformation.GlobalInformation.RunningTime;
                    ulong ctx = qn.QueryResult.NodeInformation.GlobalInformation.ContextSwitch;
                    long sum = running + (long)ctx;

                    if (sum > 0)
                    {
                        total += 100.0 * running / sum;
                        valid++;
                    }
                }

                return valid > 0 ? total / valid : -1.0;
            }
            catch
            {
                return -1.0;
            }
        }

        private static double QueryVramUsed(IntPtr adapterPtr)
        {
            try
            {
                IDXGIAdapter1 adapter = Marshal.GetObjectForIUnknown(adapterPtr) as IDXGIAdapter1;
                if (adapter == null)
                    return -1.0;

                // Try to get IDXGIAdapter3
                Guid iid = new Guid("645967A4-1392-4310-A798-8053CE3E93FD");
                IntPtr adapter3Ptr = Marshal.GetIUnknownForObject(adapter);
                
                object adapter3Obj = Marshal.GetObjectForIUnknown(adapter3Ptr);
                IDXGIAdapter3 adapter3 = adapter3Obj as IDXGIAdapter3;
                
                if (adapter3 == null)
                {
                    Marshal.Release(adapter3Ptr);
                    return -1.0;
                }

                DXGI_QUERY_VIDEO_MEMORY_INFO vmi;
                adapter3.QueryVideoMemoryInfo(0, DXGI_MEMORY_SEGMENT_GROUP.DXGI_MEMORY_SEGMENT_GROUP_LOCAL, out vmi);
                
                Marshal.Release(adapter3Ptr);
                
                return (double)vmi.CurrentUsage / (1024.0 * 1024.0);
            }
            catch
            {
                return -1.0;
            }
        }

        public CpuStats GetCpuStats()
        {
            CpuStats result = new CpuStats { UsagePercent = -1.0 };

            FILETIME idle1, kernel1, user1;
            if (!NativeMethods.GetSystemTimes(out idle1, out kernel1, out user1))
                return result;

            long i1 = FileTimeToLong(idle1);
            long k1 = FileTimeToLong(kernel1);
            long u1 = FileTimeToLong(user1);

            if (firstCpuCall)
            {
                prevIdle = i1;
                prevKernel = k1;
                prevUser = u1;
                firstCpuCall = false;
            }

            Thread.Sleep(POLL_MS);

            FILETIME idle2, kernel2, user2;
            if (!NativeMethods.GetSystemTimes(out idle2, out kernel2, out user2))
                return result;

            long i2 = FileTimeToLong(idle2);
            long k2 = FileTimeToLong(kernel2);
            long u2 = FileTimeToLong(user2);

            long dI = i2 - prevIdle;
            long dK = k2 - prevKernel;
            long dU = u2 - prevUser;

            prevIdle = i2;
            prevKernel = k2;
            prevUser = u2;

            long dBusy = (dK - dI) + dU;
            long dTotal = dK + dU;

            if (dTotal <= 0)
                return result;

            result.UsagePercent = 100.0 * dBusy / dTotal;
            return result;
        }

        public RamStats GetRamStats()
        {
            RamStats result = new RamStats
            {
                UsagePercent = -1.0,
                TotalMB = -1,
                UsedMB = -1
            };

            MEMORYSTATUSEX ms = new MEMORYSTATUSEX();
            ms.dwLength = (uint)Marshal.SizeOf(typeof(MEMORYSTATUSEX));

            if (!NativeMethods.GlobalMemoryStatusEx(ref ms))
                return result;

            result.UsagePercent = ms.dwMemoryLoad;
            result.TotalMB = (long)(ms.ullTotalPhys / (1024 * 1024));
            result.UsedMB = result.TotalMB - (long)(ms.ullAvailPhys / (1024 * 1024));

            return result;
        }

        public GpuStats GetGpuStats()
        {
            GpuStats result = new GpuStats
            {
                EnginePercent = -1.0,
                VramUsedMB = -1.0,
                VramTotalMB = -1.0,
                Name = "Unknown"
            };

            if (dxgiFactory == IntPtr.Zero)
                return result;

            try
            {
                IDXGIFactory1 factory = Marshal.GetObjectForIUnknown(dxgiFactory) as IDXGIFactory1;
                if (factory == null)
                    return result;

                IntPtr adapterPtr;
                if (factory.EnumAdapters1(0, out adapterPtr) != 0)
                    return result;

                IDXGIAdapter1 adapter = Marshal.GetObjectForIUnknown(adapterPtr) as IDXGIAdapter1;
                if (adapter != null)
                {
                    DXGI_ADAPTER_DESC1 desc;
                    if (adapter.GetDesc1(out desc) == 0)
                    {
                        result.Name = desc.Description;
                        result.VramTotalMB = (double)desc.DedicatedVideoMemory.ToInt64() / (1024.0 * 1024.0);
                        result.VramUsedMB = QueryVramUsed(adapterPtr);
                        result.EnginePercent = QueryD3dkmtEngineUtil(desc.AdapterLuid);
                    }
                }

                Marshal.Release(adapterPtr);
            }
            catch
            {
                // Return default values
            }

            return result;
        }
    }

    // Example usage
    class Program
    {
        static void Main(string[] args)
        {
            Console.WriteLine("Hardware Monitor - Press ESC to exit\n");

            using (var monitor = new HardwareMonitor())
            {
                while (true)
                {
                    if (Console.KeyAvailable && Console.ReadKey(true).Key == ConsoleKey.Escape)
                        break;

                    var cpu = monitor.GetCpuStats();
                    var ram = monitor.GetRamStats();
                    var gpu = monitor.GetGpuStats();

                    Console.Clear();
                    Console.WriteLine("=== Hardware Monitor ===\n");

                    Console.WriteLine($"CPU Usage: {cpu.UsagePercent:F1}%");
                    Console.WriteLine($"\nRAM Usage: {ram.UsagePercent:F1}% ({ram.UsedMB:N0} MB / {ram.TotalMB:N0} MB)");
                    Console.WriteLine($"\nGPU: {gpu.Name}");
                    Console.WriteLine($"GPU Engine: {gpu.EnginePercent:F1}%");
                    Console.WriteLine($"VRAM: {gpu.VramUsedMB:F0} MB / {gpu.VramTotalMB:F0} MB");
                    
                    Console.WriteLine("\nPress ESC to exit...");

                    Thread.Sleep(1000);
                }
            }
        }
    }
}
