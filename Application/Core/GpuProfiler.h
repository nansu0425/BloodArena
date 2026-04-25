#pragma once

#include "Core/Profiler.h"

#ifndef BA_PROFILER_DISABLED

#include <tracy/TracyD3D11.hpp>

namespace BA
{

class GpuProfiler
{
public:
    void Initialize();
    void Shutdown();

    TracyD3D11Ctx GetGpuContext() const;

private:
    TracyD3D11Ctx m_gpuContext = nullptr;
};

extern std::unique_ptr<GpuProfiler> g_gpuProfiler;

} // namespace BA

#define BA_PROFILE_GPU_SCOPE(name)      TracyD3D11Zone(::BA::g_gpuProfiler->GetGpuContext(), name)
#define BA_PROFILE_GPU_COLLECT()        TracyD3D11Collect(::BA::g_gpuProfiler->GetGpuContext())

#else // BA_PROFILER_DISABLED

namespace BA
{

class GpuProfiler
{
public:
    void Initialize() {}
    void Shutdown() {}
};

extern std::unique_ptr<GpuProfiler> g_gpuProfiler;

} // namespace BA

#define BA_PROFILE_GPU_SCOPE(name)      ((void)0)
#define BA_PROFILE_GPU_COLLECT()        ((void)0)

#endif // BA_PROFILER_DISABLED
