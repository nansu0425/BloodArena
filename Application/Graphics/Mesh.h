#pragma once

namespace BA
{

struct Mesh
{
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
    uint32_t indexCount = 0;
    bool isIndex32Bit = false;
    std::string textureName;
};

} // namespace BA
