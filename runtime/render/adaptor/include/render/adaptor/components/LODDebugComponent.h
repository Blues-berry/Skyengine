//
// Created by AI on 2025/1/14.
// LOD Debug Component - 可视化LOD信息
//

#pragma once

#include <framework/world/Component.h>
#include <render/adaptor/components/LODComponent.h>

namespace sky {

    /**
     * LOD调试信息
     */
    struct LODDebugInfo {
        uint32_t currentLODLevel = 0;      // 当前LOD级别
        float screenSize = 0.0f;            // 屏幕占比
        float distanceToCamera = 0.0f;        // 距相机距离
        float lodFadeFactor = 1.0f;          // LOD混合因子
        uint32_t targetLODLevel = 0;        // 目标LOD级别
        bool isTransitioning = false;         // 是否正在切换
        float transitionProgress = 0.0f;      // 切换进度
        uint32_t totalLODLevels = 0;         // 总LOD级别数
    };

    /**
     * LOD调试组件
     * 用于在运行时查看LOD状态
     */
    class LODDebugComponent : public ComponentBase {
    public:
        LODDebugComponent() = default;
        ~LODDebugComponent() override = default;

        COMPONENT_RUNTIME_INFO(LODDebugComponent)

        static void Reflect(SerializationContext *context);

        void Tick(float time) override;

        void SaveJson(JsonOutputArchive &ar) const override;
        void LoadJson(JsonInputArchive &ar) override;

        // 调试信息
        const LODDebugInfo& GetDebugInfo() const { return debugInfo; }

        // 启用/禁用
        void SetEnableDebug(bool enable) { enableDebug = enable; }
        bool GetEnableDebug() const { return enableDebug; }

        // 自动打印日志
        void SetAutoLog(bool autoLog) { this->autoLog = autoLog; }
        bool GetAutoLog() const { return autoLog; }

        // 日志间隔（秒）
        void SetLogInterval(float interval) { logInterval = interval; }
        float GetLogInterval() const { return logInterval; }

    private:
        void UpdateDebugInfo();
        void PrintDebugInfo();

        LODDebugInfo debugInfo;
        
        bool enableDebug = true;      // 启用调试
        bool autoLog = false;        // 自动打印日志
        float logInterval = 1.0f;    // 日志间隔（秒）
        float logTimer = 0.0f;       // 日志计时器
        
        LODComponent* lodComponent = nullptr;  // LOD组件引用
    };

} // namespace sky
