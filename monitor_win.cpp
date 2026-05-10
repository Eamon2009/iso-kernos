#include "Monitor.hpp"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <dxgi.h>

#include <string>
#include <sstream>

static constexpr int POLL_MS = 500;

typedef LONG NTSTATUS;

typedef enum _D3DKMT_QUERYSTATISTICS_TYPE
{
      D3DKMT_QUERYSTATISTICS_ADAPTER = 0,
      D3DKMT_QUERYSTATISTICS_NODE = 6
} D3DKMT_QUERYSTATISTICS_TYPE;

typedef struct _D3DKMT_QUERYSTATISTICS_NODE_INFORMATION
{
      struct
      {
            LARGE_INTEGER RunningTime;
            ULONGLONG ContextSwitch;
            BYTE _pad[168];
      } GlobalInformation;
      BYTE _tail[64];
} D3DKMT_QUERYSTATISTICS_NODE_INFORMATION;

typedef struct _D3DKMT_QUERYSTATISTICS_ADAPTER_INFORMATION
{
      ULONG NodeCount;
      BYTE _pad[572];
} D3DKMT_QUERYSTATISTICS_ADAPTER_INFORMATION;

typedef union _D3DKMT_QUERYSTATISTICS_RESULT
{
      D3DKMT_QUERYSTATISTICS_ADAPTER_INFORMATION AdapterInformation;
      D3DKMT_QUERYSTATISTICS_NODE_INFORMATION NodeInformation;
} D3DKMT_QUERYSTATISTICS_RESULT;

typedef struct _D3DKMT_QUERYSTATISTICS_QUERY_NODE
{
      ULONG NodeId;
} D3DKMT_QUERYSTATISTICS_QUERY_NODE;

typedef struct _D3DKMT_QUERYSTATISTICS
{
      D3DKMT_QUERYSTATISTICS_TYPE Type;
      LUID AdapterLuid;
      D3DKMT_QUERYSTATISTICS_QUERY_NODE QueryNode;
      D3DKMT_QUERYSTATISTICS_RESULT QueryResult;
} D3DKMT_QUERYSTATISTICS;

typedef struct _DXGI_QUERY_VIDEO_MEMORY_INFO
{
      UINT64 Budget;
      UINT64 CurrentUsage;
      UINT64 AvailableForReservation;
      UINT64 CurrentReservation;
} DXGI_QUERY_VIDEO_MEMORY_INFO;

typedef enum _DXGI_MEMORY_SEGMENT_GROUP
{
      DXGI_MEMORY_SEGMENT_GROUP_LOCAL = 0,
      DXGI_MEMORY_SEGMENT_GROUP_NON_LOCAL = 1
} DXGI_MEMORY_SEGMENT_GROUP;

static const GUID IID_IDXGIAdapter3_local = {
    0x645967A4, 0x1392, 0x4310, {0xA7, 0x98, 0x80, 0x53, 0xCE, 0x3E, 0x93, 0xFD}};

struct IDXGIAdapter3Vtbl;
struct IDXGIAdapter3
{
      IDXGIAdapter3Vtbl *lpVtbl;
};

typedef HRESULT(STDMETHODCALLTYPE *PFN_QueryVideoMemoryInfo)(
    IDXGIAdapter3 *, UINT, DXGI_MEMORY_SEGMENT_GROUP,
    DXGI_QUERY_VIDEO_MEMORY_INFO *);

static long long filetimeToLL(const FILETIME &ft)
{
      ULARGE_INTEGER ul;
      ul.LowPart = ft.dwLowDateTime;
      ul.HighPart = ft.dwHighDateTime;
      return static_cast<long long>(ul.QuadPart);
}

static double queryD3dkmtEngineUtil(LUID adapterLuid)
{
      typedef NTSTATUS(APIENTRY * PFN)(const D3DKMT_QUERYSTATISTICS *);
      HMODULE gdi = ::GetModuleHandleW(L"gdi32.dll");
      if (!gdi)
            return -1.0;
      auto fn = reinterpret_cast<PFN>(::GetProcAddress(gdi, "D3DKMTQueryStatistics"));
      if (!fn)
            return -1.0;

      D3DKMT_QUERYSTATISTICS qs{};
      qs.Type = D3DKMT_QUERYSTATISTICS_ADAPTER;
      qs.AdapterLuid = adapterLuid;
      if (fn(&qs) != 0)
            return -1.0;

      ULONG nodeCount = qs.QueryResult.AdapterInformation.NodeCount;
      if (nodeCount == 0 || nodeCount > 64)
            return -1.0;

      double total = 0.0;
      int valid = 0;
      for (ULONG n = 0; n < nodeCount; ++n)
      {
            D3DKMT_QUERYSTATISTICS qn{};
            qn.Type = D3DKMT_QUERYSTATISTICS_NODE;
            qn.AdapterLuid = adapterLuid;
            qn.QueryNode.NodeId = n;
            if (fn(&qn) != 0)
                  continue;
            ULONGLONG running = qn.QueryResult.NodeInformation.GlobalInformation.RunningTime.QuadPart;
            ULONGLONG ctx = qn.QueryResult.NodeInformation.GlobalInformation.ContextSwitch;
            ULONGLONG sum = running + ctx;
            if (sum > 0)
            {
                  total += 100.0 * running / sum;
                  ++valid;
            }
      }
      return valid > 0 ? total / valid : -1.0;
}

static double queryVramUsed(IDXGIAdapter1 *adapter)
{
      void *raw = nullptr;
      if (FAILED(adapter->QueryInterface(IID_IDXGIAdapter3_local, &raw)))
            return -1.0;
      IDXGIAdapter3 *a3 = reinterpret_cast<IDXGIAdapter3 *>(raw);
      void **vtbl = reinterpret_cast<void **>(a3->lpVtbl);
      auto qvmi = reinterpret_cast<PFN_QueryVideoMemoryInfo>(vtbl[24]);
      DXGI_QUERY_VIDEO_MEMORY_INFO vmi{};
      double result = -1.0;
      if (SUCCEEDED(qvmi(a3, 0, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, &vmi)))
            result = static_cast<double>(vmi.CurrentUsage) / (1024.0 * 1024.0);
      reinterpret_cast<IUnknown *>(raw)->Release();
      return result;
}

HardwareMonitor::HardwareMonitor()
{
      typedef HRESULT(WINAPI * PFN_CreateDXGIFactory1)(REFIID, void **);
      HMODULE dxgiMod = ::LoadLibraryW(L"dxgi.dll");
      if (!dxgiMod)
            return;
      auto createFn = reinterpret_cast<PFN_CreateDXGIFactory1>(
          ::GetProcAddress(dxgiMod, "CreateDXGIFactory1"));
      if (!createFn)
            return;
      IDXGIFactory1 *factory = nullptr;
      if (SUCCEEDED(createFn(__uuidof(IDXGIFactory1),
                             reinterpret_cast<void **>(&factory))))
            dxgiFactory = factory;
}

HardwareMonitor::~HardwareMonitor()
{
      if (dxgiFactory)
      {
            reinterpret_cast<IDXGIFactory1 *>(dxgiFactory)->Release();
            dxgiFactory = nullptr;
      }
}

CpuStats HardwareMonitor::getCpuStats()
{
      CpuStats result{-1.0};
      auto snap = [&](long long &i, long long &k, long long &u) -> bool
      {
            FILETIME fi, fk, fu;
            if (!::GetSystemTimes(&fi, &fk, &fu))
                  return false;
            i = filetimeToLL(fi);
            k = filetimeToLL(fk);
            u = filetimeToLL(fu);
            return true;
      };
      long long i1 = 0, k1 = 0, u1 = 0;
      if (!snap(i1, k1, u1))
            return result;
      if (firstCpuCall)
      {
            prevIdle = i1;
            prevKernel = k1;
            prevUser = u1;
            firstCpuCall = false;
      }
      ::Sleep(POLL_MS);
      long long i2 = 0, k2 = 0, u2 = 0;
      if (!snap(i2, k2, u2))
            return result;
      long long dI = i2 - prevIdle, dK = k2 - prevKernel, dU = u2 - prevUser;
      prevIdle = i2;
      prevKernel = k2;
      prevUser = u2;
      long long dBusy = (dK - dI) + dU, dTotal = dK + dU;
      if (dTotal <= 0)
            return result;
      result.usagePercent = 100.0 * static_cast<double>(dBusy) / dTotal;
      return result;
}

RamStats HardwareMonitor::getRamStats()
{
      RamStats result{-1.0, -1, -1};
      MEMORYSTATUSEX ms{};
      ms.dwLength = sizeof(ms);
      if (!::GlobalMemoryStatusEx(&ms))
            return result;
      result.usagePercent = static_cast<double>(ms.dwMemoryLoad);
      result.totalMB = static_cast<long long>(ms.ullTotalPhys) / (1024 * 1024);
      result.usedMB = result.totalMB - static_cast<long long>(ms.ullAvailPhys) / (1024 * 1024);
      return result;
}

GpuStats HardwareMonitor::getGpuStats()
{
      GpuStats result{-1.0, -1.0, -1.0, "Unknown"};
      if (!dxgiFactory)
            return result;
      IDXGIFactory1 *factory = reinterpret_cast<IDXGIFactory1 *>(dxgiFactory);
      IDXGIAdapter1 *adapter = nullptr;
      if (FAILED(factory->EnumAdapters1(0, &adapter)))
            return result;
      DXGI_ADAPTER_DESC1 desc{};
      if (SUCCEEDED(adapter->GetDesc1(&desc)))
      {
            char narrow[128] = {};
            ::WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1,
                                  narrow, sizeof(narrow) - 1, nullptr, nullptr);
            result.name = narrow;
            result.vramTotalMB = static_cast<double>(desc.DedicatedVideoMemory) / (1024.0 * 1024.0);
            result.vramUsedMB = queryVramUsed(adapter);
            result.enginePercent = queryD3dkmtEngineUtil(desc.AdapterLuid);
      }
      adapter->Release();
      return result;
}