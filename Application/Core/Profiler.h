#pragma once

#ifndef BA_PROFILER_DISABLED

#define TRACY_ENABLE

#include <tracy/Tracy.hpp>

#define BA_PROFILE_SCOPE(name)          ZoneScopedN(name)
#define BA_PROFILE_SCOPE_DYNAMIC(s)     ZoneScoped; ZoneName((s).data(), (s).size())
#define BA_PROFILE_FRAME_MARK()         FrameMark

#else // BA_PROFILER_DISABLED

#define BA_PROFILE_SCOPE(name)          ((void)0)
#define BA_PROFILE_SCOPE_DYNAMIC(s)     ((void)0)
#define BA_PROFILE_FRAME_MARK()         ((void)0)

#endif // BA_PROFILER_DISABLED
