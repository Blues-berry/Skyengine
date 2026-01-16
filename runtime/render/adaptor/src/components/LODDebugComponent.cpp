//
// Created for SkyEngine LOD System
//

#include <render/adaptor/components/LODDebugComponent.h>
#include <render/adaptor/components/LODComponent.h>
#include <render/Renderer.h>
#include <framework/world/Actor.h>
#include <core/logger/Logger.h>
#include <sstream>
#include <iomanip>

static const char* TAG = "LODDebugComponent";

namespace sky {

    LODDebugComponent::LODDebugComponent()
        : lodComponent(nullptr)
        , enableDebugOverlay(false)
        , enableWireframe(false)
        , overlayX(10.0f)
        , overlayY(10.0f)
        , overlayColor(1.0f, 1.0f, 1.0f, 1.0f)
    {
    }

    LODDebugComponent::~LODDebugComponent() = default;

    void LODDebugComponent::Reflect(SerializationContext* context)
    {
        REGISTER_BEGIN(LODDebugComponent, context)
            REGISTER_MEMBER(enableDebugOverlay)
            REGISTER_MEMBER(enableWireframe)
            REGISTER_MEMBER(overlayX)
            REGISTER_MEMBER(overlayY)
            REGISTER_MEMBER(overlayColor)
        REGISTER_END()
    }

    void LODDebugComponent::SaveJson(JsonOutputArchive& ar) const
    {
        ar.Save("enableDebugOverlay", enableDebugOverlay);
        ar.Save("enableWireframe", enableWireframe);
        ar.Save("overlayX", overlayX);
        ar.Save("overlayY", overlayY);
        ar.Save("overlayColor", overlayColor);
    }

    void LODDebugComponent::LoadJson(JsonInputArchive& ar)
    {
        ar.Load("enableDebugOverlay", enableDebugOverlay);
        ar.Load("enableWireframe", enableWireframe);
        ar.Load("overlayX", overlayX);
        ar.Load("overlayY", overlayY);
        ar.Load("overlayColor", overlayColor);
    }

    void LODDebugComponent::Tick(float time)
    {
        // Cache LOD component reference
        if (lodComponent == nullptr) {
            lodComponent = GetOwner()->GetComponent<LODComponent>();
        }

        // Update debug information
        if (lodComponent != nullptr) {
            UpdateDebugInfo();
        }

        // Render debug visualization
        if (enableDebugOverlay) {
            RenderDebugOverlay();
        }

        if (enableWireframe) {
            RenderLODBounds();
        }
    }

    void LODDebugComponent::UpdateDebugInfo()
    {
        if (lodComponent == nullptr) {
            return;
        }

        debugInfo.currentLODLevel = lodComponent->GetCurrentLevel();
        debugInfo.numLODLevels = lodComponent->GetNumLevels();
        debugInfo.lodFadeFactor = lodComponent->GetCurrentLODFadeFactor();
        debugInfo.isLODEnabled = lodComponent->IsLODEnabled();
        debugInfo.forcedLOD = lodComponent->GetForcedLOD();
        debugInfo.lodBias = lodComponent->GetLODBias();
    }

    void LODDebugComponent::PrintDebugInfo()
    {
        LOG_I(TAG, "===== LOD Debug Information =====");
        LOG_I(TAG, "Current LOD Level: %u / %zu", 
            debugInfo.currentLODLevel, debugInfo.numLODLevels);
        LOG_I(TAG, "LOD Fade Factor: %.3f", debugInfo.lodFadeFactor);
        LOG_I(TAG, "Screen Size: %.3f", debugInfo.screenSize);
        LOG_I(TAG, "Distance to Camera: %.2f", debugInfo.distanceToCamera);
        LOG_I(TAG, "LOD Enabled: %s", debugInfo.isLODEnabled ? "Yes" : "No");
        LOG_I(TAG, "Forced LOD: %d", debugInfo.forcedLOD);
        LOG_I(TAG, "LOD Bias: %.2f", debugInfo.lodBias);
        LOG_I(TAG, "================================");
    }

    std::string LODDebugComponent::GetDebugString() const
    {
        std::stringstream ss;
        ss << "===== LOD Debug =====\n";
        ss << "LOD Level: " << debugInfo.currentLODLevel << " / " << debugInfo.numLODLevels << "\n";
        ss << "Fade Factor: " << std::fixed << std::setprecision(3) << debugInfo.lodFadeFactor << "\n";
        ss << "Screen Size: " << debugInfo.screenSize << "\n";
        ss << "Distance: " << std::fixed << std::setprecision(2) << debugInfo.distanceToCamera << "\n";
        ss << "Enabled: " << (debugInfo.isLODEnabled ? "Yes" : "No") << "\n";
        ss << "Forced: " << debugInfo.forcedLOD << "\n";
        ss << "Bias: " << debugInfo.lodBias << "\n";
        return ss.str();
    }

    void LODDebugComponent::RenderDebugOverlay()
    {
        // This would typically use ImGui or a debug rendering system
        // For now, we'll just log the information periodically
        
        static float lastLogTime = 0.0f;
        lastLogTime += 0.016f; // Approx 60fps
        
        if (lastLogTime >= 1.0f) {
            PrintDebugInfo();
            lastLogTime = 0.0f;
        }
    }

    void LODDebugComponent::RenderLODBounds()
    {
        // This would render wireframe spheres around LOD meshes
        // Implementation depends on the debug rendering system
        // TODO: Implement when debug rendering system is available
    }

} // namespace sky
