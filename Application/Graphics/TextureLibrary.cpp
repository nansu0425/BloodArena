#include "Core/PCH.h"
#include "Graphics/TextureLibrary.h"
#include "Graphics/GraphicsDevice.h"

namespace BA
{

namespace
{

constexpr const char* kDefaultTextureName = "white";

} // namespace

void TextureLibrary::Initialize()
{
    CreateDefaultTexture();

    BA_LOG_INFO("TextureLibrary initialized.");
}

void TextureLibrary::Shutdown()
{
    m_textures.clear();

    BA_LOG_INFO("TextureLibrary shutdown.");
}

const Texture* TextureLibrary::FindTexture(const std::string& name) const
{
    auto it = m_textures.find(name);
    if (it == m_textures.end())
    {
        return nullptr;
    }

    return &it->second;
}

const Texture* TextureLibrary::GetDefaultTexture() const
{
    return FindTexture(kDefaultTextureName);
}

void TextureLibrary::RegisterTexture(const std::string& name, const uint8_t* pixelsRgba8, uint32_t width, uint32_t height)
{
    if (m_textures.contains(name))
    {
        BA_LOG_WARN("Texture '{}' already registered, skipping", name);
        return;
    }

    BA_ASSERT(pixelsRgba8);
    BA_ASSERT(width > 0 && height > 0);

    Texture texture;
    texture.srv = g_graphicsDevice->CreateTextureRgba8SRV(pixelsRgba8, width, height);

    m_textures[name] = std::move(texture);
    BA_LOG_INFO("Registered texture '{}' ({}x{})", name, width, height);
}

void TextureLibrary::CreateDefaultTexture()
{
    constexpr uint8_t kWhitePixel[4] = {255, 255, 255, 255};

    Texture texture;
    texture.srv = g_graphicsDevice->CreateTextureRgba8SRV(kWhitePixel, 1, 1);

    m_textures[kDefaultTextureName] = std::move(texture);
}

std::unique_ptr<TextureLibrary> g_textureLibrary;

} // namespace BA
