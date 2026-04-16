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

void ExtractVertices(const tinygltf::Model& model, const tinygltf::Primitive& primitive,
                     std::vector<Vertex>& outVertices, uint32_t baseVertex)
{
    int vertexCount = GetAttributeCount(model, primitive, "POSITION");
    BA_ASSERT(vertexCount > 0);

    const float* positions = GetAttributeData(model, primitive, "POSITION");
    const float* normals   = GetAttributeData(model, primitive, "NORMAL");
    const float* texcoords = GetAttributeData(model, primitive, "TEXCOORD_0");

    int posStride = GetAttributeByteStride(model, primitive, "POSITION") / sizeof(float);
    int normStride = normals ? GetAttributeByteStride(model, primitive, "NORMAL") / sizeof(float) : 0;
    int uvStride = texcoords ? GetAttributeByteStride(model, primitive, "TEXCOORD_0") / sizeof(float) : 0;

    outVertices.resize(baseVertex + vertexCount);

    for (int i = 0; i < vertexCount; ++i)
    {
        Vertex& v = outVertices[baseVertex + i];

        // Position: negate Z for RH -> LH conversion
        v.position.x =  positions[i * posStride + 0];
        v.position.y =  positions[i * posStride + 1];
        v.position.z = -positions[i * posStride + 2];

        // Normal: negate Z for RH -> LH conversion
        if (normals)
        {
            v.normal.x =  normals[i * normStride + 0];
            v.normal.y =  normals[i * normStride + 1];
            v.normal.z = -normals[i * normStride + 2];
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
}

void ExtractIndices(const tinygltf::Model& model, const tinygltf::Primitive& primitive,
                    std::vector<uint32_t>& outIndices, uint32_t baseVertex, uint32_t baseIndex)
{
    BA_ASSERT(primitive.indices >= 0);

    const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
    const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buf = model.buffers[view.buffer];
    const uint8_t* rawData = &buf.data[view.byteOffset + accessor.byteOffset];

    int indexCount = static_cast<int>(accessor.count);
    outIndices.resize(baseIndex + indexCount);

    // Read indices based on component type
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

        outIndices[baseIndex + i] = index + baseVertex;
    }

    // Reverse winding order for RH -> LH conversion (swap i1 and i2 per triangle)
    BA_ASSERT(indexCount % 3 == 0);
    for (uint32_t i = baseIndex; i < baseIndex + indexCount; i += 3)
    {
        std::swap(outIndices[i + 1], outIndices[i + 2]);
    }
}

void ComputeFlatNormals(std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
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

    std::vector<Vertex> allVertices;
    std::vector<uint32_t> allIndices;

    bool hasAnyNormals = false;

    for (const tinygltf::Mesh& mesh : model.meshes)
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

            uint32_t baseVertex = static_cast<uint32_t>(allVertices.size());
            uint32_t baseIndex = static_cast<uint32_t>(allIndices.size());

            bool hasNormals = primitive.attributes.find("NORMAL") != primitive.attributes.end();
            hasAnyNormals = hasAnyNormals || hasNormals;

            ExtractVertices(model, primitive, allVertices, baseVertex);
            ExtractIndices(model, primitive, allIndices, baseVertex, baseIndex);
        }
    }

    if (allVertices.empty())
    {
        BA_LOG_ERROR("No valid mesh data found in '{}'", filePath);
        return {.isLoaded = false};
    }

    if (!hasAnyNormals)
    {
        ComputeFlatNormals(allVertices, allIndices);
    }

    return {
        .isLoaded = true,
        .vertices = std::move(allVertices),
        .indices  = std::move(allIndices),
    };
}

} // namespace BA
