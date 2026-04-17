#pragma once

#include "Graphics/Texture.h"

namespace BA
{

class TextureLibrary
{
public:
    void Initialize();
    void Shutdown();

    const Texture* FindTexture(const std::string& name) const;
    const Texture* GetDefaultTexture() const;
    void RegisterTexture(const std::string& name, const uint8_t* pixelsRgba8, uint32_t width, uint32_t height);

private:
    void CreateDefaultTexture();

    std::unordered_map<std::string, Texture> m_textures;
};

extern std::unique_ptr<TextureLibrary> g_textureLibrary;

} // namespace BA
