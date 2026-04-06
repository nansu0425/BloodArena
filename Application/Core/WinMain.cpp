#include "Core/PCH.h"
#include "Core/Lifecycle.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPWSTR /*lpCmdLine*/, int nShowCmd)
{
    BA::Initialize(hInstance, nShowCmd);
    int exitCode = BA::Run();
    BA::Shutdown();

    return exitCode;
}
