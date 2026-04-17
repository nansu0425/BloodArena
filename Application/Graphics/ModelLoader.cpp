#include "Core/PCH.h"
#include "Graphics/ModelLoader.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include <tiny_gltf.h>

namespace BA
{

namespace
{

struct LoadedTextureData
{
    bool hasTexture = false;
    std::vector<uint8_t> rgba;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct ExtractionContext
{
    std::vector<Vertex> allVertices;
    std::vector<uint32_t> allIndices;
    int firstMaterialIndex = -1;
    bool hasAnyNormals = false;
    bool hasMultipleMaterials = false;
};

const float* GetAttributeData(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const char* attribute)
{
    auto it = primitive.attributes.find(attribute);
    if (it == primitive.attributes.end())
    {
        return nullptr;
    }

    const tinygltf::Accessor& accessor = model.accessors[it->second];
    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buf = model.buffers[view.buffer];

    return reinterpret_cast<const float*>(&buf.data[view.byteOffset + accessor.byteOffset]);
}

int GetAttributeCount(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const char* attribute)
{
    auto it = primitive.attributes.find(attribute);
    if (it == primitive.attributes.end())
    {
        return 0;
    }

    return static_cast<int>(model.accessors[it->second].count);
}

int GetAttributeByteStride(const tinygltf::Model& model, const tinygltf::Primitive& primitive, const char* attribute)
{
    auto it = primitive.attributes.find(attribute);
    if (it == primitive.attributes.end())
    {
        return 0;
    }

    const tinygltf::Accessor& accessor = model.accessors[it->second];
    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];

    int stride = static_cast<int>(view.byteStride);
    if (stride == 0)
    {
        // Tightly packed
        int componentSize = tinygltf::GetComponentSizeInBytes(static_cast<uint32_t>(accessor.componentType));
        int numComponents = tinygltf::GetNumComponentsInType(static_cast<uint32_t>(accessor.type));
        stride = componentSize * numComponents;
    }

    return stride;
}

Matrix LoadColumnMajorMatrix(const std::vector<double>& m)
{
    // glTF stores `matrix` as 16 floats in column-major order (first 4 values = column 0).
    // SimpleMath::Matrix takes a row-major 4x4 constructor, so transpose while loading.
    return Matrix(
        static_cast<float>(m[0]),  static_cast<float>(m[4]),  static_cast<float>(m[8]),  static_cast<float>(m[12]),
        static_cast<float>(m[1]),  static_cast<float>(m[5]),  static_cast<float>(m[9]),  static_cast<float>(m[13]),
        static_cast<float>(m[2]),  static_cast<float>(m[6]),  static_cast<float>(m[10]), static_cast<float>(m[14]),
        static_cast<float>(m[3]),  static_cast<float>(m[7]),  static_cast<float>(m[11]), static_cast<float>(m[15])
    );
}

Matrix ComputeNodeLocalTransform(const tinygltf::Node& node)
{
    constexpr size_t kMat4ElementCount = 16;

    if (node.matrix.size() == kMat4ElementCount)
    {
        return LoadColumnMajorMatrix(node.matrix);
    }

    Vector3 translation = {0.0f, 0.0f, 0.0f};
    if (node.translation.size() == 3)
    {
        translation.x = static_cast<float>(node.translation[0]);
        translation.y = static_cast<float>(node.translation[1]);
        translation.z = static_cast<float>(node.translation[2]);
    }

    Quaternion rotation = Quaternion::Identity;
    if (node.rotation.size() == 4)
    {
        // glTF rotation is stored as xyzw; SimpleMath::Quaternion uses the same order.
        rotation.x = static_cast<float>(node.rotation[0]);
        rotation.y = static_cast<float>(node.rotation[1]);
        rotation.z = static_cast<float>(node.rotation[2]);
        rotation.w = static_cast<float>(node.rotation[3]);
    }

    Vector3 scale = {1.0f, 1.0f, 1.0f};
    if (node.scale.size() == 3)
    {
        scale.x = static_cast<float>(node.scale[0]);
        scale.y = static_cast<float>(node.scale[1]);
        scale.z = static_cast<float>(node.scale[2]);
    }

    return Matrix::CreateScale(scale)
         * Matrix::CreateFromQuaternion(rotation)
         * Matrix::CreateTranslation(translation);
}

std::vector<Vertex> ExtractVertices(const tinygltf::Model& model, const tinygltf::Primitive& primitive)
{
    int vertexCount = GetAttributeCount(model, primitive, "POSITION");
    BA_ASSERT(vertexCount > 0);

    const float* positions = GetAttributeData(model, primitive, "POSITION");
    const float* normals   = GetAttributeData(model, primitive, "NORMAL");
    const float* texcoords = GetAttributeData(model, primitive, "TEXCOORD_0");

    int posStride = GetAttributeByteStride(model, primitive, "POSITION") / sizeof(float);
    int normStride = normals ? GetAttributeByteStride(model, primitive, "NORMAL") / sizeof(float) : 0;
    int uvStride = texcoords ? GetAttributeByteStride(model, primitive, "TEXCOORD_0") / sizeof(float) : 0;

    std::vector<Vertex> vertices(vertexCount);

    for (int i = 0; i < vertexCount; ++i)
    {
        Vertex& v = vertices[i];

        // Raw glTF (right-handed) values. Handedness conversion is deferred to ConvertRhToLh,
        // after node transforms are applied in glTF's native RH space.
        v.position.x = positions[i * posStride + 0];
        v.position.y = positions[i * posStride + 1];
        v.position.z = positions[i * posStride + 2];

        if (normals)
        {
            v.normal.x = normals[i * normStride + 0];
            v.normal.y = normals[i * normStride + 1];
            v.normal.z = normals[i * normStride + 2];
        }
        else
        {
            v.normal = {0.0f, 1.0f, 0.0f};
        }

        // UV: no conversion needed (both top-left origin)
        if (texcoords)
        {
            v.uv.x = texcoords[i * uvStride + 0];
            v.uv.y = texcoords[i * uvStride + 1];
        }
        else
        {
            v.uv = {0.0f, 0.0f};
        }
    }

    return vertices;
}

std::vector<uint32_t> ExtractIndices(const tinygltf::Model& model, const tinygltf::Primitive& primitive, uint32_t baseVertex)
{
    BA_ASSERT(primitive.indices >= 0);

    const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buf = model.buffers[view.buffer];
    const uint8_t* rawData = &buf.data[view.byteOffset + accessor.byteOffset];

    int indexCount = static_cast<int>(accessor.count);
    BA_ASSERT(indexCount % 3 == 0);

    std::vector<uint32_t> indices(indexCount);

    for (int i = 0; i < indexCount; ++i)
    {
        uint32_t index = 0;

        switch (accessor.componentType)
        {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            index = rawData[i];
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            index = reinterpret_cast<const uint16_t*>(rawData)[i];
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            index = reinterpret_cast<const uint32_t*>(rawData)[i];
            break;
        default:
            BA_ASSERT(false);
            break;
        }

        indices[i] = index + baseVertex;
    }

    return indices;
}

void ApplyNodeTransform(std::vector<Vertex>& vertices, size_t firstVertexIndex, const Matrix& worldTransform, bool hasNormals)
{
    Matrix normalMatrix = worldTransform.Invert().Transpose();

    for (size_t i = firstVertexIndex; i < vertices.size(); ++i)
    {
        Vertex& v = vertices[i];
        v.position = Vector3::Transform(v.position, worldTransform);

        if (hasNormals)
        {
            v.normal = Vector3::TransformNormal(v.normal, normalMatrix);
            v.normal.Normalize();
        }
    }
}

void ExtractMeshWithTransform(
    const tinygltf::Model& model,
    const tinygltf::Mesh& mesh,
    const Matrix& worldTransform,
    ExtractionContext& ctx)
{
    for (const tinygltf::Primitive& primitive : mesh.primitives)
    {
        if (primitive.mode != TINYGLTF_MODE_TRIANGLES && primitive.mode != -1)
        {
            BA_LOG_WARN("Skipping non-triangle primitive (mode={})", primitive.mode);
            continue;
        }

        if (primitive.attributes.find("POSITION") == primitive.attributes.end())
        {
            BA_LOG_WARN("Skipping primitive without POSITION attribute");
            continue;
        }

        if (primitive.indices < 0)
        {
            BA_LOG_WARN("Skipping non-indexed primitive");
            continue;
        }

        if (ctx.firstMaterialIndex < 0)
        {
            ctx.firstMaterialIndex = primitive.material;
        }
        else if (primitive.material != ctx.firstMaterialIndex)
        {
            ctx.hasMultipleMaterials = true;
        }

        uint32_t baseVertex = static_cast<uint32_t>(ctx.allVertices.size());

        bool hasNormals = primitive.attributes.find("NORMAL") != primitive.attributes.end();
        ctx.hasAnyNormals = ctx.hasAnyNormals || hasNormals;

        std::vector<Vertex> primVertices = ExtractVertices(model, primitive);
        std::vector<uint32_t> primIndices = ExtractIndices(model, primitive, baseVertex);

        size_t firstVertexIndex = ctx.allVertices.size();

        ctx.allVertices.insert(
            ctx.allVertices.end(),
            std::make_move_iterator(primVertices.begin()),
            std::make_move_iterator(primVertices.end())
        );
        ctx.allIndices.insert(
            ctx.allIndices.end(),
            std::make_move_iterator(primIndices.begin()),
            std::make_move_iterator(primIndices.end())
        );

        ApplyNodeTransform(ctx.allVertices, firstVertexIndex, worldTransform, hasNormals);
    }
}

void WalkNode(
    const tinygltf::Model& model,
    int nodeIndex,
    const Matrix& parentWorldTransform,
    ExtractionContext& ctx)
{
    BA_ASSERT(nodeIndex >= 0 && nodeIndex < static_cast<int>(model.nodes.size()));

    const tinygltf::Node& node = model.nodes[nodeIndex];
    Matrix localTransform = ComputeNodeLocalTransform(node);

    // Row-vector convention: v' = v * (local * parent) applies local first, then parent.
    Matrix worldTransform = localTransform * parentWorldTransform;

    if (node.mesh >= 0 && node.mesh < static_cast<int>(model.meshes.size()))
    {
        ExtractMeshWithTransform(model, model.meshes[node.mesh], worldTransform, ctx);
    }

    for (int childIndex : node.children)
    {
        WalkNode(model, childIndex, worldTransform, ctx);
    }
}

std::vector<uint8_t> PadRgbToRgba(const std::vector<uint8_t>& rgb)
{
    BA_ASSERT(rgb.size() % 3 == 0);

    size_t pixelCount = rgb.size() / 3;
    std::vector<uint8_t> rgba(pixelCount * 4);

    for (size_t i = 0; i < pixelCount; ++i)
    {
        rgba[i * 4 + 0] = rgb[i * 3 + 0];
        rgba[i * 4 + 1] = rgb[i * 3 + 1];
        rgba[i * 4 + 2] = rgb[i * 3 + 2];
        rgba[i * 4 + 3] = 255;
    }

    return rgba;
}

LoadedTextureData ExtractBaseColorTexture(const tinygltf::Model& model, int materialIndex)
{
    if (materialIndex < 0 || materialIndex >= static_cast<int>(model.materials.size()))
    {
        return {};
    }

    const tinygltf::Material& material = model.materials[materialIndex];
    int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
    if (textureIndex < 0 || textureIndex >= static_cast<int>(model.textures.size()))
    {
        return {};
    }

    int imageIndex = model.textures[textureIndex].source;
    if (imageIndex < 0 || imageIndex >= static_cast<int>(model.images.size()))
    {
        return {};
    }

    const tinygltf::Image& image = model.images[imageIndex];
    if (image.image.empty() || image.width <= 0 || image.height <= 0)
    {
        BA_LOG_WARN("baseColorTexture image is empty or invalid");
        return {};
    }

    LoadedTextureData result;
    result.width = static_cast<uint32_t>(image.width);
    result.height = static_cast<uint32_t>(image.height);

    if (image.component == 4)
    {
        result.rgba = image.image;
    }
    else if (image.component == 3)
    {
        result.rgba = PadRgbToRgba(image.image);
    }
    else
    {
        BA_LOG_WARN("Unsupported baseColorTexture channel count: {}", image.component);
        return {};
    }

    result.hasTexture = true;
    return result;
}

std::vector<Vertex> ComputeFlatNormals(std::vector<Vertex> vertices, const std::vector<uint32_t>& indices)
{
    for (auto& v : vertices)
    {
        v.normal = {0.0f, 0.0f, 0.0f};
    }

    for (size_t i = 0; i < indices.size(); i += 3)
    {
        Vertex& v0 = vertices[indices[i + 0]];
        Vertex& v1 = vertices[indices[i + 1]];
        Vertex& v2 = vertices[indices[i + 2]];

        Vector3 edge1 = v1.position - v0.position;
        Vector3 edge2 = v2.position - v0.position;
        Vector3 faceNormal = edge1.Cross(edge2);

        v0.normal += faceNormal;
        v1.normal += faceNormal;
        v2.normal += faceNormal;
    }

    for (auto& v : vertices)
    {
        v.normal.Normalize();
    }

    return vertices;
}

void ConvertRhToLh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
    for (Vertex& v : vertices)
    {
        v.position.z = -v.position.z;
        v.normal.z   = -v.normal.z;
    }

    BA_ASSERT(indices.size() % 3 == 0);
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        std::swap(indices[i + 1], indices[i + 2]);
    }
}

} // namespace

LoadedMeshData LoadModelFromFile(const std::string& filePath)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool isLoaded = false;

    // Determine file type by extension
    if (filePath.size() >= 4 && filePath.substr(filePath.size() - 4) == ".glb")
    {
        isLoaded = loader.LoadBinaryFromFile(&model, &err, &warn, filePath);
    }
    else
    {
        isLoaded = loader.LoadASCIIFromFile(&model, &err, &warn, filePath);
    }

    if (!warn.empty())
    {
        BA_LOG_WARN("glTF warning: {}", warn);
    }

    if (!isLoaded)
    {
        BA_LOG_ERROR("Failed to load glTF file '{}': {}", filePath, err);
        return {.isLoaded = false};
    }

    ExtractionContext ctx;

    if (model.scenes.empty())
    {
        // glTF library file with no scene metadata — fall back to flat mesh iteration.
        for (const tinygltf::Mesh& mesh : model.meshes)
        {
            ExtractMeshWithTransform(model, mesh, Matrix::Identity, ctx);
        }
    }
    else
    {
        int sceneIndex = (model.defaultScene >= 0) ? model.defaultScene : 0;
        for (int rootNodeIndex : model.scenes[sceneIndex].nodes)
        {
            WalkNode(model, rootNodeIndex, Matrix::Identity, ctx);
        }
    }

    if (ctx.allVertices.empty())
    {
        BA_LOG_ERROR("No valid mesh data found in '{}'", filePath);
        return {.isLoaded = false};
    }

    if (!ctx.hasAnyNormals)
    {
        ctx.allVertices = ComputeFlatNormals(std::move(ctx.allVertices), ctx.allIndices);
    }

    ConvertRhToLh(ctx.allVertices, ctx.allIndices);

    if (ctx.hasMultipleMaterials)
    {
        BA_LOG_WARN("glTF '{}' uses multiple materials; only the first is applied", filePath);
    }

    LoadedTextureData texture = ExtractBaseColorTexture(model, ctx.firstMaterialIndex);

    return {
        .isLoaded = true,
        .vertices = std::move(ctx.allVertices),
        .indices  = std::move(ctx.allIndices),
        .hasTexture = texture.hasTexture,
        .textureRgba8 = std::move(texture.rgba),
        .textureWidth = texture.width,
        .textureHeight = texture.height,
    };
}

} // namespace BA
