# SkyEngine LOD System - Quick Start Guide

## Prerequisites

Before using the LOD system, ensure you have:

- SkyEngine v1.0 or later
- Model files with LOD meshes (GLB/GLTF/FBX format)
- Understanding of basic SkyEngine editor operations

## Quick Start: 5 Steps

### Step 1: Prepare Your Model

Export your model with multiple LOD versions using standard naming:

```
MyObject_LOD0.mesh  // Highest detail (0-50m)
MyObject_LOD1.mesh  // Medium detail (50-100m)
MyObject_LOD2.mesh  // Low detail (100-200m)
```

**Tip**: Use 3D modeling software (Blender, 3ds Max) to create progressive LOD meshes. Each level should have ~50% fewer polygons than the previous.

### Step 2: Import Your Model

Use the asset builder to import your model:

```cmd
cd build/install/bin
asset_builder.exe -p "C:/SkyEngine" -i "C:/models/MyObject.glb"
```

The LOD system will automatically:
- Detect LOD meshes from naming
- Create LOD configuration file
- Import all mesh variants

Expected output files:
```
assets/Prefabs/MyObject.glb/
├── MyObject.lod              # LOD configuration
├── MyObject_LOD0.mesh        # High detail mesh
├── MyObject_LOD1.mesh        # Medium detail mesh
├── MyObject_LOD2.mesh        # Low detail mesh
└── MyObject.prefab           # Main prefab
```

### Step 3: Create an Actor with LOD

1. Open SkyEngine Editor
2. Right-click in scene → Create Actor
3. Add components:
   - **StaticMeshComponent** (required)
   - **LODComponent** (required)
   - **LODDebugComponent** (optional, for debugging)

### Step 4: Configure LOD

In the LODComponent inspector:

1. **Load Prefab**: Select `MyObject.prefab`
2. **Verify LOD Levels**: Check that all 3 levels are detected
3. **Adjust Settings** (optional):
   - `Enable LOD`: Toggle LOD on/off
   - `LOD Bias`: Shift thresholds (-0.1 to +0.1)
   - `Forced LOD`: Force specific level (-1 = auto, 0/1/2 = specific)

For debugging, enable in LODDebugComponent:
- **Enable Debug Overlay**: Shows current LOD on screen
- **Enable Wireframe**: Visualizes mesh changes

### Step 5: Test and Refine

1. **Play the scene**: Press Play button
2. **Move camera**: Approach and retreat from the object
3. **Observe transitions**: Watch for LOD level changes
4. **Adjust if needed**:
   - If transitions are too abrupt: Enable crossfade or increase duration
   - If details pop too early: Increase LOD bias
   - If details pop too late: Decrease LOD bias

**Expected Result**: As you move the camera away, the object smoothly transitions from high-detail (LOD0) to medium-detail (LOD1) to low-detail (LOD2).

## Common Use Cases

### Use Case 1: Urban Scene

For city scenes with many buildings:

```python
# Typical LOD levels for buildings
LOD0: Full detail (windows, doors)  - 0-30m
LOD1: Medium detail (simplified)      - 30-80m
LOD2: Low detail (block shapes)      - 80-150m
LOD3: Very low (just boxes)         - 150m+
```

**Settings**:
```json
{
  "enableLOD": true,
  "lodBias": 0.1,
  "enableCrossfade": true,
  "crossfadeDuration": 0.3
}
```

### Use Case 2: Forest/Terrain

For natural environments with many trees/rocks:

```python
# Typical LOD levels for nature objects
LOD0: Full branches and leaves  - 0-20m
LOD1: Simplified foliage     - 20-60m
LOD2: Impostor sprite      - 60m+
```

**Settings**:
```json
{
  "enableLOD": true,
  "lodBias": -0.05,  // Keep higher quality slightly longer
  "enableCrossfade": false,  // No crossfade for impostors
  "crossfadeDuration": 0.1
}
```

### Use Case 3: Indoor Scene

For interior scenes with furniture:

```python
# Typical LOD levels for props
LOD0: Full detail        - 0-10m
LOD1: Medium detail     - 10-30m
```

**Settings**:
```json
{
  "enableLOD": true,
  "lodBias": 0.0,
  "enableCrossfade": true,
  "crossfadeDuration": 0.15
}
```

## Troubleshooting Quick Guide

| Problem | Cause | Solution |
|---------|---------|----------|
| LOD not switching | LOD disabled or no camera | Enable `enableLOD`, check camera in world |
| Visible popping | No crossfade, poor LOD quality | Enable `enableCrossfade`, improve LOD meshes |
| Wrong LOD level selected | Incorrect thresholds | Adjust `lodBias`, verify screen percentages |
| Performance impact | Too many LOD levels or crossfade | Reduce LOD levels, disable crossfade |
| Mesh doesn't load | Wrong mesh UUID | Check `.lod` file, verify mesh paths |

## Performance Tips

1. **Optimize Mesh Complexity**:
   - LOD0: Full quality needed
   - LOD1: ~50% of LOD0
   - LOD2: ~25% of LOD0
   - LOD3+: ~12.5% of previous

2. **Use Crossfade Wisely**:
   - Enabled: Better visuals, slightly higher cost
   - Disabled: Better performance, visible popping

3. **Adjust LOD Bias**:
   - Lower bias for high-quality machines
   - Higher bias for low-end devices

4. **Profile Your Scene**:
   - Use SkyEngine profiler to measure impact
   - Test on target hardware

## Next Steps

Once you're comfortable with the basics:

1. **Read Full Documentation**: See `docs/LODSystem.md` for details
2. **Experiment with Settings**: Try different LOD configurations
3. **Create Custom LODs**: Model your own LOD meshes
4. **Integrate with Game Logic**: Adjust LOD based on gameplay events
5. **Optimize Performance**: Profile and fine-tune for your target platform

## Example: Complete LOD Setup

Here's a complete example of setting up a LOD object:

```cpp
// Create actor with LOD
auto actor = world->CreateActor();
auto meshComp = actor->AddComponent<StaticMeshComponent>();
auto lodComp = actor->AddComponent<LODComponent>();
auto debugComp = actor->AddComponent<LODDebugComponent>();

// Load prefab with LODs
auto prefab = AssetManager::Get()->LoadAsset<Prefab>("Prefabs/Building.prefab");
prefab->BlockUntilLoaded();
meshComp->SetPrefab(prefab->GetUuid());

// Load LOD configuration
auto lodConfig = AssetManager::Get()->LoadAsset("Prefabs/Building.lod");
lodConfig->BlockUntilLoaded();

// Configure LOD levels (usually auto-loaded from .lod file)
std::vector<LODLevel> levels = {
    { /* LOD0 */ uuid0, 1.0f, 5000, 3000, center, radius },
    { /* LOD1 */ uuid1, 0.5f, 2000, 1200, center, radius },
    { /* LOD2 */ uuid2, 0.25f, 800, 500, center, radius }
};
lodComp->SetLODLevels(levels);

// Enable debug visualization
debugComp->SetEnableDebugOverlay(true);
debugComp->SetEnableWireframe(true);

// Adjust LOD behavior
lodComp->SetLODBias(0.05f);
lodComp->SetEnableCrossfade(true);
lodComp->SetCrossfadeDuration(0.2f);

// Place in scene
actor->SetTransform(Transform{{0, 0, 0}, Quaternion::Identity(), {1, 1, 1}});
```

## Additional Resources

- **Full Documentation**: `docs/LODSystem.md`
- **Build Instructions**: `docs/LODBuildInstructions.md`
- **Test Scene**: `assets/Scenes/LODTestScene.json`
- **Test Script**: `scripts/test_lod.py`

## Getting Help

If you encounter issues:

1. Check this guide for common solutions
2. Read `docs/LODSystem.md` for detailed explanations
3. Review `docs/LODBuildInstructions.md` for build issues
4. Check GitHub issues: https://github.com/SkyEngine/SkyEngine/issues
5. Ask in community forums

---

**Happy LOD-ing!** 🚀

Remember: Good LOD balance comes from practice and iteration. Don't be afraid to experiment with different settings to find what works best for your project.
