# SkyEngine LOD系统 - 快速开始指南

## 5分钟快速体验LOD

### 步骤1: 编译项目 (如果还未编译)

```bash
cd c:/Users/Bluesky/Desktop/SKY/SkyEngine-dev_anim
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### 步骤2: 导入带有LOD的模型

```bash
cd build/install/bin
asset_builder.exe -p "c:/Users/Bluesky/Desktop/SKY/SkyEngine-dev_anim" -i "C:/Users/Bluesky/Desktop/model_small_city/small_city_gltf-1/Small_City_LVL.glb"
```

**预期输出:**
```
Detected UE HLOD: Small_City_LVL_HLOD0_256m_767m_L0_X0_Y0_Material -> Base: Small_City_LVL_HLOD0_256m_767m, LOD: 0, Screen: 1.00
Detected UE HLOD: Small_City_LVL_HLOD0_256m_767m_L1_X0_Y0_Material -> Base: Small_City_LVL_HLOD0_256m_767m, LOD: 1, Screen: 0.50
...
Created LOD group 'Small_City_LVL_HLOD0_256m_767m' with 3 LOD levels
Saved LOD configuration: 3 LOD groups
```

### 步骤3: 验证生成的文件

检查以下目录是否包含LOD文件：
```
assets/Prefabs/Small_City_LVL.glb/
├── Small_City_LVL.lod       # LOD配置文件
├── Small_City_LVL.prefab    # 预制体
├── *.mesh                   # 网格文件
└── *.mati                   # 材质文件
```

### 步骤4: 查看LOD配置

打开 `Small_City_LVL.lod` 文件，查看生成的LOD配置：

```json
{
  "lodGroups": [
    {
      "baseName": "Small_City_LVL_HLOD0_256m_767m",
      "levels": [
        {
          "meshUuid": "...",
          "screenPercentage": 1.0,
          "triangleCount": 0,
          "vertexCount": 0,
          "boundsCenter": [0, 0, 0],
          "boundsRadius": 10.0
        },
        {
          "meshUuid": "...",
          "screenPercentage": 0.5,
          ...
        }
      ]
    }
  ],
  "enableLOD": true,
  "lodBias": 0.0
}
```

### 步骤5: 在编辑器中测试

1. **启动SkyEngine编辑器**
   ```bash
   cd build/install/bin
   SkyEngineEditor.exe
   ```

2. **创建LOD测试Actor**
   - 创建新的Actor
   - 添加 `LODComponent` 组件
   - 在Inspector中配置LOD级别

3. **添加场景**
   - 加载 `assets/Scenes/LODTestScene.json`
   - 或手动创建场景

4. **运行场景**
   - 按Play按钮运行
   - 移动相机观察LOD切换

5. **添加LOD调试** (可选)
   - 为Actor添加 `LODDebugComponent`
   - 在Inspector中启用 `AutoLog`
   - 查看控制台输出LOD信息

## 常用LOD配置

### 配置1: 简单3级LOD
```json
{
  "enableLOD": true,
  "lodBias": 0.0,
  "crossFadeDuration": 0.1,
  "levels": [
    {
      "meshUuid": "high-uuid",
      "screenPercentage": 1.0
    },
    {
      "meshUuid": "medium-uuid",
      "screenPercentage": 0.5
    },
    {
      "meshUuid": "low-uuid",
      "screenPercentage": 0.25
    }
  ]
}
```

### 配置2: 调试模式（强制LOD）
```json
{
  "enableLOD": true,
  "forceLOD": true,
  "forcedLevel": 1,  // 强制使用LOD1
  "levels": [...]
}
```

### 配置3: 高质量优先
```json
{
  "enableLOD": true,
  "lodBias": 0.5,  // 偏向高质量
  "crossFadeDuration": 0.2,
  "levels": [...]
}
```

### 配置4: 性能优先
```json
{
  "enableLOD": true,
  "lodBias": -0.5,  // 偏向性能
  "crossFadeDuration": 0.05,  // 快速切换
  "levels": [...]
}
```

## 调试技巧

### 1. 查看当前LOD
在代码中：
```cpp
auto lodComp = actor->GetComponent<LODComponent>();
uint32_t currentLevel = lodComp->GetCurrentLevel();
LOG_I("LOD", "当前LOD级别: %u", currentLevel);
```

### 2. 强制特定LOD
```cpp
lodComp->SetForceLOD(true);
lodComp->SetForcedLevel(2);  // 强制显示LOD2
```

### 3. 调整LOD偏移
```cpp
// 正数=更高质量，负数=更低质量
lodComp->SetLODBias(0.3);
```

### 4. 添加LOD调试组件
```cpp
// 在Actor上添加
auto debugComp = actor->CreateComponent<LODDebugComponent>();
debugComp->SetAutoLog(true);  // 每秒打印LOD信息
```

## 性能调优

### 场景1: 角色模型
```json
{
  "levels": [
    {"screenPercentage": 1.0, "triangles": 15000},
    {"screenPercentage": 0.5, "triangles": 7500},
    {"screenPercentage": 0.25, "triangles": 3000}
  ]
}
```

### 场景2: 建筑/环境
```json
{
  "levels": [
    {"screenPercentage": 1.0, "triangles": 10000},
    {"screenPercentage": 0.5, "triangles": 5000},
    {"screenPercentage": 0.25, "triangles": 2000},
    {"screenPercentage": 0.125, "triangles": 500}
  ]
}
```

### 场景3: 大型城市
```json
{
  "levels": [
    {"screenPercentage": 1.0, "triangles": 50000},
    {"screenPercentage": 0.5, "triangles": 25000},
    {"screenPercentage": 0.25, "triangles": 10000},
    {"screenPercentage": 0.125, "triangles": 5000},
    {"screenPercentage": 0.0625, "triangles": 2000}
  ]
}
```

## 故障排查

### 问题: LOD不切换
**检查:**
1. `enableLOD` 是否为true
2. `screenPercentage` 是否设置正确
3. 相机距离是否足够

**解决:**
```cpp
// 调整LOD偏移
lodComp->SetLODBias(0.2);

// 或强制LOD测试
lodComp->SetForceLOD(true);
lodComp->SetForcedLevel(1);
```

### 问题: LOD切换闪烁
**解决:**
```cpp
// 增加跨度混合时间
lodComp->SetCrossFadeDuration(0.3);

// 或调整LOD偏移避免快速切换
lodComp->SetLODBias(0.1);
```

### 问题: UE HLOD未检测
**检查:**
1. 网格命名是否正确
2. Assimp是否正确导入

**解决:**
查看导入日志，确认检测到LOD：
```
Detected UE HLOD: ...
```

## 下一步

- 阅读完整文档: `docs/LODSystem.md`
- 查看构建说明: `docs/LODBuildInstructions.md`
- 运行测试脚本: `scripts/test_lod.py`
- 查看示例场景: `assets/Scenes/LODTestScene.json`

## 获取帮助

- 文档: https://docs.skyengine.dev/lod
- 论坛: https://forum.skyengine.dev
- GitHub: https://github.com/SkyEngine/SkyEngine/issues

## 常见问题

**Q: LOD系统支持多少个LOD级别？**
A: 理论上无限制，建议2-8个。

**Q: 可以在运行时添加LOD级别吗？**
A: 可以，使用 `AddLODLevel()` 方法。

**Q: LOD系统会影响骨骼动画吗？**
A: 当前版本不支持骨骼网格LOD，计划在v1.5中添加。

**Q: 如何自定义LOD切换逻辑？**
A: 可以继承LODComponent并重写 `CalculateLODLevel()` 方法。

**Q: LOD系统支持Instance Mesh吗？**
A: 支持，每个实例会独立计算LOD。

**Q: 如何在编辑器中预览LOD？**
A: 使用LODDebugComponent，或强制特定LOD级别。
