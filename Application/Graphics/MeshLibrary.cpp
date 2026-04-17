#include "Core/PCH.h"
#include "Graphics/MeshLibrary.h"
#include "Graphics/Vertex.h"
#include "Graphics/ModelLoader.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/TextureLibrary.h"
#include "Core/PathUtils.h"

#include <cctype>
#include <filesystem>

namespace BA
{

namespace
{

constexpr const char* kDefaultMeshName = "cube";
constexpr const char* kModelsDirectoryRelative = "Assets/Models";
constexpr std::string_view kGlbExtension = ".glb";
constexpr std::string_view kGltfExtension = ".gltf";

std::string ToLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

} // namespace

void MeshLibrary::Initialize()
{
    CreateBuiltInCube();
    LoadModelsFromAssetsDirectory();

    BA_LOG_INFO("MeshLibrary initialized.");
}

void MeshLibrary::Shutdown()
{
    m_meshes.clear();

    BA_LOG_INFO("MeshLibrary shutdown.");
}

const Mesh* MeshLibrary::FindMesh(const std::string& name) const
{
    auto it = m_meshes.find(name);
    if (it == m_meshes.end())
    {
        return nullptr;
    }

    return &it->second;
}

const Mesh* MeshLibrary::GetDefaultMesh() const
{
    return FindMesh(kDefaultMeshName);
}

std::vector<std::string> MeshLibrary::GetMeshNames() const
{
    std::vector<std::string> names;
    names.reserve(m_meshes.size());
    for (const auto& entry : m_meshes)
    {
        names.push_back(entry.first);
    }
    std::sort(names.begin(), names.end());
    return names;
}

bool MeshLibrary::LoadMesh(const std::string& name, const std::string& filePath)
{
    if (m_meshes.contains(name))
    {
        BA_LOG_WARN("Mesh '{}' already loaded, skipping", name);
        return true;
    }

    // Resolve asset path: wstring for ResolveAssetPath, then convert to narrow string via filesystem::path
    std::wstring wideRelative(filePath.begin(), filePath.end());
    std::wstring wideResolved = ResolveAssetPath(wideRelative.c_str());
    std::string resolvedPath = std::filesystem::path(wideResolved).string();

    LoadedMeshData meshData = LoadModelFromFile(resolvedPath);
    if (!meshData.isLoaded)
    {
        return false;
    }

    bool isIndex32Bit = meshData.vertices.size() > UINT16_MAX;

    Mesh mesh;
    mesh.vertexBuffer = g_graphicsDevice->CreateVertexBuffer(
        meshData.vertices.data(),
        static_cast<UINT>(meshData.vertices.size() * sizeof(Vertex))
    );
    mesh.indexCount = static_cast<uint32_t>(meshData.indices.size());
    mesh.isIndex32Bit = isIndex32Bit;

    if (isIndex32Bit)
    {
        mesh.indexBuffer = g_graphicsDevice->CreateIndexBuffer(
            meshData.indices.data(),
            static_cast<UINT>(meshData.indices.size() * sizeof(uint32_t))
        );
    }
    else
    {
        std::vector<uint16_t> indices16;
        indices16.reserve(meshData.indices.size());
        for (uint32_t idx : meshData.indices)
        {
            indices16.push_back(static_cast<uint16_t>(idx));
        }
        mesh.indexBuffer = g_graphicsDevice->CreateIndexBuffer(
            indices16.data(),
            static_cast<UINT>(indices16.size() * sizeof(uint16_t))
        );
    }

    if (meshData.hasTexture)
    {
        std::string textureName = name + "::diffuse";
        g_textureLibrary->RegisterTexture(
            textureName,
            meshData.textureRgba8.data(),
            meshData.textureWidth,
            meshData.textureHeight
        );
        mesh.textureName = std::move(textureName);
    }

    m_meshes[name] = std::move(mesh);
    BA_LOG_INFO("Loaded mesh '{}' ({} vertices, {} indices)", name, meshData.vertices.size(), meshData.indices.size());

    return true;
}

void MeshLibrary::CreateBuiltInCube()
{
    // Unit cube [-0.5, 0.5]^3, 24 vertices (4 per face, each with face normal), CW winding (LH)
    // Per-face UV layout (index 0/1/2/3): (0,0) / (1,0) / (1,1) / (0,1)
    Vertex vertices[] =
    {
        // +Z face (front)
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}, {0.0f, 1.0f}},

        // -Z face (back)
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f,  0.0f, -1.0f}, {0.0f, 1.0f}},

        // +Y face (top)
        {{-0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  1.0f,  0.0f}, {0.0f, 1.0f}},

        // -Y face (bottom)
        {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f}, {0.0f, 1.0f}},

        // +X face (right)
        {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},

        // -X face (left)
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f}, {0.0f, 1.0f}},
    };

    uint16_t indices[] =
    {
        // +Z
         0,  2,  1,   0,  3,  2,
        // -Z
         4,  6,  5,   4,  7,  6,
        // +Y
         8, 10,  9,   8, 11, 10,
        // -Y
        12, 14, 13,  12, 15, 14,
        // +X
        16, 18, 17,  16, 19, 18,
        // -X
        20, 22, 21,  20, 23, 22,
    };

    Mesh mesh;
    mesh.vertexBuffer = g_graphicsDevice->CreateVertexBuffer(vertices, sizeof(vertices));
    mesh.indexBuffer = g_graphicsDevice->CreateIndexBuffer(indices, sizeof(indices));
    mesh.indexCount = _countof(indices);
    mesh.isIndex32Bit = false;

    m_meshes[kDefaultMeshName] = std::move(mesh);
}

void MeshLibrary::LoadModelsFromAssetsDirectory()
{
    std::string_view relativeDir = kModelsDirectoryRelative;
    std::wstring wideRelativeDir(relativeDir.begin(), relativeDir.end());
    std::wstring resolvedDirW = ResolveAssetPath(wideRelativeDir.c_str());
    std::filesystem::path dirPath(resolvedDirW);

    if (!std::filesystem::exists(dirPath))
    {
        BA_LOG_WARN("Assets/Models directory not found at '{}', skipping auto-load", dirPath.string());
        return;
    }

    try
    {
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(dirPath))
        {
            if (!entry.is_regular_file())
            {
                continue;
            }

            std::string extension = ToLower(entry.path().extension().string());
            if (extension != kGlbExtension && extension != kGltfExtension)
            {
                continue;
            }

            std::string meshName = entry.path().stem().string();
            if (meshName == kDefaultMeshName)
            {
                BA_LOG_WARN("Skipping '{}' — name collides with built-in mesh", entry.path().string());
                continue;
            }

            std::string relativePath = std::string(kModelsDirectoryRelative) + "/" + entry.path().filename().string();
            LoadMesh(meshName, relativePath);
        }
    }
    catch (const std::filesystem::filesystem_error& error)
    {
        BA_LOG_ERROR("Failed to scan models directory: {}", error.what());
    }
}

std::unique_ptr<MeshLibrary> g_meshLibrary;

} // namespace BA
