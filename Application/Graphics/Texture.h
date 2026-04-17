#pragma once

namespace BA
{

struct Texture
{
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
};

} // namespace BA
