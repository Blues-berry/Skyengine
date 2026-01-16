# SkyEngine LOD System

## Overview

The Level of Detail (LOD) system in SkyEngine allows for efficient rendering of 3D models by automatically switching between different mesh quality levels based on their distance from the camera. This system is particularly useful for large scenes with many objects, where distant objects can be rendered using simplified meshes without noticeable visual difference.

## Key Features

- **Screen Space-Based LOD Switching**: Automatically switches between LOD levels based on screen size
- **Crossfade Support**: Smooth transitions between LOD levels
- **UE HLOD Compatibility**: Auto-detects UE5 HLOD naming conventions
- **Runtime Control**: Real-time LOD adjustment and debugging capabilities
- **Component-Based Architecture**: Integrated with SkyEngine's component system

## Components

### LODComponent

The main component that manages LOD for static meshes.

#### Key Properties

| Property | Type | Description | Default |
|-----------|-------|-------------|----------|
| `baseName` | string | Base name of the LOD group | - |
| `levels` | vector | List of LOD levels | [] |
| `enableLOD` | bool | Enable/disable LOD switching | true |
| `lodBias` | float | Global LOD bias (shifts thresholds) | 0.0 |
| `forcedLOD` | int | Force specific LOD level (-1 = auto) | -1 |
| `enableCrossfade` | bool | Enable smooth transitions | true |
| `crossfadeDuration` | float | Crossfade duration in seconds | 0.2 |

#### LOD Level Structure

Each LOD level contains:

- `meshUuid`: UUID of the mesh asset for this level
- `screenPercentage`: Screen size threshold (0.0-1.0)
- `triangleCount`: Number of triangles
- `vertexCount`: Number of vertices
- `boundsCenter`: Center of bounding sphere
- `boundsRadius`: Radius of bounding sphere

### LODDebugComponent

Provides debugging and visualization for the LOD system.

#### Key Properties

| Property | Type | Description | Default |
|-----------|-------|-------------|----------|
| `enableDebugOverlay` | bool | Show debug overlay on screen | false |
| `enableWireframe` | bool | Render LOD meshes in wireframe | false |
| `overlayX` | float | Debug overlay X position | 10.0 |
| `overlayY` | float | Debug overlay Y position | 10.0 |
| `overlayColor` | Vector4 | Overlay text color | (1,1,1,1) |

## Workflow

### 1. Model Preparation

Export your models with LOD meshes using appropriate naming conventions:

#### Standard LOD Naming
```
Building_LOD0.mesh    (Highest quality, full detail)
Building_LOD1.mesh    (Medium quality)
Building_LOD2.mesh    (Low quality, distant)
```

#### UE HLOD Naming
```
Small_City_LVL_HLOD0_256m_767m_L0_X0_Y0_Material
Small_City_LVL_HLOD1_512m_1534m_L1_X0_Y0_Material
```

### 2. Model Import

When importing a model with LOD meshes, the LOD system will:

1. Auto-detect LOD groups from mesh names
2. Create `.lod` configuration file alongside the prefab
3. Calculate screen percentage thresholds automatically

Example import command:
```bash
asset_builder.exe -p "C:/SkyEngine" -i "path/to/model.glb"
```

This generates:
```
assets/Prefabs/model.glb/model.prefab      (Main prefab)
assets/Prefabs/model.glb/model.lod        (LOD configuration)
assets/Prefabs/model.glb/Building_LOD0.mesh
assets/Prefabs/model.glb/Building_LOD1.mesh
assets/Prefabs/model.glb/Building_LOD2.mesh
```

### 3. Using LOD in Editor

1. Create a new Actor
2. Add `StaticMeshComponent`
3. Add `LODComponent`
4. Add `LODDebugComponent` (optional, for debugging)
5. In the LOD Component:
   - Load the prefab with LOD meshes
   - The LOD configuration is automatically loaded from `.lod` file
   - Adjust `lodBias` if needed
6. (Optional) In the Debug Component:
   - Enable `enableDebugOverlay` to see real-time LOD info
   - Enable `enableWireframe` to visualize mesh changes

### 4. Runtime Behavior

During runtime:

1. Camera position is tracked
2. Screen space size of each LOD group is calculated
3. LOD level is selected based on `screenPercentage` thresholds
4. Mesh is switched to the appropriate LOD
5. If `enableCrossfade` is true, smooth transition occurs
6. Debug component displays current LOD information

## LOD Threshold Calculation

### Screen Size Calculation

Screen size is calculated as:

```
screenSize = (2 * boundingSphereRadius) / (distance * cameraHeight)
```

Where:
- `boundingSphereRadius`: Radius of the mesh's bounding sphere
- `distance`: Distance from camera to object center
- `cameraHeight`: Camera's view height

### LOD Selection

Given `N` LOD levels with screen thresholds `[S0, S1, ..., SN-1]` (descending):

- Level 0 (highest quality): Used when `screenSize > S0`
- Level 1: Used when `S1 < screenSize <= S0`
- Level N-1 (lowest quality): Used when `screenSize <= SN-2`

### LOD Bias

`lodBias` shifts all thresholds:

```cpp
adjustedScreenSize = screenSize - lodBias
```

- Positive bias: Use lower LODs (less detail)
- Negative bias: Use higher LODs (more detail)
- Typical range: -0.2 to +0.2

## Configuration File Format (.lod)

```json
{
  "lodGroups": [
    {
      "baseName": "Building",
      "levels": [
        {
          "meshUuid": "uuid-here-0000",
          "screenPercentage": 1.0,
          "triangleCount": 5000,
          "vertexCount": 3000,
          "boundsCenter": [0.0, 0.0, 0.0],
          "boundsRadius": 5.0
        },
        {
          "meshUuid": "uuid-here-0001",
          "screenPercentage": 0.5,
          "triangleCount": 2000,
          "vertexCount": 1200,
          "boundsCenter": [0.0, 0.0, 0.0],
          "boundsRadius": 5.0
        }
      ]
    }
  ],
  "enableLOD": true,
  "lodBias": 0.0
}
```

## Performance Considerations

### LOD Switching Overhead
- Screen size calculation: ~0.01ms per LOD group
- LOD level selection: ~0.005ms per group
- Crossfade blending: ~0.02ms per transition

### Memory Usage
- Per LOD level: ~50-200KB (depending on mesh complexity)
- LOD configuration file: ~1-10KB

### Recommended LOD Counts

| Scene Type | LOD Levels | Typical Switch Distances |
|-------------|--------------|-------------------------|
| Small indoor | 2-3 | 10-50m |
| Medium outdoor | 3-4 | 10-100m |
| Large city/terrain | 4-6 | 10-200m |

## API Reference

### LODComponent

```cpp
// Add LOD level at runtime
void AddLODLevel(const Uuid& meshUuid, float screenPercentage);

// Set all LOD levels
void SetLODLevels(const std::vector<LODLevel>& levels);

// Force specific LOD level
void SetCurrentLevel(uint32_t level);

// Control LOD behavior
void SetEnableLOD(bool enable);
void SetLODBias(float bias);
void SetForcedLOD(int32_t level);
void SetEnableCrossfade(bool enable);

// Query current state
uint32_t GetCurrentLevel() const;
uint32_t GetNumLevels() const;
Uuid GetCurrentMeshUuid() const;
float GetCurrentLODFadeFactor() const;
```

### LODDebugComponent

```cpp
// Enable debug visualization
void SetEnableDebugOverlay(bool enable);
void SetEnableWireframe(bool enable);
void SetOverlayPosition(float x, float y);
void SetOverlayColor(const Vector4& color);

// Get debug info
const LODDebugInfo& GetDebugInfo() const;
void PrintDebugInfo();
std::string GetDebugString() const;
```

## Best Practices

1. **LOD Planning**: Design LODs from the start, not as an afterthought
2. **Progressive Simplification**: Each level should be ~50% of the previous
3. **Preserve Silhouettes**: Lower LODs should maintain recognizable shapes
4. **Balance Quality/Performance**: Test different LOD counts for your use case
5. **Use Debug Tools**: Enable LODDebugComponent during development
6. **Profile Performance**: Measure impact with profiler tools

## Troubleshooting

### LOD Not Switching
- Verify `enableLOD` is true
- Check that camera is registered in the world
- Ensure mesh bounds are correct

### Visible Popping
- Enable `enableCrossfade`
- Adjust `crossfadeDuration`
- Increase LOD level count
- Review LOD quality transitions

### Performance Issues
- Reduce `crossfadeDuration`
- Disable debug overlays in production
- Optimize mesh complexity at each LOD level

## Future Enhancements

Planned improvements for future versions:

- **Skeletal Mesh LOD Support**: Extend to animated meshes
- **Material LOD**: Switch materials at different distances
- **Adaptive LOD**: Automatically adjust based on frame rate
- **Screen Space LOD**: Pixel-density based selection
- **GPU-Driven LOD**: Compute on GPU for better performance
- **Auto-LOD Generation**: Automatically generate lower LODs

## Examples

See the included test scene for a working example:

- `assets/Scenes/LODTestScene.json`
- `scripts/test_lod.py` - Automated test script

## Contributing

When contributing to the LOD system:

1. Follow SkyEngine coding standards
2. Add unit tests for new features
3. Update documentation
4. Profile performance impact
5. Test on various hardware configurations

## Version History

### v1.0.0 (2025-01-14)
- Initial LOD system implementation
- UE HLOD auto-detection
- Screen space-based LOD switching
- Crossfade support
- Debug component with visualization
- Complete documentation
