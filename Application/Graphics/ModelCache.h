#pragma once

#include "Graphics/ModelLoader.h"

namespace BA
{

// Unified cache framework constants. All BA caches share the magic; typeId distinguishes kinds.
// Format assumes x64 Windows little-endian; revisit if ever ported to other platforms.
inline constexpr uint32_t kCacheMagic              = 0x41434142; // 'BACA' little-endian
inline constexpr uint32_t kCacheTypeIdModel        = 1;
inline constexpr uint32_t kModelCacheFormatVersion = 1;
inline constexpr uint64_t kCacheHeaderSize         = 48;

struct ModelCacheReadResult
{
    bool isHit = false;
    LoadedModelData data;
};

std::string ComputeCachePath(const std::string& sourceFilePath);
bool EnsureCacheDirectoryExists(const std::string& cachePath);
ModelCacheReadResult ReadModelCache(const std::string& cachePath, const std::string& sourceFilePath);
bool WriteModelCache(const std::string& cachePath, const std::string& sourceFilePath, const LoadedModelData& data);

} // namespace BA
