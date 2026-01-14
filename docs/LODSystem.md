# SkyEngine LOD (Level of Detail) 系统文档

## 概述

SkyEngine LOD系统提供自动的多级细节切换功能，根据相机距离动态选择合适的网格精度，优化渲染性能。

## 核心特性

### 1. 自动LOD检测
- 支持UE HLOD命名格式: `Small_City_LVL_HLOD0_256m_767m_L0_X0_Y0`
- 支持标准LOD命名格式: `Building_LOD0`, `Building_LOD1`
- 自动生成LOD配置文件 (`.lod`)

### 2. 运行时LOD切换
- 基于屏幕占比计算LOD级别
- 支持LOD跨度混合 (Cross-fade)
- 可调整LOD偏移 (Bias)
- 支持强制指定LOD级别

### 3. 性能优化
- 多级网格自动管理
- 视锥裁剪
- 距离剔除

## 使用方法

### 方法1: 从UE导出自动导入 (推荐)

#### 1.1 UE导出设置
```
右键 Asset → Export → glTF Binary (.glb)
✓ Export Meshes
✓ Export Materials
✓ Export Textures
✓ Export LODs (如果可用)
```

#### 1.2 SkyEngine导入
```bash
# 命令行导入
asset_builder -p <project_path> -i <model_path>.glb

# 或在编辑器中
Asset Browser → Import → 选择.glb文件
```

#### 1.3 自动生成的LOD配置
导入后，系统会自动生成 `.lod` 配置文件:

```json
{
  "lodGroups": [
    {
      "baseName": "Small_City_LVL_HLOD0_256m_767m",
      "levels": [
        {
          "meshUuid": "uuid-1",
          "screenPercentage": 1.0,
          "triangleCount": 0,
          "vertexCount": 0,
          "boundsCenter": [0, 0, 0],
          "boundsRadius": 10.0
        },
        {
          "meshUuid": "uuid-2",
          "screenPercentage": 0.5,
          "triangleCount": 0,
          "vertexCount": 0,
          "boundsCenter": [0, 0, 0],
          "boundsRadius": 10.0
        }
      ]
    }
  ],
  "enableLOD": true,
  "lodBias": 0.0
}
```

### 方法2: 手动创建LOD对象

#### 2.1 创建Actor
在编辑器中创建Actor，添加 `LODComponent` 组件。

#### 2.2 配置LOD级别
在Inspector中设置:
- `EnableLOD`: 启用/禁用LOD系统
- `ForceLOD`: 强制使用特定LOD
- `ForcedLevel`: 强制的LOD级别 (0=最高质量)
- `LODBias`: LOD偏移 (正数=更高质量, 负数=更低质量)
- `CrossFadeDuration`: 跨度混合时间 (秒)

#### 2.3 添加LOD级别
```json
{
  "type": "LODComponent",
  "data": {
    "enableLOD": true,
    "lodBias": 0.0,
    "crossFadeDuration": 0.1,
    "levels": [
      {
        "meshUuid": "high-quality-mesh-uuid",
        "screenPercentage": 1.0,
        "triangleCount": 10000,
        "vertexCount": 5000
      },
      {
        "meshUuid": "medium-quality-mesh-uuid",
        "screenPercentage": 0.5,
        "triangleCount": 5000,
        "vertexCount": 2500
      },
      {
        "meshUuid": "low-quality-mesh-uuid",
        "screenPercentage": 0.25,
        "triangleCount": 1000,
        "vertexCount": 500
      }
    ]
  }
}
```

## LOD级别配置

### ScreenPercentage (屏幕占比阈值)
| LOD级别 | 默认阈值 | 说明 |
|---------|---------|------|
| LOD0 | 1.0 (100%) | 最高质量，相机近距离 |
| LOD1 | 0.5 (50%) | 中等质量 |
| LOD2 | 0.25 (25%) | 低质量 |
| LOD3 | 0.125 (12.5%) | 最低质量，远距离 |

### 屏幕占比计算
```
screenSize = (boundsRadius * 2) / (2 * tan(FOV/2) * distance)
```

## 性能优化建议

### 1. LOD级别数量
- 小型物体: 2-3个LOD级别
- 大型物体: 3-4个LOD级别
- 城市/地形: 4-8个LOD级别

### 2. 面数比例
```
LOD0: 100% (基础)
LOD1: 50% 面数
LOD2: 25% 面数
LOD3: 12.5% 面数
```

### 3. 切换距离
```
LOD0 → LOD1: 距离 < 10米
LOD1 → LOD2: 距离 10-30米
LOD2 → LOD3: 距离 > 30米
```

## 调试工具

### 1. 强制LOD
```cpp
// 在代码中
auto lodComp = actor->GetComponent<LODComponent>();
lodComp->SetForceLOD(true);
lodComp->SetForcedLevel(2);  // 强制显示LOD2
```

### 2. 调整LOD偏移
```cpp
lodComp->SetLODBias(0.5);  // 偏向更高质量
lodComp->SetLODBias(-0.5); // 偏向更低质量
```

### 3. 查看当前LOD
```cpp
uint32_t currentLevel = lodComp->GetCurrentLevel();
float fadeFactor = lodComp->GetCurrentLODFadeFactor();
```

## 与UE的兼容性

### UE HLOD命名规则
```
<Name>_HLOD<Level>_<MinDist>m_<MaxDist>m_L<LODLevel>_X<X>_Y<Y>
```

例如:
- `Small_City_LVL_HLOD0_256m_767m_L0_X0_Y0` → LOD0
- `Small_City_LVL_HLOD0_256m_767m_L1_X0_Y0` → LOD1

### 自动转换
SkyEngine的 `PrefabBuilder` 会自动检测并转换UE的HLOD格式:
1. 提取基础名称
2. 识别LOD级别 (L0, L1, L2...)
3. 计算屏幕占比阈值
4. 生成标准LOD配置

## 已知限制

### 当前版本
- ✓ 支持静态网格LOD
- ✓ 支持UE HLOD自动导入
- ✓ 运行时LOD切换
- ✓ 跨度混合 (基础版)
- ✗ 不支持骨骼网格LOD
- ✗ 不支持材质LOD
- ✗ 跨度混合需要材质支持

### 未来改进
- [ ] 骨骼网格LOD支持
- [ ] 材质LOD (简化材质)
- [ ] 更平滑的LOD过渡
- [ ] 自适应LOD (基于帧率)
- [ ] 屏幕空间LOD

## 故障排查

### 问题1: LOD不切换
**原因**:
- 没有配置正确的ScreenPercentage
- 对象太小或太大

**解决方案**:
```cpp
// 调试当前屏幕占比
auto lodComp = actor->GetComponent<LODComponent>();
// 在LODComponent::CalculateLODLevel()中添加日志
LOG_I("LOD", "Screen size: %.2f, Current LOD: %d", screenSize, targetLod);
```

### 问题2: LOD切换闪烁
**原因**:
- LOD切换太快
- ScreenPercentage设置不合理

**解决方案**:
```cpp
lodComp->SetCrossFadeDuration(0.3);  // 增加混合时间
lodComp->SetLODBias(0.1);           // 增加偏移
```

### 问题3: UE HLOD未检测
**原因**:
- 命名格式不匹配
- Assimp未正确导入

**解决方案**:
```cpp
// 检查导入的网格名称
for (uint32_t i = 0; i < scene->mNumMeshes; ++i) {
    LOG_I("Mesh", "Mesh name: %s", scene->mMeshes[i]->mName.C_Str());
}
```

## 示例场景

参见 `assets/Scenes/LODTestScene.json` 查看完整示例。

## API参考

### LODComponent

#### 主要方法
```cpp
// 设置LOD级别
void SetLODLevels(const std::vector<LODLevel> &levels);

// 添加单个LOD级别
void AddLODLevel(const Uuid &meshUuid, float screenPercentage);

// 设置当前LOD
void SetCurrentLevel(uint32_t level);

// 强制LOD
void SetForceLOD(bool force);
void SetForcedLevel(uint32_t level);

// LOD偏移
void SetLODBias(float bias);

// 跨度混合
void SetCrossFadeDuration(float duration);

// 查询
uint32_t GetCurrentLevel() const;
Uuid GetCurrentMeshUuid() const;
float GetCurrentLODFadeFactor() const;
```

#### 数据结构
```cpp
struct LODLevel {
    Uuid meshUuid;              // 网格UUID
    float screenPercentage;      // 屏幕占比阈值 (0.0-1.0)
    uint32_t triangleCount;      // 三角面数
    uint32_t vertexCount;        // 顶点数
    BoundingSphere bounds;       // 包围球
};

struct LODGroupData {
    std::vector<LODLevel> levels;    // 所有LOD级别
    uint32_t currentLevel = 0;        // 当前LOD级别
    float crossFadeDuration = 0.1f;   // 混合持续时间
    bool forceLOD = false;            // 强制LOD模式
    uint32_t forcedLevel = 0;         // 强制的LOD级别
    bool enableLOD = true;            // 启用LOD
    float lodBias = 0.0f;            // LOD偏移
};
```

## 版本历史

### v1.0.0 (2025-01-14)
- 初始LOD系统实现
- UE HLOD自动检测
- 运行时LOD切换
- 跨度混合 (基础版)
- LOD偏移和强制模式

## 贡献者
- SkyEngine Team

## 许可证
SkyEngine Internal License
