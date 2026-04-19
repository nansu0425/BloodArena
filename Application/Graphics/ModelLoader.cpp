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

Matrix ConvertNodeTransformRhToLh(Matrix m)
{
    // Z-axis reflection F = diag(1,1,-1,1). Applied as M_LH = F * M_RH * F, which
    // flips the four Z off-diagonals of the rotation/scale basis plus the Z translation.
    // The Z-diagonal (_33) and non-Z translation components stay as-is.
    m._13 = -m._13;
    m._23 = -m._23;
    m._31 = -m._31;
    m._32 = -m._32;
    m._43 = -m._43;
    return m;
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

        // Raw glTF (right-handed) values; ConvertRhToLh flips Z after normals are resolved.
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

std::vector<uint32_t> ExtractIndices(const tinygltf::Model& model, const tinygltf::Primitive& primitive)
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

        indices[i] = index;
    }

    return indices;
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

bool IsExtractablePrimitive(const tinygltf::Primitive& primitive)
{
    if (primitive.mode != TINYGLTF_MODE_TRIANGLES && primitive.mode != -1)
    {
        BA_LOG_WARN("Skipping non-triangle primitive (mode={})", primitive.mode);
        return false;
    }

    if (primitive.attributes.find("POSITION") == primitive.attributes.end())
    {
        BA_LOG_WARN("Skipping primitive without POSITION attribute");
        return false;
    }

    if (primitive.indices < 0)
    {
        BA_LOG_WARN("Skipping non-indexed primitive");
        return false;
    }

    return true;
}

LoadedPrimitiveData ExtractPrimitive(const tinygltf::Model& model, const tinygltf::Primitive& primitive)
{
    LoadedPrimitiveData result;
    result.materialIndex = primitive.material;

    result.vertices = ExtractVertices(model, primitive);
    result.indices  = ExtractIndices(model, primitive);

    bool hasNormals = primitive.attributes.find("NORMAL") != primitive.attributes.end();
    if (!hasNormals)
    {
        result.vertices = ComputeFlatNormals(std::move(result.vertices), result.indices);
    }

    ConvertRhToLh(result.vertices, result.indices);
    return result;
}

std::vector<uint8_t> PadRgbToRgba(const std::vector<uint8_t>& rgb)
{
    BA_ASSERT(rgb.size() % 3 == 0);

    size_t pixelCount = rgb.size() / 3;
    std::vector<uint8_t> rgba(pixelCount * 4);

    constexpr uint8_t kOpaqueAlpha = 255;
    for (size_t i = 0; i < pixelCount; ++i)
    {
        rgba[i * 4 + 0] = rgb[i * 3 + 0];
        rgba[i * 4 + 1] = rgb[i * 3 + 1];
        rgba[i * 4 + 2] = rgb[i * 3 + 2];
        rgba[i * 4 + 3] = kOpaqueAlpha;
    }

    return rgba;
}

LoadedMaterialData ExtractMaterial(const tinygltf::Model& model, const tinygltf::Material& material)
{
    LoadedMaterialData result;

    constexpr size_t kBaseColorFactorComponentCount = 4;
    const std::vector<double>& factor = material.pbrMetallicRoughness.baseColorFactor;
    if (factor.size() == kBaseColorFactorComponentCount)
    {
        for (size_t i = 0; i < kBaseColorFactorComponentCount; ++i)
        {
            result.baseColorFactor[i] = static_cast<float>(factor[i]);
        }
    }

    int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
    if (textureIndex < 0 || textureIndex >= static_cast<int>(model.textures.size()))
    {
        return result;
    }

    int imageIndex = model.textures[textureIndex].source;
    if (imageIndex < 0 || imageIndex >= static_cast<int>(model.images.size()))
    {
        return result;
    }

    const tinygltf::Image& image = model.images[imageIndex];
    if (image.image.empty() || image.width <= 0 || image.height <= 0)
    {
        BA_LOG_WARN("baseColorTexture image is empty or invalid");
        return result;
    }

    result.diffuseWidth = static_cast<uint32_t>(image.width);
    result.diffuseHeight = static_cast<uint32_t>(image.height);

    constexpr int kRgbChannelCount = 3;
    constexpr int kRgbaChannelCount = 4;
    if (image.component == kRgbaChannelCount)
    {
        result.diffuseRgba8 = image.image;
    }
    else if (image.component == kRgbChannelCount)
    {
        result.diffuseRgba8 = PadRgbToRgba(image.image);
    }
    else
    {
        BA_LOG_WARN("Unsupported baseColorTexture channel count: {}", image.component);
        return result;
    }

    result.hasDiffuseTexture = true;
    return result;
}

} // namespace

LoadedModelData LoadModelFromFile(const std::string& filePath)
{
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool isLoaded = false;

    constexpr std::string_view kGlbExtension = ".glb";
    if (filePath.size() >= kGlbExtension.size()
        && filePath.compare(filePath.size() - kGlbExtension.size(), kGlbExtension.size(), kGlbExtension) == 0)
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

    LoadedModelData out;
    out.isLoaded = true;

    out.materials.reserve(model.materials.size());
    for (const tinygltf::Material& material : model.materials)
    {
        out.materials.push_back(ExtractMaterial(model, material));
    }

    out.meshes.reserve(model.meshes.size());
    for (const tinygltf::Mesh& mesh : model.meshes)
    {
        LoadedMeshData meshOut;
        for (const tinygltf::Primitive& primitive : mesh.primitives)
        {
            if (!IsExtractablePrimitive(primitive))
            {
                continue;
            }
            meshOut.primitives.push_back(ExtractPrimitive(model, primitive));
        }
        out.meshes.push_back(std::move(meshOut));
    }

    out.nodes.reserve(model.nodes.size());
    for (const tinygltf::Node& node : model.nodes)
    {
        LoadedNodeData nodeOut;
        nodeOut.localTransform = ConvertNodeTransformRhToLh(ComputeNodeLocalTransform(node));
        nodeOut.meshIndex = node.mesh;
        nodeOut.childIndices.assign(node.children.begin(), node.children.end());
        out.nodes.push_back(std::move(nodeOut));
    }

    if (!model.scenes.empty())
    {
        int sceneIndex = (model.defaultScene >= 0) ? model.defaultScene : 0;
        out.rootNodeIndices.assign(
            model.scenes[sceneIndex].nodes.begin(),
            model.scenes[sceneIndex].nodes.end());
    }
    else
    {
        // Library-style glTF file with no scene metadata: wrap each mesh in a synthetic identity node.
        for (size_t meshIdx = 0; meshIdx < out.meshes.size(); ++meshIdx)
        {
            LoadedNodeData nodeOut;
            nodeOut.meshIndex = static_cast<int>(meshIdx);
            out.nodes.push_back(std::move(nodeOut));
            out.rootNodeIndices.push_back(static_cast<int>(out.nodes.size() - 1));
        }
    }

    if (out.meshes.empty() || out.rootNodeIndices.empty())
    {
        BA_LOG_ERROR("No valid mesh data found in '{}'", filePath);
        return {.isLoaded = false};
    }

    return out;
}

} // namespace BA
