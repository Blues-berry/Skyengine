//
// Created by AI on 2025/1/14.
// LOD Debug Component Implementation
//

#include <render/adaptor/components/LODDebugComponent.h>
#include <framework/serialization/JsonArchive.h>
#include <framework/serialization/PropertyCommon.h>
#include <framework/world/Actor.h>
#include <core/logger/Logger.h>
#include <core/math/Vector3.h>

namespace sky {

    static const char* TAG = "LODDebug";

    void LODDebugComponent::Reflect(SerializationContext *context)
    {
        context->Register<LODDebugInfo>("LODDebugInfo")
            .Member<&LODDebugInfo::currentLODLevel>("CurrentLODLevel")
            .Member<&LODDebugInfo::screenSize>("ScreenSize")
            .Member<&LODDebugInfo::distanceToCamera>("DistanceToCamera")
            .Member<&LODDebugInfo::lodFadeFactor>("LODFadeFactor")
            .Member<&LODDebugInfo::targetLODLevel>("TargetLODLevel")
            .Member<&LODDebugInfo::isTransitioning>("IsTransitioning")
            .Member<&LODDebugInfo::transitionProgress>("TransitionProgress")
            .Member<&LODDebugInfo::totalLODLevels>("TotalLODLevels");

        REGISTER_BEGIN(LODDebugComponent, context)
            REGISTER_MEMBER(EnableDebug, SetEnableDebug, GetEnableDebug)
            REGISTER_MEMBER(AutoLog, SetAutoLog, GetAutoLog)
            REGISTER_MEMBER(LogInterval, SetLogInterval, GetLogInterval);
    }

    void LODDebugComponent::SaveJson(JsonOutputArchive &ar) const
    {
        ar.StartObject();
        ar.SaveValueObject(std::string("enableDebug"), enableDebug);
        ar.SaveValueObject(std::string("autoLog"), autoLog);
        ar.SaveValueObject(std::string("logInterval"), logInterval);
        ar.EndObject();
    }

    void LODDebugComponent::LoadJson(JsonInputArchive &ar)
    {
        ar.LoadKeyValue("enableDebug", enableDebug);
        ar.LoadKeyValue("autoLog", autoLog);
        ar.LoadKeyValue("logInterval", logInterval);
    }

    void LODDebugComponent::Tick(float time)
    {
        if (!enableDebug) {
            return;
        }

        // 获取LOD组件引用
        if (lodComponent == nullptr) {
            lodComponent = actor->GetComponent<LODComponent>();
        }

        if (lodComponent != nullptr) {
            UpdateDebugInfo();
            
            // 自动打印日志
            if (autoLog) {
                logTimer += time;
                if (logTimer >= logInterval) {
                    logTimer = 0.0f;
                    PrintDebugInfo();
                }
            }
        }
    }

    void LODDebugComponent::UpdateDebugInfo()
    {
        if (lodComponent == nullptr) {
            return;
        }

        // 更新调试信息
        debugInfo.currentLODLevel = lodComponent->GetCurrentLevel();
        debugInfo.lodFadeFactor = lodComponent->GetCurrentLODFadeFactor();

        // TODO: 获取其他LOD信息
        // 需要在LODComponent中添加更多getter方法
        // - GetScreenSize()
        // - GetDistanceToCamera()
        // - GetTargetLODLevel()
        // - IsTransitioning()
        // - GetTransitionProgress()
        // - GetTotalLODLevels()

        debugInfo.targetLODLevel = debugInfo.currentLODLevel;  // 默认值
        debugInfo.isTransitioning = false;                   // 默认值
        debugInfo.transitionProgress = 0.0f;                 // 默认值
        debugInfo.totalLODLevels = 4;                        // 默认值
    }

    void LODDebugComponent::PrintDebugInfo()
    {
        LOG_I(TAG, "=== LOD Debug Info ===");
        LOG_I(TAG, "当前LOD级别: %u / %u", debugInfo.currentLODLevel, debugInfo.totalLODLevels);
        LOG_I(TAG, "屏幕占比: %.2f%%", debugInfo.screenSize * 100.0f);
        LOG_I(TAG, "相机距离: %.2f 米", debugInfo.distanceToCamera);
        LOG_I(TAG, "LOD混合因子: %.2f", debugInfo.lodFadeFactor);
        
        if (debugInfo.isTransitioning) {
            LOG_I(TAG, "LOD切换中: %u → %u (进度: %.2f%%)",
                debugInfo.currentLODLevel,
                debugInfo.targetLODLevel,
                debugInfo.transitionProgress * 100.0f);
        }
        
        LOG_I(TAG, "=====================");
    }

} // namespace sky
