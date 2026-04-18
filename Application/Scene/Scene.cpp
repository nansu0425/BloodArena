#include "Core/PCH.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"
#include "Graphics/ModelLibrary.h"
#include "Core/PathUtils.h"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

namespace BA
{

namespace
{

using json = nlohmann::json;

constexpr float kPalette[][4] =
{
    {1.0f, 0.2f, 0.2f, 1.0f},
    {0.2f, 1.0f, 0.2f, 1.0f},
    {0.2f, 0.4f, 1.0f, 1.0f},
    {1.0f, 1.0f, 0.2f, 1.0f},
    {1.0f, 0.2f, 1.0f, 1.0f},
    {0.2f, 1.0f, 1.0f, 1.0f},
    {1.0f, 0.6f, 0.2f, 1.0f},
    {0.6f, 0.2f, 1.0f, 1.0f},
};

constexpr uint32_t kPaletteSize = _countof(kPalette);
constexpr uint32_t kGridColumns = 5;
constexpr float kGridStartX = -4.0f;
constexpr float kGridStartY = 3.0f;
constexpr float kGridSpacing = 2.0f;

constexpr uint32_t kSceneSchemaVersion = 2;

std::filesystem::path GetScenePath(const std::string& name)
{
    std::wstring wideRelativeDir = L"Assets/Scenes/";
    std::wstring resolvedDir = ResolveAssetPath(wideRelativeDir.c_str());
    return std::filesystem::path(resolvedDir) / (name + ".json");
}

json WriteVector3(const Vector3& v)
{
    return json::array({v.x, v.y, v.z});
}

Vector3 ReadVector3(const json& j, const Vector3& fallback)
{
    if (!j.is_array() || j.size() != 3)
    {
        return fallback;
    }
    return Vector3{
        j[0].get<float>(),
        j[1].get<float>(),
        j[2].get<float>()
    };
}

json WriteTransform(const Transform& t)
{
    return json{
        {"position", WriteVector3(t.position)},
        {"rotation", WriteVector3(t.rotation)},
        {"scale",    WriteVector3(t.scale)}
    };
}

Transform ReadTransform(const json& j)
{
    Transform t;
    if (!j.is_object())
    {
        return t;
    }
    t.position = ReadVector3(j.value("position", json{}), t.position);
    t.rotation = ReadVector3(j.value("rotation", json{}), t.rotation);
    t.scale    = ReadVector3(j.value("scale",    json{}), t.scale);
    return t;
}

json WriteCamera(const CameraSettings& s)
{
    return json{
        {"positionX", s.position.x},
        {"positionY", s.position.y},
        {"positionZ", s.position.z},
        {"yaw", s.yaw},
        {"pitch", s.pitch},
        {"fovY", s.fovY},
        {"nearZ", s.nearZ},
        {"farZ", s.farZ},
        {"moveSpeed", s.moveSpeed},
        {"mouseSensitivity", s.mouseSensitivity}
    };
}

CameraSettings ReadCamera(const json& j)
{
    CameraSettings s;
    if (!j.is_object())
    {
        return s;
    }
    s.position.x = j.value("positionX", s.position.x);
    s.position.y = j.value("positionY", s.position.y);
    s.position.z = j.value("positionZ", s.position.z);
    s.yaw = j.value("yaw", s.yaw);
    s.pitch = j.value("pitch", s.pitch);
    s.fovY = j.value("fovY", s.fovY);
    s.nearZ = j.value("nearZ", s.nearZ);
    s.farZ = j.value("farZ", s.farZ);
    s.moveSpeed = j.value("moveSpeed", s.moveSpeed);
    s.mouseSensitivity = j.value("mouseSensitivity", s.mouseSensitivity);
    return s;
}

json WriteGameObject(const GameObject& obj)
{
    json result{
        {"id", obj.id},
        {"color", json::array({obj.color[0], obj.color[1], obj.color[2], obj.color[3]})},
        {"transform", WriteTransform(obj.transform)}
    };
    if (obj.modelComponent)
    {
        result["modelComponent"] = json{{"modelName", obj.modelComponent->modelName}};
    }
    return result;
}

GameObject ReadGameObject(const json& j)
{
    GameObject obj;
    obj.id = j.value("id", obj.id);

    if (j.contains("color") && j["color"].is_array() && j["color"].size() == 4)
    {
        const json& c = j["color"];
        obj.color[0] = c[0].get<float>();
        obj.color[1] = c[1].get<float>();
        obj.color[2] = c[2].get<float>();
        obj.color[3] = c[3].get<float>();
    }

    if (j.contains("transform"))
    {
        obj.transform = ReadTransform(j["transform"]);
    }

    if (j.contains("modelComponent") && j["modelComponent"].is_object())
    {
        const json& mc = j["modelComponent"];
        std::string name = mc.value("modelName", std::string{});
        BA_ASSERT(!name.empty());
        BA_ASSERT(g_modelLibrary->FindModel(name));
        obj.modelComponent = std::make_unique<ModelComponent>(ModelComponent{std::move(name)});
    }

    return obj;
}

} // namespace

void Scene::Initialize()
{
    BA_LOG_INFO("Scene initialized.");
}

void Scene::Shutdown()
{
    m_gameObjects.clear();
    BA_LOG_INFO("Scene shutdown.");
}

uint32_t Scene::CreateGameObject()
{
    uint32_t id = m_nextId++;
    uint32_t index = static_cast<uint32_t>(m_gameObjects.size());

    GameObject gameObject;
    gameObject.id = id;

    const float* palette = kPalette[index % kPaletteSize];
    gameObject.color[0] = palette[0];
    gameObject.color[1] = palette[1];
    gameObject.color[2] = palette[2];
    gameObject.color[3] = palette[3];

    gameObject.transform.position.x = kGridStartX + static_cast<float>(index % kGridColumns) * kGridSpacing;
    gameObject.transform.position.y = kGridStartY - static_cast<float>(index / kGridColumns) * kGridSpacing;

    m_gameObjects.push_back(std::move(gameObject));

    BA_LOG_INFO("Created GameObject (ID: {})", id);
    return id;
}

void Scene::DestroyGameObject(uint32_t id)
{
    auto it = std::find_if(
        m_gameObjects.begin(),
        m_gameObjects.end(),
        [id](const GameObject& obj) { return (obj.id == id); }
    );

    BA_ASSERT(it != m_gameObjects.end());

    m_gameObjects.erase(it);
    BA_LOG_INFO("Destroyed GameObject (ID: {})", id);
}

void Scene::Clear()
{
    m_gameObjects.clear();
    m_nextId = 1;
    BA_LOG_INFO("Scene cleared.");
}

std::span<const GameObject> Scene::GetGameObjects() const
{
    return m_gameObjects;
}

GameObject* Scene::FindGameObject(uint32_t id)
{
    auto it = std::find_if(
        m_gameObjects.begin(),
        m_gameObjects.end(),
        [id](const GameObject& obj) { return (obj.id == id); }
    );

    return (it != m_gameObjects.end()) ? &(*it) : nullptr;
}

bool Scene::SaveToFile(const std::string& name) const
{
    BA_ASSERT(g_camera);

    std::filesystem::path filePath = GetScenePath(name);

    std::error_code ec;
    std::filesystem::create_directories(filePath.parent_path(), ec);

    json j;
    j["version"] = kSceneSchemaVersion;
    j["nextId"] = m_nextId;
    j["camera"] = WriteCamera(g_camera->GetSettings());

    json objects = json::array();
    for (const GameObject& obj : m_gameObjects)
    {
        objects.push_back(WriteGameObject(obj));
    }
    j["gameObjects"] = std::move(objects);

    std::ofstream file(filePath);
    if (!file.is_open())
    {
        BA_LOG_WARN("Failed to open scene file for writing: {}", filePath.string());
        return false;
    }

    file << j.dump(4);
    BA_LOG_INFO("Scene saved to: {}", filePath.string());
    return true;
}

bool Scene::LoadFromFile(const std::string& name)
{
    BA_ASSERT(g_camera);

    std::filesystem::path filePath = GetScenePath(name);

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        BA_LOG_WARN("Scene file not found: {}", filePath.string());
        return false;
    }

    json j = json::parse(file, nullptr, false);
    if (j.is_discarded())
    {
        BA_LOG_WARN("Failed to parse scene file: {}", filePath.string());
        return false;
    }

    m_gameObjects.clear();
    m_nextId = j.value("nextId", static_cast<uint32_t>(1));

    if (j.contains("camera"))
    {
        g_camera->SetSettings(ReadCamera(j["camera"]));
    }

    if (j.contains("gameObjects") && j["gameObjects"].is_array())
    {
        for (const json& entry : j["gameObjects"])
        {
            m_gameObjects.push_back(ReadGameObject(entry));
        }
    }

    BA_LOG_INFO("Scene loaded from: {}", filePath.string());
    return true;
}

std::unique_ptr<Scene> g_scene;

} // namespace BA
