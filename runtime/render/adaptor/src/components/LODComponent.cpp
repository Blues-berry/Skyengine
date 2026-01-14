//
// Created by AI on 2025/1/14.
// LOD Component Implementation
//

#include <render/adaptor/components/LODComponent.h>
#include <framework/asset/AssetManager.h>
#include <framework/serialization/JsonArchive.h>
#include <framework/serialization/PropertyCommon.h>
#include <framework/world/Actor.h>
#include <framework/world/TransformComponent.h>
#include <core/profile/Profiler.h>
#include <render/adaptor/Util.h>
#include <render/mesh/MeshFeatureProcessor.h>
#include <render/Renderer.h>
#include <render/resource/Camera.h>

namespace sky {

    LODComponent::LODComponent()
    {
        objectBounds.center = {0, 0, 0};
        objectBounds.radius = 1.0f;
    }

    LODComponent::~LODComponent()
    {
        // 清理所有LOD渲染器
        for (auto &[level, lod] : lodRenderers) {
            if (lod.renderer != nullptr && actor != nullptr) {
                auto *mf = GetFeatureProcessor<MeshFeatureProcessor>(actor);
                if (mf != nullptr) {
                    mf->RemoveStaticMesh(lod.renderer);
                }
                lod.renderer = nullptr;
            }
            lod.meshInstance = nullptr;
        }
        lodRenderers.clear();
    }

    void LODComponent::Reflect(SerializationContext *context)
    {
        context->Register<LODGroupData>("LODGroupData")
            .Member<&LODGroupData::enableLOD>("EnableLOD")
            .Member<&LODGroupData::forceLOD>("ForceLOD")
            .Member<&LODGroupData::forcedLevel>("ForcedLevel")
            .Member<&LODGroupData::lodBias>("LODBias")
            .Member<&LODGroupData::crossFadeDuration>("CrossFadeDuration")
            .Member<&LODGroupData::levels>("Levels");

        REGISTER_BEGIN(LODComponent, context)
            REGISTER_MEMBER(EnableLOD, SetEnableLOD, GetEnableLOD)
            REGISTER_MEMBER(ForceLOD, SetForceLOD, GetForceLOD)
            REGISTER_MEMBER(ForcedLevel, SetForcedLevel, GetForcedLevel)
            REGISTER_MEMBER(LODBias, SetLODBias, SetLODBias)
            REGISTER_MEMBER(CrossFadeDuration, SetCrossFadeDuration, GetCrossFadeDuration);
    }

    void LODComponent::SaveJson(JsonOutputArchive &ar) const
    {
        ar.StartObject();
        ar.SaveValueObject(std::string("enableLOD"), data.enableLOD);
        ar.SaveValueObject(std::string("forceLOD"), data.forceLOD);
        ar.SaveValueObject(std::string("forcedLevel"), data.forcedLevel);
        ar.SaveValueObject(std::string("lodBias"), data.lodBias);
        ar.SaveValueObject(std::string("crossFadeDuration"), data.crossFadeDuration);
        
        // 保存LOD级别
        ar.StartArray(std::string("levels"));
        for (const auto &level : data.levels) {
            ar.StartObject();
            ar.SaveValueObject(std::string("meshUuid"), level.meshUuid);
            ar.SaveValueObject(std::string("screenPercentage"), level.screenPercentage);
            ar.SaveValueObject(std::string("triangleCount"), level.triangleCount);
            ar.SaveValueObject(std::string("vertexCount"), level.vertexCount);
            ar.SaveValueObject(std::string("boundsCenter"), level.bounds.center);
            ar.SaveValueObject(std::string("boundsRadius"), level.bounds.radius);
            ar.EndObject();
        }
        ar.EndArray();
        
        ar.EndObject();
    }

    void LODComponent::LoadJson(JsonInputArchive &ar)
    {
        ar.LoadKeyValue("enableLOD", data.enableLOD);
        ar.LoadKeyValue("forceLOD", data.forceLOD);
        ar.LoadKeyValue("forcedLevel", data.forcedLevel);
        ar.LoadKeyValue("lodBias", data.lodBias);
        ar.LoadKeyValue("crossFadeDuration", data.crossFadeDuration);
        
        // 加载LOD级别
        data.levels.clear();
        if (ar.HasValue("levels")) {
            size_t levelCount = ar.GetArraySize("levels");
            data.levels.reserve(levelCount);
            for (size_t i = 0; i < levelCount; ++i) {
                LODLevel level;
                ar.EnterObject("levels", i);
                ar.LoadKeyValue("meshUuid", level.meshUuid);
                ar.LoadKeyValue("screenPercentage", level.screenPercentage);
                ar.LoadKeyValue("triangleCount", level.triangleCount);
                ar.LoadKeyValue("vertexCount", level.vertexCount);
                ar.LoadKeyValue("boundsCenter", level.bounds.center);
                ar.LoadKeyValue("boundsRadius", level.bounds.radius);
                ar.ExitObject();
                data.levels.push_back(level);
            }
        }
    }

    void LODComponent::AddLODLevel(const Uuid &meshUuid, float screenPercentage)
    {
        LODLevel level;
        level.meshUuid = meshUuid;
        level.screenPercentage = screenPercentage;
        level.triangleCount = 0;
        level.vertexCount = 0;
        level.bounds = {Vector3(0, 0, 0), 1.0f};
        data.levels.push_back(level);

        // 加载新LOD级别的网格
        if (meshUuid) {
            binder.Bind(this, meshUuid);
            loadingMeshes.push_back(meshUuid);
            auto meshAsset = AssetManager::Get()->LoadAsset<Mesh>(meshUuid);
            if (meshAsset && meshAsset->IsLoaded()) {
                OnAssetLoaded();
            }
        }
    }

    void LODComponent::SetLODLevels(const std::vector<LODLevel> &levels)
    {
        data.levels = levels;
        
        // 加载所有LOD级别的网格
        for (const auto &level : levels) {
            if (level.meshUuid) {
                binder.Bind(this, level.meshUuid);
                loadingMeshes.push_back(level.meshUuid);
                auto meshAsset = AssetManager::Get()->LoadAsset<Mesh>(level.meshUuid);
                if (meshAsset && meshAsset->IsLoaded()) {
                    OnAssetLoaded();
                }
            }
        }
        
        // 按屏幕占比排序（从高到低）
        std::sort(data.levels.begin(), data.levels.end(), 
            [](const LODLevel &a, const LODLevel &b) {
                return a.screenPercentage > b.screenPercentage;
            });
    }

    void LODComponent::SetCurrentLevel(uint32_t level)
    {
        if (level >= data.levels.size()) {
            level = static_cast<uint32_t>(data.levels.size()) - 1;
        }
        
        if (data.currentLevel != level) {
            targetLevel = level;
            isTransitioning = true;
            transitionProgress = 0.0f;
        }
    }

    Uuid LODComponent::GetCurrentMeshUuid() const
    {
        if (data.currentLevel < data.levels.size()) {
            return data.levels[data.currentLevel].meshUuid;
        }
        return Uuid::GetEmpty();
    }

    void LODComponent::OnAttachToWorld()
    {
        // 创建所有LOD级别的渲染器
        auto *mf = GetFeatureProcessor<MeshFeatureProcessor>(actor);
        if (mf == nullptr) {
            return;
        }

        for (const auto &level : data.levels) {
            if (level.meshUuid) {
                auto meshAsset = AssetManager::Get()->LoadAsset<Mesh>(level.meshUuid);
                if (meshAsset && meshAsset->IsLoaded()) {
                    auto &lodRenderer = lodRenderers[static_cast<uint32_t>(&level - &data.levels[0])];
                    lodRenderer.meshInstance = CreateMeshFromAsset(meshAsset);
                    lodRenderer.renderer = mf->CreateStaticMesh();
                    lodRenderer.visible = false;
                }
            }
        }
    }

    void LODComponent::OnDetachFromWorld()
    {
        // 清理所有渲染器
        for (auto &[level, lod] : lodRenderers) {
            if (lod.renderer != nullptr && actor != nullptr) {
                auto *mf = GetFeatureProcessor<MeshFeatureProcessor>(actor);
                if (mf != nullptr) {
                    mf->RemoveStaticMesh(lod.renderer);
                }
                lod.renderer = nullptr;
            }
            lod.meshInstance = nullptr;
        }
        lodRenderers.clear();
    }

    void LODComponent::OnAssetLoaded()
    {
        // 资产加载完成，创建渲染器
        for (size_t i = 0; i < data.levels.size(); ++i) {
            const auto &level = data.levels[i];
            
            // 检查是否已加载
            bool alreadyLoaded = false;
            for (auto it = loadingMeshes.begin(); it != loadingMeshes.end(); ) {
                if (*it == level.meshUuid) {
                    loadingMeshes.erase(it);
                    alreadyLoaded = true;
                    break;
                } else {
                    ++it;
                }
            }
            
            if (alreadyLoaded) {
                auto &lodRenderer = lodRenderers[static_cast<uint32_t>(i)];
                auto meshAsset = AssetManager::Get()->LoadAsset<Mesh>(level.meshUuid);
                meshAsset->BlockUntilLoaded();
                
                lodRenderer.meshInstance = CreateMeshFromAsset(meshAsset);
                
                auto *mf = GetFeatureProcessor<MeshFeatureProcessor>(actor);
                if (mf != nullptr) {
                    lodRenderer.renderer = mf->CreateStaticMesh();
                    lodRenderer.visible = false;
                }
            }
        }
    }

    void LODComponent::Tick(float time)
    {
        SKY_PROFILE_NAME("LODComponent Tick")

        // 更新当前网格的变换
        for (auto &[level, lod] : lodRenderers) {
            if (lod.renderer != nullptr && lod.visible) {
                auto *ts = actor->GetComponent<TransformComponent>();
                if (ts != nullptr) {
                    lod.renderer->UpdateTransform(ts->GetWorldMatrix());
                }
            }
        }

        // 如果未启用LOD或正在加载，跳过
        if (!data.enableLOD || data.levels.empty()) {
            return;
        }

        // 强制LOD模式
        if (data.forceLOD) {
            SetCurrentLevel(data.forcedLevel);
        } else {
            // 计算应使用的LOD级别
            CalculateLODLevel();
        }

        // 处理LOD切换过渡
        if (isTransitioning) {
            transitionProgress += time / data.crossFadeDuration;
            
            if (transitionProgress >= 1.0f) {
                // 过渡完成
                transitionProgress = 1.0f;
                isTransitioning = false;
                data.currentLevel = targetLevel;
                lodFadeFactor = 1.0f;
                
                UpdateMeshRenderer();
            } else {
                // 计算当前透明度因子
                lodFadeFactor = 1.0f - transitionProgress;
                UpdateMeshRenderer();
            }
        }
    }

    void LODComponent::CalculateLODLevel()
    {
        if (data.levels.empty()) {
            return;
        }

        // 计算屏幕尺寸
        float screenSize = CalculateScreenSize(objectBounds);
        
        // 应用LOD偏移
        screenSize += data.lodBias;
        screenSize = std::clamp(screenSize, 0.0f, 1.0f);

        // 根据屏幕尺寸选择LOD级别
        uint32_t targetLod = 0;
        for (uint32_t i = 0; i < data.levels.size(); ++i) {
            if (screenSize <= data.levels[i].screenPercentage) {
                targetLod = i;
                break;
            }
        }

        SetCurrentLevel(targetLod);
    }

    float LODComponent::CalculateScreenSize(const BoundingSphere &bounds) const
    {
        // 获取相机位置
        Vector3 cameraPos = GetCameraPosition();
        
        // 获取物体世界位置
        Vector3 objectPos = Vector3(0, 0, 0);
        auto *ts = actor->GetComponent<TransformComponent>();
        if (ts != nullptr) {
            auto worldMatrix = ts->GetWorldMatrix();
            objectPos = Vector3(worldMatrix[0][3], worldMatrix[1][3], worldMatrix[2][3]);
        }

        // 计算距离
        float distance = Length(cameraPos - objectPos);
        
        // 获取相机参数
        auto *renderer = Renderer::Get();
        auto camera = renderer->GetMainCamera();
        if (camera == nullptr) {
            return 1.0f;
        }

        float fov = camera->GetFOV();
        float aspect = camera->GetAspect();
        
        // 计算屏幕投影大小
        float viewSize = 2.0f * std::tan(fov * 0.5f) * distance;
        float screenSize = (bounds.radius * 2.0f) / viewSize;
        
        // 考虑宽高比
        if (aspect > 1.0f) {
            screenSize /= aspect;
        }

        return std::clamp(screenSize, 0.0f, 1.0f);
    }

    Vector3 LODComponent::GetCameraPosition() const
    {
        auto *renderer = Renderer::Get();
        auto camera = renderer->GetMainCamera();
        if (camera == nullptr) {
            return Vector3(0, 0, 0);
        }

        auto viewMatrix = camera->GetViewMatrix();
        return Vector3(-viewMatrix[0][3], -viewMatrix[1][3], -viewMatrix[2][3]);
    }

    void LODComponent::UpdateMeshRenderer()
    {
        if (data.levels.empty()) {
            return;
        }

        auto *mf = GetFeatureProcessor<MeshFeatureProcessor>(actor);
        if (mf == nullptr) {
            return;
        }

        // 更新LOD可见性
        for (uint32_t i = 0; i < data.levels.size(); ++i) {
            auto it = lodRenderers.find(i);
            if (it == lodRenderers.end()) {
                continue;
            }
            
            auto &lod = it->second;
            if (lod.renderer == nullptr || lod.meshInstance == nullptr) {
                continue;
            }

            bool shouldVisible = false;
            
            if (!isTransitioning) {
                // 无过渡时，只显示当前LOD
                shouldVisible = (i == data.currentLevel);
            } else {
                // 过渡期间，显示当前LOD和目标LOD
                shouldVisible = (i == data.currentLevel) || (i == targetLevel);
                
                // 设置透明度因子（可以通过修改材质实现淡入淡出）
                if (shouldVisible) {
                    float alpha = (i == data.currentLevel) ? lodFadeFactor : (1.0f - lodFadeFactor);
                    // TODO: 将alpha传递给渲染器或材质
                }
            }

            if (lod.visible != shouldVisible) {
                lod.visible = shouldVisible;
                // 更新渲染器的可见性
                // TODO: 添加可见性控制接口
            }
        }
    }

} // namespace sky
