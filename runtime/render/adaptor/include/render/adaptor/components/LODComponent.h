//
// Created by AI on 2025/1/14.
// LOD (Level of Detail) Component for SkyEngine
//

#pragma once

#include <framework/world/Component.h>
#include <framework/asset/AssetEvent.h>
#include <render/adaptor/assets/MeshAsset.h>
#include <core/math/Vector3.h>
#include <core/math/BoundingSphere.h>
#include <vector>
#include <unordered_map>

namespace sky {

    /**
     * 单个LOD级别定义
     */
    struct LODLevel {
        Uuid meshUuid;              // 当前级别的网格UUID
        float screenPercentage;      // 屏幕占比阈值 (0.0-1.0)
        uint32_t triangleCount;      // 三角面数（用于调试）
        uint32_t vertexCount;        // 顶点数（用于调试）
        BoundingSphere bounds;       // 包围球
    };

    /**
     * LOD组数据
     */
    struct LODGroupData {
        std::vector<LODLevel> levels;    // 所有LOD级别
        uint32_t currentLevel = 0;        // 当前激活的LOD级别
        float crossFadeDuration = 0.1f;   // 跨度混合持续时间（秒）
        bool forceLOD = false;            // 是否强制使用特定LOD
        uint32_t forcedLevel = 0;         // 强制使用的LOD级别
        bool enableLOD = true;            // 是否启用LOD系统
        float lodBias = 0.0f;            // LOD偏移（正数显示更高质量）
    };

    /**
     * LOD组件
     * 管理网格的多级别细节切换
     */
    class LODComponent : public ComponentBase, public IAssetEvent {
    public:
        LODComponent();
        ~LODComponent() override;

        COMPONENT_RUNTIME_INFO(LODComponent)

        static void Reflect(SerializationContext *context);

        void Tick(float time) override;

        void SaveJson(JsonOutputArchive &ar) const override;
        void LoadJson(JsonInputArchive &ar) override;

        // LOD级别管理
        void AddLODLevel(const Uuid &meshUuid, float screenPercentage);
        void SetLODLevels(const std::vector<LODLevel> &levels);
        const std::vector<LODLevel>& GetLODLevels() const { return data.levels; }

        // 当前LOD控制
        uint32_t GetCurrentLevel() const { return data.currentLevel; }
        void SetCurrentLevel(uint32_t level);

        // 强制LOD
        void SetForceLOD(bool force) { data.forceLOD = force; }
        bool GetForceLOD() const { return data.forceLOD; }
        void SetForcedLevel(uint32_t level) { data.forcedLevel = level; }

        // 系统开关
        void SetEnableLOD(bool enable) { data.enableLOD = enable; }
        bool GetEnableLOD() const { return data.enableLOD; }

        // LOD偏移
        void SetLODBias(float bias) { data.lodBias = bias; }
        float GetLODBias() const { return data.lodBias; }

        // 跨度混合
        void SetCrossFadeDuration(float duration) { data.crossFadeDuration = duration; }
        float GetCrossFadeDuration() const { return data.crossFadeDuration; }

        void OnAttachToWorld() override;
        void OnDetachFromWorld() override;

        // 获取当前渲染的网格UUID
        Uuid GetCurrentMeshUuid() const;
        float GetCurrentLODFadeFactor() const { return lodFadeFactor; }

    private:
        void CalculateLODLevel();
        void UpdateMeshRenderer();
        float CalculateScreenSize(const BoundingSphere &bounds) const;
        Vector3 GetCameraPosition() const;

        void OnAssetLoaded() override;

        LODGroupData data;

        // 渲染相关
        struct LODRenderer {
            RDMeshPtr meshInstance;
            MeshRenderer *renderer = nullptr;
            bool visible = false;
        };

        std::unordered_map<uint32_t, LODRenderer> lodRenderers;
        
        // LOD切换状态
        uint32_t targetLevel = 0;
        float lodFadeFactor = 1.0f;      // 当前LOD的透明度因子 (0.0-1.0)
        float transitionProgress = 0.0f;    // 跨度混合进度 (0.0-1.0)
        bool isTransitioning = false;       // 是否正在切换LOD

        // 资产加载
        std::vector<Uuid> loadingMeshes;
        EventBinder<IAssetEvent, Uuid> binder;
        
        // 包围球（用于距离计算）
        BoundingSphere objectBounds;
    };

} // namespace sky
