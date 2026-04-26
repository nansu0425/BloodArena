#include "Core/PCH.h"
#include "Graphics/ModelLibrary.h"
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

constexpr const char* kModelsDirectoryRelative = "Assets/Models";
constexpr std::string_view kGlbExtension = ".glb";
constexpr std::string_view kGltfExtension = ".gltf";

std::string ToLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string BuildDiffuseTextureName(const std::string& modelName, size_t materialIndex)
{
    return modelName + "::mat" + std::to_string(materialIndex) + "::diffuse";
}

Primitive CreatePrimitiveFromLoaded(const LoadedPrimitiveData& src)
{
    BA_PROFILE_SCOPE("CreatePrimitiveFromLoaded");

    Primitive p;
    p.materialIndex = src.materialIndex;
    p.indexCount = static_cast<uint32_t>(src.indices.size());
    p.isIndex32Bit = (src.vertices.size() > UINT16_MAX);

    p.vertexBuffer = g_graphicsDevice->CreateVertexBuffer(
        src.vertices.data(),
        static_cast<UINT>(src.vertices.size() * sizeof(Vertex))
    );

    if (p.isIndex32Bit)
    {
        p.indexBuffer = g_graphicsDevice->CreateIndexBuffer(
            src.indices.data(),
            static_cast<UINT>(src.indices.size() * sizeof(uint32_t))
        );
    }
    else
    {
        std::vector<uint16_t> indices16;
        indices16.reserve(src.indices.size());
        for (uint32_t idx : src.indices)
        {
            indices16.push_back(static_cast<uint16_t>(idx));
        }
        p.indexBuffer = g_graphicsDevice->CreateIndexBuffer(
            indices16.data(),
            static_cast<UINT>(indices16.size() * sizeof(uint16_t))
        );
    }

    p.cpuPositions.reserve(src.vertices.size());
    for (const Vertex& v : src.vertices)
    {
        p.cpuPositions.push_back(v.position);
        p.localAabb = ExpandAabbWithPoint(p.localAabb, v.position);
    }
    p.cpuIndices = src.indices;

    return p;
}

} // namespace

void ModelLibrary::Initialize()
{
    BA_PROFILE_SCOPE("ModelLibrary::Initialize");

    CreateBuiltInCube();
    LoadModelsFromAssetsDirectory();

    BA_LOG_INFO("ModelLibrary initialized.");
}

void ModelLibrary::Shutdown()
{
    m_models.clear();

    BA_LOG_INFO("ModelLibrary shutdown.");
}

const Model* ModelLibrary::FindModel(const std::string& name) const
{
    auto it = m_models.find(name);
    if (it == m_models.end())
    {
        return nullptr;
    }

    return &it->second;
}

std::vector<std::string> ModelLibrary::GetModelNames() const
{
    std::vector<std::string> names;
    names.reserve(m_models.size());
    for (const auto& entry : m_models)
    {
        names.push_back(entry.first);
    }
    std::sort(names.begin(), names.end());
    return names;
}

bool ModelLibrary::LoadModel(const std::string& name, const std::string& filePath)
{
    std::string profileLabel = "LoadModel: " + name;
    BA_PROFILE_SCOPE_DYNAMIC(profileLabel);

    if (m_models.contains(name))
    {
        BA_LOG_WARN("Model '{}' already loaded, skipping", name);
        return true;
    }

    // Resolve asset path: wstring for ResolveAssetPath, then convert to narrow string via filesystem::path
    std::wstring wideRelative(filePath.begin(), filePath.end());
    std::wstring wideResolved = ResolveAssetPath(wideRelative.c_str());
    std::string resolvedPath = std::filesystem::path(wideResolved).string();

    LoadedModelData data = LoadModelFromFile(resolvedPath);
    if (!data.isLoaded)
    {
        return false;
    }

    Model model;

    // Materials: register each diffuse texture under "<modelName>::mat<idx>::diffuse".
    {
        BA_PROFILE_SCOPE("RegisterMaterialTextures");

        model.materials.reserve(data.materials.size());
        for (size_t i = 0; i < data.materials.size(); ++i)
        {
            Material mat;
            const LoadedMaterialData& src = data.materials[i];
            if (src.hasDiffuseTexture)
            {
                std::string textureName = BuildDiffuseTextureName(name, i);
                g_textureLibrary->RegisterTexture(
                    textureName,
                    src.diffuseRgba8.data(),
                    src.diffuseWidth,
                    src.diffuseHeight
                );
                mat.diffuseTextureName = std::move(textureName);
            }
            std::copy(std::begin(src.baseColorFactor), std::end(src.baseColorFactor), std::begin(mat.baseColorFactor));
            mat.alphaMode     = src.alphaMode;
            mat.alphaCutoff   = src.alphaCutoff;
            mat.isDoubleSided = src.isDoubleSided;
            model.materials.push_back(std::move(mat));
        }
    }

    // Meshes: upload per-primitive VB/IB.
    {
        BA_PROFILE_SCOPE("UploadMeshPrimitives");

        model.meshes.reserve(data.meshes.size());
        for (const LoadedMeshData& srcMesh : data.meshes)
        {
            Mesh dstMesh;
            dstMesh.primitives.reserve(srcMesh.primitives.size());
            for (const LoadedPrimitiveData& srcPrim : srcMesh.primitives)
            {
                dstMesh.primitives.push_back(CreatePrimitiveFromLoaded(srcPrim));
            }
            model.meshes.push_back(std::move(dstMesh));
        }
    }

    // Nodes: direct copy (indices already match glTF ordering).
    model.nodes.reserve(data.nodes.size());
    for (const LoadedNodeData& src : data.nodes)
    {
        Node node;
        node.localTransform = src.localTransform;
        node.meshIndex = src.meshIndex;
        node.childIndices = src.childIndices;
        model.nodes.push_back(std::move(node));
    }

    model.rootNodeIndices = std::move(data.rootNodeIndices);

    BA_LOG_INFO(
        "Loaded model '{}' ({} nodes, {} meshes, {} materials)",
        name,
        model.nodes.size(),
        model.meshes.size(),
        model.materials.size()
    );

    m_models[name] = std::move(model);

    return true;
}

void ModelLibrary::CreateBuiltInCube()
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

    Primitive prim;
    prim.vertexBuffer = g_graphicsDevice->CreateVertexBuffer(vertices, sizeof(vertices));
    prim.indexBuffer = g_graphicsDevice->CreateIndexBuffer(indices, sizeof(indices));
    prim.indexCount = _countof(indices);
    prim.isIndex32Bit = false;
    prim.materialIndex = -1;

    prim.cpuPositions.reserve(_countof(vertices));
    for (const Vertex& v : vertices)
    {
        prim.cpuPositions.push_back(v.position);
        prim.localAabb = ExpandAabbWithPoint(prim.localAabb, v.position);
    }
    prim.cpuIndices.reserve(_countof(indices));
    for (uint16_t idx : indices)
    {
        prim.cpuIndices.push_back(static_cast<uint32_t>(idx));
    }

    Mesh mesh;
    mesh.primitives.push_back(std::move(prim));

    Node node;
    node.meshIndex = 0;

    Model model;
    model.meshes.push_back(std::move(mesh));
    model.nodes.push_back(std::move(node));
    model.rootNodeIndices.push_back(0);

    m_models[kDefaultModelName] = std::move(model);
}

void ModelLibrary::LoadModelsFromAssetsDirectory()
{
    BA_PROFILE_SCOPE("ModelLibrary::LoadModelsFromAssetsDirectory");

    std::string_view relativeDir = kModelsDirectoryRelative;
    std::wstring wideRelativeDir(relativeDir.begin(), relativeDir.end());
    std::wstring resolvedDirW = ResolveAssetPath(wideRelativeDir.c_str());
    std::filesystem::path dirPath(resolvedDirW);

    std::error_code existsErr;
    if (!std::filesystem::exists(dirPath, existsErr))
    {
        BA_LOG_WARN("Assets/Models directory not found at '{}', skipping auto-load", dirPath.string());
        return;
    }

    std::error_code iterErr;
    std::filesystem::recursive_directory_iterator it(dirPath, iterErr);
    if (iterErr)
    {
        BA_LOG_ERROR("Failed to open models directory '{}': {}", dirPath.string(), iterErr.message());
        return;
    }

    for (; it != std::filesystem::recursive_directory_iterator(); it.increment(iterErr))
    {
        if (iterErr)
        {
            BA_LOG_ERROR("Failed to scan models directory: {}", iterErr.message());
            return;
        }

        const std::filesystem::directory_entry& entry = *it;
        if (!entry.is_regular_file())
        {
            continue;
        }

        std::string extension = ToLower(entry.path().extension().string());
        if (extension != kGlbExtension && extension != kGltfExtension)
        {
            continue;
        }

        std::string modelName = entry.path().stem().string();
        if (modelName == kDefaultModelName)
        {
            BA_LOG_WARN("Skipping '{}' — name collides with built-in model", entry.path().string());
            continue;
        }

        std::error_code relErr;
        std::filesystem::path relativeToDir = std::filesystem::relative(entry.path(), dirPath, relErr);
        if (relErr)
        {
            BA_LOG_ERROR("Failed to resolve relative path for '{}': {}", entry.path().string(), relErr.message());
            continue;
        }

        std::string relativePath = std::string(kModelsDirectoryRelative) + "/" + relativeToDir.generic_string();
        LoadModel(modelName, relativePath);
    }
}

std::unique_ptr<ModelLibrary> g_modelLibrary;

} // namespace BA
