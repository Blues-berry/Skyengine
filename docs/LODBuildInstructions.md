# SkyEngine LOD System - Build Instructions

## Overview

This document describes how to compile and integrate the SkyEngine LOD system.

## File清单

### 新增文件

#### 组件 (Components)
- `runtime/render/adaptor/include/render/adaptor/components/LODComponent.h`
- `runtime/render/adaptor/src/components/LODComponent.cpp`
- `runtime/render/adaptor/include/render/adaptor/components/LODDebugComponent.h`
- `runtime/render/adaptor/src/components/LODDebugComponent.cpp`

#### 文档 (Documentation)
- `docs/LODSystem.md` - LOD system usage documentation
- `docs/LODBuildInstructions.md` - This file
- `docs/LODQuickStart.md` - Quick start guide

#### 测试 (Testing)
- `scripts/test_lod.py` - LOD system test script
- `assets/Scenes/LODTestScene.json` - LOD test scene

### 修改文件 (Modified Files)

- `runtime/render/builder/render/src/PrefabBuilder.cpp` - Added LOD detection and import
- `runtime/render/adaptor/src/RenderModule.cpp` - Registered LOD components

## 编译步骤

### 1. 更新 CMakeLists.txt

CMakeLists.txt 文件已更新以包含新文件和依赖：

#### Render Adaptor CMakeLists.txt
```cmake
sky_add_library(TARGET RenderAdaptor STATIC
    ...
    LINK_LIBS
        RenderCore
        Animation
        ImGuiRender
        Framework
        3rdParty::rapidjson
    ...
)
```

#### RenderBuilder CMakeLists.txt
```cmake
sky_add_library(TARGET RenderBuilder.Static STATIC
    ...
    LINK_LIBS
        ...
        3rdParty::stb
        3rdParty::rapidjson
        ...
)
```

### 2. 编译项目

#### Windows (Visual Studio)
```cmd
cd build
cmake .. -G "Visual Studio 17 2022" -D3RD_PATH=C:/path/to/3rdparty
cmake --build . --config Release
```

#### Linux (GCC/Clang)
```bash
mkdir build && cd build
cmake .. -D3RD_PATH=/path/to/3rdparty
make -j$(nproc)
```

### 3. 验证编译

检查以下内容是否正确编译：

1. LODComponent.o
2. LODDebugComponent.o
3. RenderModule.o (包含 LOD 注册)
4. PrefabBuilder.o (包含 LOD 检测)

## 测试步骤

### 1. 导入测试模型

```cmd
cd build/install/bin
asset_builder.exe -p "c:/SkyEngine" -i "C:/path/to/model_with_LODs.glb"
```

### 2. 验证生成的文件

检查以下文件是否生成：

- `assets/Prefabs/model_with_LODs.glb/model_with_LODs.lod` - LOD 配置
- `assets/Prefabs/model_with_LODs.glb/*.mesh` - 网格文件
- `assets/Prefabs/model_with_LODs.glb/*.mati` - 材质文件

### 3. 运行测试脚本

```cmd
python scripts/test_lod.py
```

### 4. 在编辑器中测试

1. 打开 SkyEngine 编辑器
2. 打开项目
3. 创建 Actor，添加 LODComponent
4. 配置 LOD 级别
5. 运行场景
6. 移动相机观察 LOD 切换

## 集成检查清单

- [ ] 所有新文件已添加到版本控制
- [ ] CMakeLists.txt 已更新
- [ ] 项目编译无错误
- [ ] 项目编译无警告
- [ ] LODComponent 已在 RenderModule 中注册
- [ ] LODDebugComponent 已注册
- [ ] PrefabBuilder LOD 检测工作正常
- [ ] 测试场景可以加载
- [ ] LOD 切换正常工作
- [ ] 跨度混合正常工作
- [ ] 强制 LOD 模式工作正常
- [ ] LOD 偏移工作正常
- [ ] 文档完整

## 常见编译错误

### 错误 1: 未定义的引用
```
error LNK2019: unresolved external symbol "..."
```

**原因**: 新文件未添加到 CMakeLists.txt

**解决**: 检查并更新 CMakeLists.txt，确保所有新文件已包含

### 错误 2: 头文件未找到
```
error: LODComponent.h: No such file or directory
```

**原因**: 头文件路径错误或文件不存在

**解决**: 检查文件路径是否正确，确认文件已创建

### 错误 3: RapidJSON 错误
```
error: 'rapidjson' has not been declared
```

**原因**: 缺少 RapidJSON 包含或链接

**解决**:
1. 确保已添加 `#include <rapidjson/document.h>`
2. 检查 CMakeLists.txt 中是否包含 `3rdParty::rapidjson`

### 错误 4: Component 未注册
```
error: LODComponent not found in component factory
```

**原因**: 组件未在 RenderModule 中注册

**解决**: 检查 RenderModule.cpp 的 RegisterComponents() 函数

## 性能注意事项

### LOD 切换开销
- 每帧计算屏幕占比: ~0.01ms
- LOD 切换检测: ~0.005ms
- 跨度混合: ~0.02ms

### 内存占用
- 每个 LOD 级别: ~50-200KB (取决于网格大小)
- LOD 配置文件: ~1-10KB

### 推荐的 LOD 配置

| 场景类型 | LOD 级别 | 切换距离 |
|---------|---------|---------|
| 小型场景 | 2-3 | 10-50米 |
| 中型场景 | 3-4 | 10-100米 |
| 大型场景 | 4-6 | 10-200米 |
| 城市/地形 | 6-8 | 10-500米 |

## 未来改进

### 短期 (v1.1)
- [ ] 添加更多 LOD 调试信息
- [ ] 优化 LOD 切换逻辑
- [ ] 添加 LOD 可视化工具
- [ ] 改进跨度混合

### 中期 (v1.5)
- [ ] 骨骼网格 LOD 支持
- [ ] 材质 LOD 支持
- [ ] 自适应 LOD（基于帧率）
- [ ] 屏幕空间 LOD

### 长期 (v2.0)
- [ ] 自动 LOD 生成
- [ ] GPU 驱动的 LOD
- [ ] 实时网格简化
- [ ] 程序化 LOD

## 贡献指南

### 代码风格
- 遵循 SkyEngine 代码风格
- 使用 Doxygen 注释
- 包含单元测试

### 提交前检查
- [ ] 代码编译通过
- [ ] 测试脚本通过
- [ ] 文档已更新
- [ ] 无内存泄漏
- [ ] 性能测试通过

## 联系方式

- 项目主页: https://github.com/SkyEngine/SkyEngine
- 问题反馈: https://github.com/SkyEngine/SkyEngine/issues
- 文档: https://docs.skyengine.dev

## 版本历史

### v1.0.0 (2025-01-14)
- 初始 LOD 系统实现
- UE HLOD 自动检测
- 运行时 LOD 切换
- 跨度混合
- LOD 调试组件
- 完整文档
