//
// Created for SkyEngine LOD System
//

#pragma once

#include <framework/world/Component.h>
#include <framework/world/Actor.h>
#include <framework/serialization/JsonArchive.h>
#include <render/adaptor/components/LODComponent.h>
#include <string>
#include <vector>

namespace sky {

    /**
     * @brief Debug Information for LOD
     * 
     * Contains current LOD state for debugging and visualization.
     */
    struct LODDebugInfo {
        uint32_t currentLODLevel;     ///< Current active LOD level
        uint32_t numLODLevels;        ///< Total number of LOD levels
        float lodFadeFactor;           ///< Current fade factor (0.0-1.0)
        float screenSize;              ///< Current screen space size
        float distanceToCamera;        ///< Distance from camera
        bool isLODEnabled;           ///< Whether LOD is enabled
        int32_t forcedLOD;           ///< Forced LOD level (or -1 if auto)
        float lodBias;                ///< LOD bias value
        
        LODDebugInfo()
            : currentLODLevel(0)
            , numLODLevels(0)
            , lodFadeFactor(1.0f)
            , screenSize(1.0f)
            , distanceToCamera(0.0f)
            , isLODEnabled(true)
            , forcedLOD(-1)
            , lodBias(0.0f)
        {}
    };

    /**
     * @brief LOD Debug Component
     * 
     * Provides debugging and visualization capabilities for the LOD system.
     * Displays LOD information in the editor and renders debug overlays.
     */
    class LODDebugComponent : public Component {
        SKY_REFLECT(LODDebugComponent, Component)
    public:
        LODDebugComponent();
        ~LODDebugComponent() override;

        // ========== Serialization ==========
        void Reflect(SerializationContext* context) override;
        void SaveJson(JsonOutputArchive& ar) const override;
        void LoadJson(JsonInputArchive& ar) override;

        // ========== Component Lifecycle ==========
        void Tick(float time) override;

        // ========== Debug Visualization ==========
        /**
         * @brief Enable or disable debug overlay rendering
         * @param enable True to enable debug overlay
         */
        void SetEnableDebugOverlay(bool enable) { enableDebugOverlay = enable; }

        /**
         * @brief Set the position of the debug overlay
         * @param x X position in screen coordinates
         * @param y Y position in screen coordinates
         */
        void SetOverlayPosition(float x, float y) {
            overlayX = x;
            overlayY = y;
        }

        /**
         * @brief Set debug overlay text color
         * @param color RGBA color
         */
        void SetOverlayColor(const Vector4& color) { overlayColor = color; }

        /**
         * @brief Enable or disable wireframe rendering for LOD
         * @param enable True to render LOD meshes in wireframe
         */
        void SetEnableWireframe(bool enable) { enableWireframe = enable; }

        /**
         * @brief Get the current debug information
         * @return LOD debug info structure
         */
        const LODDebugInfo& GetDebugInfo() const { return debugInfo; }

        /**
         * @brief Print debug information to console
         */
        void PrintDebugInfo();

        /**
         * @brief Get debug information as a formatted string
         * @return Formatted debug string
         */
        std::string GetDebugString() const;

    private:
        /**
         * @brief Update debug information from LOD component
         */
        void UpdateDebugInfo();

        /**
         * @brief Render debug overlay
         */
        void RenderDebugOverlay();

        /**
         * @brief Render LOD bounds visualization
         */
        void RenderLODBounds();

    private:
        // ========== Debug State ==========
        LODDebugInfo debugInfo;
        
        // LOD component reference (cached)
        LODComponent* lodComponent;

        // ========== Visualization Settings ==========
        bool enableDebugOverlay;      ///< Enable on-screen debug overlay
        bool enableWireframe;         ///< Render LOD meshes in wireframe
        float overlayX;              ///< Debug overlay X position
        float overlayY;              ///< Debug overlay Y position
        Vector4 overlayColor;         ///< Debug overlay text color
    };

} // namespace sky
