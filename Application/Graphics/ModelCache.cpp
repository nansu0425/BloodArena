#include "Core/PCH.h"
#include "Graphics/ModelCache.h"
#include "Core/PathUtils.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <type_traits>

namespace BA
{

namespace
{

constexpr uint32_t kFnvOffsetBasis = 0x811c9dc5u;
constexpr uint32_t kFnvPrime       = 0x01000193u;

constexpr const wchar_t* kCacheRootRelativeW = L".cache/Models";
constexpr std::string_view kCacheExtension   = ".bacache";

constexpr size_t kBaseColorFactorComponentCount = 4;

static_assert(std::is_trivially_copyable_v<Vertex>, "Vertex must be trivially copyable for caching");
static_assert(std::is_trivially_copyable_v<Matrix>, "Matrix must be trivially copyable for caching");

template <typename T>
struct ReadResult
{
    bool isOk = false;
    T    value{};
};

uint32_t Fnv1a32(const std::string& s)
{
    uint32_t hash = kFnvOffsetBasis;
    for (unsigned char c : s)
    {
        hash ^= c;
        hash *= kFnvPrime;
    }
    return hash;
}

std::string FormatHash8(uint32_t hash)
{
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%08x", hash);
    return std::string(buf);
}

#pragma pack(push, 1)
struct CacheHeader
{
    uint32_t magic;
    uint32_t typeId;
    uint32_t typeFormatVersion;
    uint32_t reserved0;
    int64_t  sourceMtimeNs;
    uint64_t sourceFileSize;
    uint64_t payloadByteLength;
    uint64_t reserved1;
};
#pragma pack(pop)
static_assert(sizeof(CacheHeader) == kCacheHeaderSize, "CacheHeader must match kCacheHeaderSize");
static_assert(std::is_trivially_copyable_v<CacheHeader>);

struct SourceStat
{
    bool     isOk     = false;
    int64_t  mtimeNs  = 0;
    uint64_t fileSize = 0;
};

SourceStat StatSource(const std::string& sourceFilePath)
{
    SourceStat result;
    std::error_code ec;

    auto mtime = std::filesystem::last_write_time(sourceFilePath, ec);
    if (ec)
    {
        return result;
    }

    auto size = std::filesystem::file_size(sourceFilePath, ec);
    if (ec)
    {
        return result;
    }

    result.isOk     = true;
    result.mtimeNs  = static_cast<int64_t>(mtime.time_since_epoch().count());
    result.fileSize = static_cast<uint64_t>(size);

    return result;
}

template <typename T>
bool WritePod(std::ostream& os, const T& v)
{
    static_assert(std::is_trivially_copyable_v<T>, "WritePod requires trivially copyable");
    os.write(reinterpret_cast<const char*>(&v), sizeof(T));
    return os.good();
}

template <typename T>
ReadResult<T> ReadPod(std::istream& is)
{
    static_assert(std::is_trivially_copyable_v<T>, "ReadPod requires trivially copyable");
    ReadResult<T> result;
    is.read(reinterpret_cast<char*>(&result.value), sizeof(T));
    result.isOk = is.good();
    return result;
}

template <typename T>
bool WritePodVector(std::ostream& os, const std::vector<T>& v)
{
    static_assert(std::is_trivially_copyable_v<T>, "WritePodVector requires trivially copyable");
    uint64_t count = static_cast<uint64_t>(v.size());
    if (!WritePod(os, count))
    {
        return false;
    }
    if (count == 0)
    {
        return true;
    }
    os.write(reinterpret_cast<const char*>(v.data()),
             static_cast<std::streamsize>(count * sizeof(T)));
    return os.good();
}

template <typename T>
ReadResult<std::vector<T>> ReadPodVector(std::istream& is)
{
    static_assert(std::is_trivially_copyable_v<T>, "ReadPodVector requires trivially copyable");
    ReadResult<std::vector<T>> result;

    ReadResult<uint64_t> countResult = ReadPod<uint64_t>(is);
    if (!countResult.isOk)
    {
        return result;
    }

    uint64_t count = countResult.value;
    result.value.resize(static_cast<size_t>(count));
    if (count == 0)
    {
        result.isOk = true;
        return result;
    }

    is.read(reinterpret_cast<char*>(result.value.data()),
            static_cast<std::streamsize>(count * sizeof(T)));
    result.isOk = is.good();
    return result;
}

bool WritePrimitive(std::ostream& os, const LoadedPrimitiveData& p)
{
    if (!WritePodVector(os, p.vertices))
    {
        return false;
    }
    if (!WritePodVector(os, p.indices))
    {
        return false;
    }
    return WritePod(os, p.materialIndex);
}

ReadResult<LoadedPrimitiveData> ReadPrimitive(std::istream& is)
{
    ReadResult<LoadedPrimitiveData> result;
    LoadedPrimitiveData p;

    ReadResult<std::vector<Vertex>> vertices = ReadPodVector<Vertex>(is);
    if (!vertices.isOk)
    {
        return result;
    }
    p.vertices = std::move(vertices.value);

    ReadResult<std::vector<uint32_t>> indices = ReadPodVector<uint32_t>(is);
    if (!indices.isOk)
    {
        return result;
    }
    p.indices = std::move(indices.value);

    ReadResult<int> materialIndex = ReadPod<int>(is);
    if (!materialIndex.isOk)
    {
        return result;
    }
    p.materialIndex = materialIndex.value;

    result.value = std::move(p);
    result.isOk = true;
    return result;
}

bool WriteMesh(std::ostream& os, const LoadedMeshData& m)
{
    uint64_t count = static_cast<uint64_t>(m.primitives.size());
    if (!WritePod(os, count))
    {
        return false;
    }

    for (const LoadedPrimitiveData& p : m.primitives)
    {
        if (!WritePrimitive(os, p))
        {
            return false;
        }
    }

    return os.good();
}

ReadResult<LoadedMeshData> ReadMesh(std::istream& is)
{
    ReadResult<LoadedMeshData> result;
    LoadedMeshData m;

    ReadResult<uint64_t> countResult = ReadPod<uint64_t>(is);
    if (!countResult.isOk)
    {
        return result;
    }

    m.primitives.reserve(static_cast<size_t>(countResult.value));
    for (uint64_t i = 0; i < countResult.value; ++i)
    {
        ReadResult<LoadedPrimitiveData> prim = ReadPrimitive(is);
        if (!prim.isOk)
        {
            return result;
        }
        m.primitives.push_back(std::move(prim.value));
    }

    result.value = std::move(m);
    result.isOk = true;

    return result;
}

bool WriteMaterial(std::ostream& os, const LoadedMaterialData& m)
{
    uint8_t hasDiffuseTexture = m.hasDiffuseTexture ? uint8_t{1} : uint8_t{0};
    if (!WritePod(os, hasDiffuseTexture))
    {
        return false;
    }
    if (!WritePod(os, m.diffuseWidth))
    {
        return false;
    }
    if (!WritePod(os, m.diffuseHeight))
    {
        return false;
    }
    uint64_t rgbaLen = static_cast<uint64_t>(m.diffuseRgba8.size());
    if (!WritePod(os, rgbaLen))
    {
        return false;
    }
    if (rgbaLen > 0)
    {
        os.write(reinterpret_cast<const char*>(m.diffuseRgba8.data()),
                 static_cast<std::streamsize>(rgbaLen));
        if (!os.good())
        {
            return false;
        }
    }
    std::array<float, kBaseColorFactorComponentCount> factor{};
    for (size_t i = 0; i < kBaseColorFactorComponentCount; ++i)
    {
        factor[i] = m.baseColorFactor[i];
    }
    if (!WritePod(os, factor))
    {
        return false;
    }
    uint32_t alphaMode = static_cast<uint32_t>(m.alphaMode);
    if (!WritePod(os, alphaMode))
    {
        return false;
    }
    if (!WritePod(os, m.alphaCutoff))
    {
        return false;
    }
    uint8_t isDoubleSided = m.isDoubleSided ? uint8_t{1} : uint8_t{0};
    return WritePod(os, isDoubleSided);
}

ReadResult<LoadedMaterialData> ReadMaterial(std::istream& is)
{
    ReadResult<LoadedMaterialData> result;
    LoadedMaterialData m;

    ReadResult<uint8_t> hasDiffuseTexture = ReadPod<uint8_t>(is);
    if (!hasDiffuseTexture.isOk)
    {
        return result;
    }
    m.hasDiffuseTexture = (hasDiffuseTexture.value != 0);

    ReadResult<uint32_t> width = ReadPod<uint32_t>(is);
    if (!width.isOk)
    {
        return result;
    }
    m.diffuseWidth = width.value;

    ReadResult<uint32_t> height = ReadPod<uint32_t>(is);
    if (!height.isOk)
    {
        return result;
    }
    m.diffuseHeight = height.value;

    ReadResult<std::vector<uint8_t>> rgba = ReadPodVector<uint8_t>(is);
    if (!rgba.isOk)
    {
        return result;
    }
    m.diffuseRgba8 = std::move(rgba.value);

    ReadResult<std::array<float, kBaseColorFactorComponentCount>> factor
        = ReadPod<std::array<float, kBaseColorFactorComponentCount>>(is);
    if (!factor.isOk)
    {
        return result;
    }
    for (size_t i = 0; i < kBaseColorFactorComponentCount; ++i)
    {
        m.baseColorFactor[i] = factor.value[i];
    }

    ReadResult<uint32_t> alphaMode = ReadPod<uint32_t>(is);
    if (!alphaMode.isOk)
    {
        return result;
    }
    m.alphaMode = static_cast<AlphaMode>(alphaMode.value);

    ReadResult<float> alphaCutoff = ReadPod<float>(is);
    if (!alphaCutoff.isOk)
    {
        return result;
    }
    m.alphaCutoff = alphaCutoff.value;

    ReadResult<uint8_t> isDoubleSided = ReadPod<uint8_t>(is);
    if (!isDoubleSided.isOk)
    {
        return result;
    }
    m.isDoubleSided = (isDoubleSided.value != 0);

    result.value = std::move(m);
    result.isOk = true;
    return result;
}

bool WriteNode(std::ostream& os, const LoadedNodeData& n)
{
    if (!WritePod(os, n.localTransform))
    {
        return false;
    }
    if (!WritePod(os, n.meshIndex))
    {
        return false;
    }
    return WritePodVector(os, n.childIndices);
}

ReadResult<LoadedNodeData> ReadNode(std::istream& is)
{
    ReadResult<LoadedNodeData> result;
    LoadedNodeData n;

    ReadResult<Matrix> localTransform = ReadPod<Matrix>(is);
    if (!localTransform.isOk)
    {
        return result;
    }
    n.localTransform = localTransform.value;

    ReadResult<int> meshIndex = ReadPod<int>(is);
    if (!meshIndex.isOk)
    {
        return result;
    }
    n.meshIndex = meshIndex.value;

    ReadResult<std::vector<int>> childIndices = ReadPodVector<int>(is);
    if (!childIndices.isOk)
    {
        return result;
    }
    n.childIndices = std::move(childIndices.value);

    result.value = std::move(n);
    result.isOk = true;

    return result;
}

bool WritePayload(std::ostream& os, const LoadedModelData& data)
{
    BA_PROFILE_SCOPE("ModelCache::SerializePayload");

    uint64_t nodeCount = static_cast<uint64_t>(data.nodes.size());
    if (!WritePod(os, nodeCount))
    {
        return false;
    }
    for (const LoadedNodeData& n : data.nodes)
    {
        if (!WriteNode(os, n))
        {
            return false;
        }
    }

    uint64_t meshCount = static_cast<uint64_t>(data.meshes.size());
    if (!WritePod(os, meshCount))
    {
        return false;
    }
    for (const LoadedMeshData& m : data.meshes)
    {
        if (!WriteMesh(os, m))
        {
            return false;
        }
    }

    uint64_t matCount = static_cast<uint64_t>(data.materials.size());
    if (!WritePod(os, matCount))
    {
        return false;
    }
    for (const LoadedMaterialData& m : data.materials)
    {
        if (!WriteMaterial(os, m))
        {
            return false;
        }
    }

    return WritePodVector(os, data.rootNodeIndices);
}

ReadResult<LoadedModelData> ReadPayload(std::istream& is)
{
    BA_PROFILE_SCOPE("ModelCache::DeserializePayload");

    ReadResult<LoadedModelData> result;
    LoadedModelData data;

    ReadResult<uint64_t> nodeCount = ReadPod<uint64_t>(is);
    if (!nodeCount.isOk)
    {
        return result;
    }

    data.nodes.reserve(static_cast<size_t>(nodeCount.value));
    for (uint64_t i = 0; i < nodeCount.value; ++i)
    {
        ReadResult<LoadedNodeData> node = ReadNode(is);
        if (!node.isOk)
        {
            return result;
        }
        data.nodes.push_back(std::move(node.value));
    }

    ReadResult<uint64_t> meshCount = ReadPod<uint64_t>(is);
    if (!meshCount.isOk)
    {
        return result;
    }
    data.meshes.reserve(static_cast<size_t>(meshCount.value));
    for (uint64_t i = 0; i < meshCount.value; ++i)
    {
        ReadResult<LoadedMeshData> mesh = ReadMesh(is);
        if (!mesh.isOk)
        {
            return result;
        }
        data.meshes.push_back(std::move(mesh.value));
    }

    ReadResult<uint64_t> matCount = ReadPod<uint64_t>(is);
    if (!matCount.isOk)
    {
        return result;
    }
    data.materials.reserve(static_cast<size_t>(matCount.value));
    for (uint64_t i = 0; i < matCount.value; ++i)
    {
        ReadResult<LoadedMaterialData> material = ReadMaterial(is);
        if (!material.isOk)
        {
            return result;
        }
        data.materials.push_back(std::move(material.value));
    }

    ReadResult<std::vector<int>> rootNodes = ReadPodVector<int>(is);
    if (!rootNodes.isOk)
    {
        return result;
    }
    data.rootNodeIndices = std::move(rootNodes.value);

    result.value = std::move(data);
    result.isOk = true;

    return result;
}

} // namespace

std::string ComputeCachePath(const std::string& sourceFilePath)
{
    std::filesystem::path src(sourceFilePath);
    std::string stem = src.stem().string();

    uint32_t hash = Fnv1a32(sourceFilePath);
    std::string fileName = stem + "_" + FormatHash8(hash) + std::string(kCacheExtension);

    std::wstring rootW = ResolveApplicationPath(kCacheRootRelativeW);
    std::filesystem::path full = std::filesystem::path(rootW) / fileName;

    return full.string();
}

bool EnsureCacheDirectoryExists(const std::string& cachePath)
{
    std::filesystem::path parent = std::filesystem::path(cachePath).parent_path();
    if (parent.empty())
    {
        return true;
    }

    std::error_code ec;
    std::filesystem::create_directories(parent, ec);
    if (ec)
    {
        BA_LOG_WARN("Failed to create cache directory '{}': {}", parent.string(), ec.message());
        return false;
    }

    return true;
}

ModelCacheReadResult ReadModelCache(const std::string& cachePath, const std::string& sourceFilePath)
{
    ModelCacheReadResult result;

    std::error_code ec;
    if (!std::filesystem::exists(cachePath, ec))
    {
        return result;
    }

    std::ifstream is(cachePath, std::ios::binary);
    if (!is.is_open())
    {
        return result;
    }

    ReadResult<CacheHeader> headerResult = ReadPod<CacheHeader>(is);
    if (!headerResult.isOk)
    {
        BA_LOG_WARN("ModelCache header truncated for '{}'", cachePath);
        return result;
    }
    const CacheHeader& header = headerResult.value;

    if (header.magic != kCacheMagic)
    {
        BA_LOG_WARN("ModelCache magic mismatch for '{}'", cachePath);
        return result;
    }

    if (header.typeId != kCacheTypeIdModel)
    {
        BA_LOG_WARN("ModelCache typeId mismatch for '{}' (expected {}, got {})",
            cachePath, kCacheTypeIdModel, header.typeId);
        return result;
    }

    if (header.reserved0 != 0 || header.reserved1 != 0)
    {
        BA_LOG_WARN("ModelCache reserved fields non-zero for '{}'", cachePath);
        return result;
    }

    if (header.typeFormatVersion != kModelCacheFormatVersion)
    {
        BA_LOG_INFO("ModelCache version mismatch for '{}' (expected {}, got {}); recooking",
            cachePath, kModelCacheFormatVersion, header.typeFormatVersion);
        return result;
    }

    SourceStat src = StatSource(sourceFilePath);
    if (!src.isOk)
    {
        return result;
    }

    if (header.sourceMtimeNs != src.mtimeNs || header.sourceFileSize != src.fileSize)
    {
        BA_LOG_INFO("ModelCache stale for '{}'; recooking", cachePath);
        return result;
    }

    ReadResult<LoadedModelData> payload = ReadPayload(is);
    if (!payload.isOk)
    {
        BA_LOG_WARN("ModelCache payload corrupted for '{}'", cachePath);
        return result;
    }

    result.data = std::move(payload.value);
    result.data.isLoaded = true;
    result.isHit = true;

    return result;
}

bool WriteModelCache(const std::string& cachePath, const std::string& sourceFilePath, const LoadedModelData& data)
{
    if (!EnsureCacheDirectoryExists(cachePath))
    {
        return false;
    }

    SourceStat src = StatSource(sourceFilePath);
    if (!src.isOk)
    {
        BA_LOG_WARN("Failed to stat source '{}' for cache write", sourceFilePath);
        return false;
    }

    std::string tmpPath = cachePath + ".tmp";

    {
        std::ofstream os(tmpPath, std::ios::binary | std::ios::trunc);
        if (!os.is_open())
        {
            BA_LOG_WARN("Failed to open temp cache file '{}'", tmpPath);
            return false;
        }

        CacheHeader header{};
        header.magic             = kCacheMagic;
        header.typeId            = kCacheTypeIdModel;
        header.typeFormatVersion = kModelCacheFormatVersion;
        header.reserved0         = 0;
        header.sourceMtimeNs     = src.mtimeNs;
        header.sourceFileSize    = src.fileSize;
        header.payloadByteLength = 0; // placeholder, rewritten after payload is sized
        header.reserved1         = 0;
        if (!WritePod(os, header))
        {
            BA_LOG_WARN("Failed to write cache header to '{}'", tmpPath);
            return false;
        }

        std::streampos payloadStart = os.tellp();
        if (!WritePayload(os, data))
        {
            BA_LOG_WARN("Failed to write cache payload to '{}'", tmpPath);
            return false;
        }
        std::streampos payloadEnd = os.tellp();

        std::streamoff payloadByteLength = payloadEnd - payloadStart;
        if (payloadByteLength < 0)
        {
            BA_LOG_WARN("Invalid payload length while writing '{}'", tmpPath);
            return false;
        }
        header.payloadByteLength = static_cast<uint64_t>(payloadByteLength);

        os.seekp(0);
        if (!WritePod(os, header))
        {
            BA_LOG_WARN("Failed to rewrite cache header for '{}'", tmpPath);
            return false;
        }
    } // ofstream destructor flushes and closes

    std::error_code ec;
    std::filesystem::rename(tmpPath, cachePath, ec);
    if (ec)
    {
        std::error_code rmEc;
        std::filesystem::remove(tmpPath, rmEc);
        BA_LOG_WARN("Failed to atomic-rename cache file '{}': {}", cachePath, ec.message());
        return false;
    }

    return true;
}

} // namespace BA
