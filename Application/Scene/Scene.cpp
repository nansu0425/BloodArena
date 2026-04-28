#include "Core/PCH.h"
#include "Scene/Scene.h"
#include "Scene/CameraComponent.h"
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

constexpr uint32_t kSceneSchemaVersion = 9;

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

json WriteQuaternion(const Quaternion& q)
{
    return json::array({q.x, q.y, q.z, q.w});
}

Quaternion ReadQuaternion(const json& j, const Quaternion& fallback)
{
    if (!j.is_array() || j.size() != 4)
    {
        return fallback;
    }
    return Quaternion{
        j[0].get<float>(),
        j[1].get<float>(),
        j[2].get<float>(),
        j[3].get<float>()
    };
}

json WriteTransform(const Transform& t)
{
    return json{
        {"position", WriteVector3(t.position)},
        {"rotation", WriteQuaternion(t.rotation)},
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
    t.rotation = ReadQuaternion(j.value("rotation", json{}), t.rotation);
    t.scale    = ReadVector3(j.value("scale",    json{}), t.scale);
    return t;
}

json WriteLightComponent(const LightComponent& light)
{
    json result;
    result["isEnabled"] = light.IsEnabled();
    result["color"]     = WriteVector3(light.GetColor());
    result["intensity"] = light.GetIntensity();
    switch (light.GetType())
    {
        case LightType::Directional:
        {
            result["type"]                  = "directional";
            result["specularStrength"]      = light.GetSpecularStrength();
            result["shininess"]             = light.GetShininess();
            result["shouldCastShadow"]      = light.ShouldCastShadow();
            result["shadowOrthoWidth"]      = light.GetShadowOrthoWidth();
            result["shadowOrthoHeight"]     = light.GetShadowOrthoHeight();
            result["shadowNearZ"]           = light.GetShadowNearZ();
            result["shadowFarZ"]            = light.GetShadowFarZ();
            result["shadowDepthBias"]       = light.GetShadowDepthBias();
            result["shadowFrustumCenter"]     = WriteVector3(light.GetShadowFrustumCenter());
            result["isShadowFrustumVisualized"] = light.IsShadowFrustumVisualized();
            result["isShadowFrustumAutoFit"]    = light.IsShadowFrustumAutoFit();
            break;
        }
        case LightType::Ambient:
        {
            result["type"] = "ambient";
            break;
        }
    }
    return result;
}

LightComponent ReadLightComponent(const json& j)
{
    LightComponent light;
    if (!j.is_object())
    {
        return light;
    }

    std::string typeStr = j.value("type", std::string{"directional"});
    if (typeStr == "directional")
    {
        light.SetType(LightType::Directional);
    }
    else if (typeStr == "ambient")
    {
        light.SetType(LightType::Ambient);
    }

    light.SetEnabled(j.value("isEnabled", true));
    light.SetColor(ReadVector3(j.value("color", json{}), light.GetColor()));
    light.SetIntensity(j.value("intensity", light.GetIntensity()));
    if (light.GetType() == LightType::Directional)
    {
        light.SetSpecularStrength(j.value("specularStrength", light.GetSpecularStrength()));
        light.SetShininess(j.value("shininess", light.GetShininess()));
        light.SetShouldCastShadow(j.value("shouldCastShadow", light.ShouldCastShadow()));
        light.SetShadowOrthoWidth(j.value("shadowOrthoWidth", light.GetShadowOrthoWidth()));
        light.SetShadowOrthoHeight(j.value("shadowOrthoHeight", light.GetShadowOrthoHeight()));
        light.SetShadowNearZ(j.value("shadowNearZ", light.GetShadowNearZ()));
        light.SetShadowFarZ(j.value("shadowFarZ", light.GetShadowFarZ()));
        light.SetShadowDepthBias(j.value("shadowDepthBias", light.GetShadowDepthBias()));
        light.SetShadowFrustumCenter(
            ReadVector3(j.value("shadowFrustumCenter", json{}), light.GetShadowFrustumCenter()));
        light.SetShadowFrustumVisualized(
            j.value("isShadowFrustumVisualized", light.IsShadowFrustumVisualized()));
        light.SetShadowFrustumAutoFit(
            j.value("isShadowFrustumAutoFit", light.IsShadowFrustumAutoFit()));
    }
    return light;
}

json WriteCameraComponent(const CameraComponent& camera)
{
    return json{
        {"isEnabled",               camera.IsEnabled()},
        {"fovY",                    camera.GetFovY()},
        {"nearZ",                   camera.GetNearZ()},
        {"farZ",                    camera.GetFarZ()},
        {"isViewFrustumVisualized", camera.IsViewFrustumVisualized()},
    };
}

CameraComponent ReadCameraComponent(const json& j)
{
    CameraComponent camera;
    if (!j.is_object())
    {
        return camera;
    }

    camera.SetEnabled(j.value("isEnabled", true));
    camera.SetFovY(j.value("fovY", camera.GetFovY()));
    camera.SetNearZ(j.value("nearZ", camera.GetNearZ()));
    camera.SetFarZ(j.value("farZ", camera.GetFarZ()));
    camera.SetViewFrustumVisualized(
        j.value("isViewFrustumVisualized", camera.IsViewFrustumVisualized()));

    return camera;
}

json WriteGameObject(const GameObject& obj)
{
    json result{
        {"id", obj.GetId()},
        {"name", obj.GetName()},
        {"transform", WriteTransform(obj.GetTransform())}
    };
    if (const auto* mc = obj.GetComponent<ModelComponent>())
    {
        result["modelComponent"] = json{
            {"modelName", mc->GetModelName()},
            {"isEnabled", mc->IsEnabled()}
        };
    }
    if (const auto* lc = obj.GetComponent<LightComponent>())
    {
        result["lightComponent"] = WriteLightComponent(*lc);
    }
    if (const auto* cc = obj.GetComponent<CameraComponent>())
    {
        result["cameraComponent"] = WriteCameraComponent(*cc);
    }
    return result;
}

GameObject ReadGameObject(const json& j)
{
    GameObject obj;
    obj.SetId(j.value("id", obj.GetId()));
    obj.SetName(j.value("name", std::string{}));

    if (j.contains("transform"))
    {
        obj.SetTransform(ReadTransform(j["transform"]));
    }

    if (j.contains("modelComponent") && j["modelComponent"].is_object())
    {
        const json& mc = j["modelComponent"];
        std::string name = mc.value("modelName", std::string{});
        BA_ASSERT(!name.empty());
        BA_ASSERT(g_modelLibrary->FindModel(name));
        ModelComponent& component = obj.AddComponent<ModelComponent>(std::move(name));
        component.SetEnabled(mc.value("isEnabled", true));
    }

    if (j.contains("lightComponent") && j["lightComponent"].is_object())
    {
        obj.AddComponent<LightComponent>(ReadLightComponent(j["lightComponent"]));
    }

    if (j.contains("cameraComponent") && j["cameraComponent"].is_object())
    {
        obj.AddComponent<CameraComponent>(ReadCameraComponent(j["cameraComponent"]));
    }

    return obj;
}

} // namespace

void Scene::Initialize()
{
    LoadFromFile("default");
    BA_LOG_INFO("Scene initialized.");
}

void Scene::Shutdown()
{
    m_gameObjects.clear();
    BA_LOG_INFO("Scene shutdown.");
}

void Scene::Tick(float /*deltaSeconds*/)
{
}

uint32_t Scene::CreateGameObject()
{
    uint32_t id = m_nextId++;

    GameObject gameObject;
    gameObject.SetId(id);
    gameObject.SetName("GameObject " + std::to_string(id));

    m_gameObjects.push_back(std::move(gameObject));

    BA_LOG_INFO("Created GameObject (ID: {})", id);
    return id;
}

void Scene::DestroyGameObject(uint32_t id)
{
    auto it = std::find_if(
        m_gameObjects.begin(),
        m_gameObjects.end(),
        [id](const GameObject& obj) { return (obj.GetId() == id); }
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

std::span<GameObject> Scene::GetGameObjects()
{
    return m_gameObjects;
}

GameObject* Scene::FindGameObject(uint32_t id)
{
    auto it = std::find_if(
        m_gameObjects.begin(),
        m_gameObjects.end(),
        [id](const GameObject& obj) { return (obj.GetId() == id); }
    );

    return (it != m_gameObjects.end()) ? &(*it) : nullptr;
}

ActiveCameraResult Scene::FindActiveCamera()
{
    for (GameObject& gameObject : m_gameObjects)
    {
        CameraComponent* candidate = gameObject.GetComponent<CameraComponent>();
        if (!candidate || !candidate->IsEnabled())
        {
            continue;
        }
        return ActiveCameraResult{ true, &gameObject, candidate };
    }

    return ActiveCameraResult{ false, nullptr, nullptr };
}

std::string Scene::SerializeToString() const
{
    BA_PROFILE_SCOPE("Scene::SerializeToString");

    json j;
    j["version"] = kSceneSchemaVersion;
    j["nextId"] = m_nextId;

    json objects = json::array();
    for (const GameObject& obj : m_gameObjects)
    {
        objects.push_back(WriteGameObject(obj));
    }
    j["gameObjects"] = std::move(objects);

    return j.dump(4);
}

bool Scene::DeserializeFromString(const std::string& jsonText)
{
    BA_PROFILE_SCOPE("Scene::DeserializeFromString");

    json j = json::parse(jsonText, nullptr, false);
    if (j.is_discarded())
    {
        BA_LOG_WARN("Failed to parse scene JSON.");
        return false;
    }

    uint32_t fileVersion = j.value("version", static_cast<uint32_t>(0));
    if (fileVersion != kSceneSchemaVersion)
    {
        BA_LOG_WARN(
            "Scene schema version mismatch (data: {}, expected: {}).",
            fileVersion, kSceneSchemaVersion
        );
        return false;
    }

    m_gameObjects.clear();
    m_nextId = j.value("nextId", static_cast<uint32_t>(1));

    if (j.contains("gameObjects") && j["gameObjects"].is_array())
    {
        for (const json& entry : j["gameObjects"])
        {
            m_gameObjects.push_back(ReadGameObject(entry));
        }
    }

    return true;
}

bool Scene::SaveToFile(const std::string& name) const
{
    std::filesystem::path filePath = GetScenePath(name);

    std::error_code ec;
    std::filesystem::create_directories(filePath.parent_path(), ec);

    std::ofstream file(filePath);
    if (!file.is_open())
    {
        BA_LOG_WARN("Failed to open scene file for writing: {}", filePath.string());
        return false;
    }

    file << SerializeToString();
    BA_LOG_INFO("Scene saved to: {}", filePath.string());
    return true;
}

bool Scene::LoadFromFile(const std::string& name)
{
    BA_PROFILE_SCOPE("Scene::LoadFromFile");

    std::filesystem::path filePath = GetScenePath(name);

    std::ifstream file(filePath);
    if (!file.is_open())
    {
        BA_LOG_WARN("Scene file not found: {}", filePath.string());
        return false;
    }

    std::string content(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    if (!DeserializeFromString(content))
    {
        BA_LOG_WARN("Failed to deserialize scene from: {}", filePath.string());
        return false;
    }

    BA_LOG_INFO("Scene loaded from: {}", filePath.string());
    return true;
}

std::unique_ptr<Scene> g_scene;

} // namespace BA
