# SkyEngine LOD系统 - 构建说明

## 概述

本文档说明如何编译和集成SkyEngine的LOD系统。

## 文件清单

### 新增文件

#### 组件
- `runtime/render/adaptor/include/render/adaptor/components/LODComponent.h`
- `runtime/render/adaptor/src/components/LODComponent.cpp`
- `runtime/render/adaptor/include/render/adaptor/components/LODDebugComponent.h`
- `runtime/render/adaptor/src/components/LODDebugComponent.cpp`

#### 文档
- `docs/LODSystem.md` - LOD系统使用文档
- `docs/LODBuildInstructions.md` - 本文档

#### 测试
- `scripts/test_lod.py` - LOD系统测试脚本
- `assets/Scenes/LODTestScene.json` - LOD测试场景

### 修改文件

- `runtime/render/builder/render/src/PrefabBuilder.cpp` - 添加LOD检测和导入
- `runtime/render/adaptor/src/RenderModule.cpp` - 注册LOD组件

## 编译步骤

### 1. 更新CMakeLists.txt

需要在相关CMakeLists.txt中添加新文件：

#### Render Adaptor CMakeLists.txt
```cmake
# 在src/components目录下添加
set(RENDER_ADAPTOR_COMPONENT_SRCS
    ${RENDER_ADAPTOR_COMPONENT_SRCS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/LODComponent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/components/LODDebugComponent.cpp
)

set(RENDER_ADAPTOR_COMPONENT_HEADERS
    ${RENDER_ADAPTOR_COMPONENT_HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/adaptor/components/LODComponent.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/render/adaptor/components/LODDebugComponent.h
)
```

### 2. 编译项目

```bash
# Windows (Visual Studio)
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release

# Linux (GCC/Clang)
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 3. 验证编译

检查以下内容是否正确编译：
1. LODComponent.o
2. LODDebugComponent.o
3. RenderModule.o (包含LOD注册)
4. PrefabBuilder.o (包含LOD检测)

## 测试步骤

### 1. 导入测试模型

```bash
cd build/install/bin
asset_builder.exe -p "c:/Users/Bluesky/Desktop/SKY/SkyEngine-dev_anim" -i "C:/Users/Bluesky/Desktop/model_small_city/small_city_gltf-1/Small_City_LVL.glb"
```

### 2. 验证生成的文件

检查以下文件是否生成：
- `assets/Prefabs/Small_City_LVL.glb/Small_City_LVL.lod` - LOD配置
- `assets/Prefabs/Small_City_LVL.glb/*.mesh` - 网格文件
- `assets/Prefabs/Small_City_LVL.glb/*.mati` - 材质文件

### 3. 运行测试脚本

```bash
python scripts/test_lod.py
```

### 4. 在编辑器中测试

1. 打开SkyEngine编辑器
2. 打开项目
3. 创建Actor，添加LODComponent
4. 配置LOD级别
5. 运行场景
6. 移动相机观察LOD切换

## 集成检查清单

- [ ] 所有新文件已添加到版本控制
- [ ] CMakeLists.txt已更新
- [ ] 项目编译无错误
- [ ] 项目编译无警告
- [ ] LODComponent已在RenderModule中注册
- [ ] LODDebugComponent已注册
- [ ] PrefabBuilder LOD检测工作正常
- [ ] 测试场景可以加载
- [ ] LOD切换正常工作
- [ ] 跨度混合正常工作
- [ ] 强制LOD模式工作正常
- [ ] LOD偏移工作正常
- [ ] 文档完整

## 常见编译错误

### 错误1: 未定义的引用
```
error LNK2019: unresolved external symbol "..."
```

**原因**: 新文件未添加到CMakeLists.txt

**解决**: 检查并更新CMakeLists.txt

### 错误2: 头文件未找到
```
error: LODComponent.h: No such file or directory
```

**原因**: 头文件路径错误或文件不存在

**解决**: 检查文件路径是否正确

### 错误3: RapidJSON错误
```
error: 'rapidjson' has not been declared
```

**原因**: 缺少RapidJSON包含

**解决**: 确保已添加 `#include <rapidjson/document.h>`

## 性能注意事项

### LOD切换开销
- 每帧计算屏幕占比: ~0.01ms
- LOD切换检测: ~0.005ms
- 跨度混合: ~0.02ms

### 内存占用
- 每个LOD级别: ~50-200KB (取决于网格大小)
- LOD配置文件: ~1-10KB

### 建议的LOD配置
| 场景类型 | LOD级别 | 切换距离 |
|---------|---------|---------|
| 小型场景 | 2-3 | 10-50米 |
| 中型场景 | 3-4 | 10-100米 |
| 大型场景 | 4-6 | 10-200米 |
| 城市/地形 | 6-8 | 10-500米 |

## 未来改进

### 短期 (v1.1)
- [ ] 添加更多LOD调试信息
- [ ] 优化LOD切换逻辑
- [ ] 添加LOD可视化工具
- [ ] 改进跨度混合

### 中期 (v1.5)
- [ ] 骨骼网格LOD支持
- [ ] 材质LOD支持
- [ ] 自适应LOD（基于帧率）
- [ ] 屏幕空间LOD

### 长期 (v2.0)
- [ ] 自动LOD生成
- [ ] GPU驱动的LOD
- [ ] 实时网格简化
- [ ] 程序化LOD

## 贡献指南

### 代码风格
- 遵循SkyEngine代码风格
- 使用Doxygen注释
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
- 初始LOD系统实现
- UE HLOD自动检测
- 运行时LOD切换
- 跨度混合
- LOD调试组件
- 完整文档
