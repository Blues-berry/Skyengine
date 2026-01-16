//
// Created for SkyEngine LOD System
//

#pragma once

#include <framework/world/Component.h>
#include <framework/world/Actor.h>
#include <framework/serialization/JsonArchive.h>
#include <render/resource/Mesh.h>
#include <core/math/Vector3.h>
#include <core/math/BoundingSphere.h>
#include <core/Uuid.h>
#include <vector>
#include <memory>

namespace sky {

    /**
     * @brief LOD Level Data Structure
     * 
     * Contains information about a single LOD level, including the mesh UUID,
     * screen percentage threshold, and bounds information.
     */
    struct LODLevel {
        Uuid meshUuid;              ///< UUID of the mesh for this LOD level
        float screenPercentage;       ///< Screen size threshold (0.0-1.0)
        uint32_t triangleCount;      ///< Number of triangles in this LOD
        uint32_t vertexCount;        ///< Number of vertices in this LOD
        Vector3 boundsCenter;        ///< Center of the bounding sphere
        float boundsRadius;          ///< Radius of the bounding sphere

        LODLevel()
            : screenPercentage(0.0f)
            , triangleCount(0)
            , vertexCount(0)
            , boundsCenter(0.0f, 0.0f, 0.0f)
            , boundsRadius(1.0f)
        {}
    };

    /**
     * @brief LOD Data for a single mesh group
     * 
     * Contains all LOD levels for a specific mesh or mesh group.
     */
    struct LODData {
        std::string baseName;         ///< Base name of the LOD group
        std::vector<LODLevel> levels;///< List of LOD levels (sorted by screenPercentage)
        
        LODData() = default;
    };

    /**
     * @brief LOD Component
     * 
     * Manages Level of Detail for static meshes. Automatically switches between
     * different LOD levels based on screen space size.
     * 
     * Features:
     * - Screen space percentage-based LOD switching
     * - Crossfade between LOD levels
     * - LOD bias for global LOD control
     * - Forced LOD mode for debugging
     */
    class LODComponent : public Component {
        SKY_REFLECT(LODComponent, Component)
    public:
        LODComponent();
        ~LODComponent() override;

        // ========== Serialization ==========
        void Reflect(SerializationContext* context) override;
        void SaveJson(JsonOutputArchive& ar) const override;
        void LoadJson(JsonInputArchive& ar) override;

        // ========== Component Lifecycle ==========
        void OnAttachToWorld(World* world) override;
        void OnDetachFromWorld() override;
        void OnAssetLoaded() override;
        void Tick(float time) override;

        // ========== LOD Configuration ==========
        /**
         * @brief Add a new LOD level
         * @param meshUuid UUID of the mesh asset
         * @param screenPercentage Screen size threshold (0.0-1.0)
         */
        void AddLODLevel(const Uuid& meshUuid, float screenPercentage);

        /**
         * @brief Set all LOD levels at once
         * @param levels Vector of LOD levels
         */
        void SetLODLevels(const std::vector<LODLevel>& levels);

        /**
         * @brief Clear all LOD levels
         */
        void ClearLODLevels();

        // ========== Runtime Control ==========
        /**
         * @brief Set the current LOD level (used for debugging)
         * @param level LOD level index (0 = highest quality)
         */
        void SetCurrentLevel(uint32_t level);

        /**
         * @brief Enable or disable LOD
         * @param enable True to enable LOD switching
         */
        void SetEnableLOD(bool enable) { enableLOD = enable; }

        /**
         * @brief Set LOD bias to shift LOD thresholds
         * @param bias Bias value (positive = use lower LODs, negative = use higher LODs)
         */
        void SetLODBias(float bias) { lodBias = bias; }

        /**
         * @brief Force a specific LOD level
         * @param level LOD level to force, or -1 to disable forcing
         */
        void SetForcedLOD(int32_t level) { forcedLOD = level; }

        /**
         * @brief Enable or disable crossfade between LOD levels
         * @param enable True to enable crossfading
         */
        void SetEnableCrossfade(bool enable) { enableCrossfade = enable; }

        /**
         * @brief Set crossfade duration in seconds
         * @param duration Crossfade duration
         */
        void SetCrossfadeDuration(float duration) { crossfadeDuration = duration; }

        // ========== Query Functions ==========
        /**
         * @brief Get the current LOD level
         * @return Current LOD level index
         */
        uint32_t GetCurrentLevel() const { return currentLODLevel; }

        /**
         * @brief Get the number of LOD levels
         * @return Number of LOD levels
         */
        uint32_t GetNumLevels() const { return static_cast<uint32_t>(lodData.levels.size()); }

        /**
         * @brief Get the UUID of the mesh for the current LOD level
         * @return Mesh UUID, or invalid UUID if no LOD levels
         */
        Uuid GetCurrentMeshUuid() const;

        /**
         * @brief Get the fade factor for crossfading
         * @return Fade factor (0.0-1.0, 0.0 = previous LOD, 1.0 = current LOD)
         */
        float GetCurrentLODFadeFactor() const { return lodFadeFactor; }

        /**
         * @brief Check if LOD is enabled
         * @return True if LOD is enabled
         */
        bool IsLODEnabled() const { return enableLOD; }

        /**
         * @brief Get the LOD bias
         * @return LOD bias value
         */
        float GetLODBias() const { return lodBias; }

        /**
         * @brief Get the forced LOD level
         * @return Forced LOD level, or -1 if not forced
         */
        int32_t GetForcedLOD() const { return forcedLOD; }

        /**
         * @brief Check if crossfade is enabled
         * @return True if crossfade is enabled
         */
        bool IsCrossfadeEnabled() const { return enableCrossfade; }

    private:
        // ========== LOD Data ==========
        LODData lodData;              ///< LOD configuration data

        // ========== Runtime State ==========
        uint32_t currentLODLevel;    ///< Current active LOD level
        uint32_t previousLODLevel;   ///< Previous LOD level (for crossfading)
        float lodFadeFactor;          ///< Current fade factor (0.0-1.0)
        float lodFadeTime;            ///< Time since LOD transition started

        // ========== Configuration ==========
        bool enableLOD;              ///< Enable LOD switching
        float lodBias;               ///< Global LOD bias (shifts thresholds)
        int32_t forcedLOD;          ///< Forced LOD level (-1 = auto)
        bool enableCrossfade;         ///< Enable crossfade between LODs
        float crossfadeDuration;      ///< Crossfade duration in seconds

        // ========== Internal Functions ==========
        /**
         * @brief Calculate the appropriate LOD level based on camera distance
         */
        void CalculateLODLevel();

        /**
         * @brief Calculate screen size of the mesh
         * @param bounds Bounding sphere of the mesh
         * @return Screen space size (0.0-1.0)
         */
        float CalculateScreenSize(const BoundingSphere& bounds) const;

        /**
         * @brief Get the camera position in world space
         * @return Camera position
         */
        Vector3 GetCameraPosition() const;

        /**
         * @brief Update the mesh renderer with the current LOD mesh
         */
        void UpdateMeshRenderer();
    };

} // namespace sky
