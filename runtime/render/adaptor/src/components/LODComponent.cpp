//
// Created for SkyEngine LOD System
//

#include <render/adaptor/components/LODComponent.h>
#include <render/adaptor/components/StaticMeshComponent.h>
#include <render/adaptor/assets/MeshAsset.h>
#include <render/Renderer.h>
#include <framework/world/World.h>
#include <framework/asset/AssetManager.h>
#include <core/logger/Logger.h>
#include <cmath>

static const char* TAG = "LODComponent";

namespace sky {

    LODComponent::LODComponent()
        : currentLODLevel(0)
        , previousLODLevel(0)
        , lodFadeFactor(1.0f)
        , lodFadeTime(0.0f)
        , enableLOD(true)
        , lodBias(0.0f)
        , forcedLOD(-1)
        , enableCrossfade(true)
        , crossfadeDuration(0.2f)
    {
    }

    LODComponent::~LODComponent() = default;

    void LODComponent::Reflect(SerializationContext* context)
    {
        REGISTER_BEGIN(LODComponent, context)
            REGISTER_MEMBER(baseName)
            REGISTER_MEMBER(levels)
            REGISTER_MEMBER(enableLOD)
            REGISTER_MEMBER(lodBias)
            REGISTER_MEMBER(forcedLOD)
            REGISTER_MEMBER(enableCrossfade)
            REGISTER_MEMBER(crossfadeDuration)
        REGISTER_END()
    }

    void LODComponent::SaveJson(JsonOutputArchive& ar) const
    {
        ar.Save("baseName", lodData.baseName);
        
        ar.StartArray("levels");
        for (const auto& level : lodData.levels) {
            ar.StartObject();
            ar.Save("meshUuid", level.meshUuid.ToString());
            ar.Save("screenPercentage", level.screenPercentage);
            ar.Save("triangleCount", level.triangleCount);
            ar.Save("vertexCount", level.vertexCount);
            ar.Save("boundsCenter", level.boundsCenter);
            ar.Save("boundsRadius", level.boundsRadius);
            ar.EndObject();
        }
        ar.EndArray();
        
        ar.Save("enableLOD", enableLOD);
        ar.Save("lodBias", lodBias);
        ar.Save("forcedLOD", forcedLOD);
        ar.Save("enableCrossfade", enableCrossfade);
        ar.Save("crossfadeDuration", crossfadeDuration);
    }

    void LODComponent::LoadJson(JsonInputArchive& ar)
    {
        ar.Load("baseName", lodData.baseName);
        
        if (ar.HasMember("levels")) {
            size_t numLevels = ar.GetArraySize("levels");
            lodData.levels.resize(numLevels);
            
            for (size_t i = 0; i < numLevels; ++i) {
                ar.EnterObject("levels", i);
                ar.Load("meshUuid", lodData.levels[i].meshUuid);
                ar.Load("screenPercentage", lodData.levels[i].screenPercentage);
                ar.Load("triangleCount", lodData.levels[i].triangleCount);
                ar.Load("vertexCount", lodData.levels[i].vertexCount);
                ar.Load("boundsCenter", lodData.levels[i].boundsCenter);
                ar.Load("boundsRadius", lodData.levels[i].boundsRadius);
                ar.LeaveObject();
            }
        }
        
        ar.Load("enableLOD", enableLOD);
        ar.Load("lodBias", lodBias);
        ar.Load("forcedLOD", forcedLOD);
        ar.Load("enableCrossfade", enableCrossfade);
        ar.Load("crossfadeDuration", crossfadeDuration);
    }

    void LODComponent::AddLODLevel(const Uuid& meshUuid, float screenPercentage)
    {
        LODLevel level;
        level.meshUuid = meshUuid;
        level.screenPercentage = screenPercentage;
        
        // Find insertion point to maintain sorted order
        auto it = std::lower_bound(lodData.levels.begin(), lodData.levels.end(),
            screenPercentage, [](const LODLevel& l, float val) {
                return l.screenPercentage > val;
            });
        
        lodData.levels.insert(it, level);
        
        LOG_I(TAG, "Added LOD level %zu with screen percentage %.3f", 
            lodData.levels.size(), screenPercentage);
    }

    void LODComponent::SetLODLevels(const std::vector<LODLevel>& levels)
    {
        lodData.levels = levels;
        
        // Sort by screen percentage (descending: higher quality first)
        std::sort(lodData.levels.begin(), lodData.levels.end(),
            [](const LODLevel& a, const LODLevel& b) {
                return a.screenPercentage > b.screenPercentage;
            });
        
        currentLODLevel = 0;
        previousLODLevel = 0;
        lodFadeFactor = 1.0f;
        
        LOG_I(TAG, "Set %zu LOD levels", lodData.levels.size());
    }

    void LODComponent::ClearLODLevels()
    {
        lodData.levels.clear();
        currentLODLevel = 0;
        previousLODLevel = 0;
        lodFadeFactor = 1.0f;
    }

    void LODComponent::SetCurrentLevel(uint32_t level)
    {
        if (level >= lodData.levels.size()) {
            LOG_W(TAG, "Invalid LOD level %u (max: %zu)", level, lodData.levels.size());
            return;
        }
        
        if (currentLODLevel != level) {
            previousLODLevel = currentLODLevel;
            currentLODLevel = level;
            lodFadeFactor = 0.0f;
            lodFadeTime = 0.0f;
            
            LOG_I(TAG, "Switching to LOD level %u", level);
            
            UpdateMeshRenderer();
        }
    }

    Uuid LODComponent::GetCurrentMeshUuid() const
    {
        if (currentLODLevel < lodData.levels.size()) {
            return lodData.levels[currentLODLevel].meshUuid;
        }
        return Uuid::Invalid();
    }

    void LODComponent::OnAttachToWorld(World* world)
    {
        // LOD component will be initialized after assets are loaded
    }

    void LODComponent::OnDetachFromWorld()
    {
        // Cleanup if needed
    }

    void LODComponent::OnAssetLoaded()
    {
        // Initialize after assets are loaded
        if (!lodData.levels.empty()) {
            currentLODLevel = 0;
            previousLODLevel = 0;
            lodFadeFactor = 1.0f;
            UpdateMeshRenderer();
        }
    }

    void LODComponent::Tick(float time)
    {
        SKY_PROFILE_NAME("LODComponent Tick");

        if (lodData.levels.empty() || !enableLOD) {
            return;
        }

        // If LOD is forced, skip automatic calculation
        if (forcedLOD >= 0) {
            uint32_t targetLevel = static_cast<uint32_t>(forcedLOD);
            if (targetLevel != currentLODLevel) {
                SetCurrentLevel(targetLevel);
            }
            return;
        }

        // Update crossfade
        if (enableCrossfade && previousLODLevel != currentLODLevel) {
            lodFadeTime += time;
            lodFadeFactor = std::min(lodFadeTime / crossfadeDuration, 1.0f);
            
            if (lodFadeFactor >= 1.0f) {
                previousLODLevel = currentLODLevel;
                lodFadeFactor = 1.0f;
            }
        }

        // Calculate target LOD level
        CalculateLODLevel();
    }

    void LODComponent::CalculateLODLevel()
    {
        if (lodData.levels.empty()) {
            return;
        }

        // Get mesh bounds
        auto* meshComp = GetOwner()->GetComponent<StaticMeshComponent>();
        if (meshComp == nullptr) {
            return;
        }

        // Get the mesh asset to retrieve bounds
        auto meshUuid = GetCurrentMeshUuid();
        if (!meshUuid.IsValid()) {
            return;
        }

        auto* meshAsset = AssetManager::Get()->GetAsset<Mesh>(meshUuid);
        if (meshAsset == nullptr) {
            return;
        }

        // Calculate screen size
        BoundingSphere bounds = meshAsset->GetGeometry()->GetBoundingSphere();
        float screenSize = CalculateScreenSize(bounds);

        // Apply LOD bias
        // Positive bias shifts to lower LODs (smaller screen threshold)
        // Negative bias shifts to higher LODs (larger screen threshold)
        float adjustedScreenSize = screenSize - lodBias;
        adjustedScreenSize = std::clamp(adjustedScreenSize, 0.0f, 1.0f);

        // Find appropriate LOD level
        uint32_t targetLevel = 0;
        for (uint32_t i = 0; i < lodData.levels.size(); ++i) {
            if (adjustedScreenSize > lodData.levels[i].screenPercentage) {
                targetLevel = i;
                break;
            }
        }

        // Update LOD level if changed
        if (targetLevel != currentLODLevel) {
            SetCurrentLevel(targetLevel);
        }
    }

    float LODComponent::CalculateScreenSize(const BoundingSphere& bounds) const
    {
        auto* world = GetOwner()->GetWorld();
        if (world == nullptr) {
            return 1.0f; // Assume highest quality if no world
        }

        // Get camera position
        Vector3 cameraPos = GetCameraPosition();
        
        // Calculate distance from camera to object center
        Vector3 toObject = bounds.center - cameraPos;
        float distance = toObject.Length();
        
        // Avoid division by zero
        distance = std::max(distance, 0.01f);
        
        // Calculate screen size
        // Screen size = (2 * radius * tan(FOV/2)) / distance
        // Using a simplified model for now
        float screenHeight = Renderer::Get()->GetCameraHeight();
        if (screenHeight <= 0.0f) {
            screenHeight = 1.0f;
        }
        
        float screenSize = (2.0f * bounds.radius) / (distance * screenHeight);
        screenSize = std::clamp(screenSize, 0.0f, 1.0f);
        
        return screenSize;
    }

    Vector3 LODComponent::GetCameraPosition() const
    {
        auto* world = GetOwner()->GetWorld();
        if (world == nullptr) {
            return Vector3(0.0f, 0.0f, 0.0f);
        }

        // Get main camera
        auto* cameraActor = world->GetMainCamera();
        if (cameraActor == nullptr) {
            return Vector3(0.0f, 0.0f, 0.0f);
        }

        return cameraActor->GetTransform().translation;
    }

    void LODComponent::UpdateMeshRenderer()
    {
        auto* meshComp = GetOwner()->GetComponent<StaticMeshComponent>();
        if (meshComp == nullptr) {
            LOG_W(TAG, "No StaticMeshComponent found for LOD update");
            return;
        }

        // Update mesh UUID based on current LOD level
        Uuid meshUuid = GetCurrentMeshUuid();
        if (meshUuid.IsValid()) {
            meshComp->SetMesh(meshUuid);
        }
    }

} // namespace sky
