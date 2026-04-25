#include "Core/PCH.h"
#include "Core/GpuProfiler.h"
#include "Graphics/GraphicsDevice.h"

namespace BA
{

#ifndef BA_PROFILER_DISABLED

void GpuProfiler::Initialize()
{
    BA_ASSERT(g_graphicsDevice);
    BA_ASSERT(m_gpuContext == nullptr);
    m_gpuContext = TracyD3D11Context(g_graphicsDevice->GetDevice(), g_graphicsDevice->GetDeviceContext());
}

void GpuProfiler::Shutdown()
{
    if (m_gpuContext != nullptr)
    {
        TracyD3D11Destroy(m_gpuContext);
        m_gpuContext = nullptr;
    }
}

TracyD3D11Ctx GpuProfiler::GetGpuContext() const
{
    return m_gpuContext;
}

#endif // BA_PROFILER_DISABLED

std::unique_ptr<GpuProfiler> g_gpuProfiler;

} // namespace BA
