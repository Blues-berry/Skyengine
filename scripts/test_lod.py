#!/usr/bin/env python3
"""
LOD系统测试脚本
用于验证SkyEngine的LOD功能
"""

import subprocess
import sys
import os
import json
from pathlib import Path

def run_command(cmd):
    """执行命令并返回输出"""
    print(f"执行: {cmd}")
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    print(result.stdout)
    if result.stderr:
        print(f"错误: {result.stderr}", file=sys.stderr)
    return result.returncode == 0

def check_file_exists(filepath):
    """检查文件是否存在"""
    if Path(filepath).exists():
        print(f"✓ 文件存在: {filepath}")
        return True
    else:
        print(f"✗ 文件不存在: {filepath}")
        return False

def validate_lod_config(lod_file):
    """验证LOD配置文件"""
    print(f"\n验证LOD配置: {lod_file}")
    if not check_file_exists(lod_file):
        return False
    
    with open(lod_file, 'r', encoding='utf-8') as f:
        try:
            config = json.load(f)
            
            # 检查必需字段
            required_fields = ['lodGroups', 'enableLOD', 'lodBias']
            for field in required_fields:
                if field not in config:
                    print(f"✗ 缺少字段: {field}")
                    return False
                print(f"✓ 字段存在: {field}")
            
            # 检查LOD组
            if 'lodGroups' in config and len(config['lodGroups']) > 0:
                print(f"✓ 找到 {len(config['lodGroups'])} 个LOD组")
                for i, group in enumerate(config['lodGroups']):
                    print(f"\n  LOD组 {i}:")
                    print(f"    - 基础名称: {group.get('baseName', 'N/A')}")
                    if 'levels' in group:
                        print(f"    - LOD级别数: {len(group['levels'])}")
                        for j, level in enumerate(group['levels']):
                            print(f"      LOD{j}:")
                            print(f"        - 网格UUID: {level.get('meshUuid', 'N/A')[:8]}...")
                            print(f"        - 屏幕占比: {level.get('screenPercentage', 0.0)}")
            else:
                print("✗ 没有LOD组")
                return False
            
            return True
        except json.JSONDecodeError as e:
            print(f"✗ JSON解析错误: {e}")
            return False

def main():
    """主函数"""
    print("=" * 60)
    print("SkyEngine LOD系统测试")
    print("=" * 60)
    
    # 配置
    project_path = Path("c:/Users/Bluesky/Desktop/SKY/SkyEngine-dev_anim")
    model_path = "C:/Users/Bluesky/Desktop/model_small_city/small_city_gltf-1/Small_City_LVL.glb"
    
    print(f"\n项目路径: {project_path}")
    print(f"模型路径: {model_path}")
    
    # 测试1: 检查文件存在
    print("\n" + "-" * 60)
    print("测试1: 检查文件存在")
    print("-" * 60)
    if not check_file_exists(model_path):
        print("✗ 模型文件不存在，请检查路径")
        return 1
    
    # 测试2: 导入模型
    print("\n" + "-" * 60)
    print("测试2: 导入模型（包含LOD）")
    print("-" * 60)
    asset_builder = project_path / "build" / "install" / "bin" / "asset_builder"
    
    if not check_file_exists(asset_builder):
        print(f"✗ asset_builder不存在，请先编译项目: {asset_builder}")
        return 1
    
    cmd = f'"{asset_builder}" -p "{project_path}" -i "{model_path}"'
    if not run_command(cmd):
        print("✗ 模型导入失败")
        return 1
    print("✓ 模型导入成功")
    
    # 测试3: 验证LOD配置文件
    print("\n" + "-" * 60)
    print("测试3: 验证生成的LOD配置")
    print("-" * 60)
    lod_config_path = project_path / "assets" / "Prefabs" / "Small_City_LVL.glb" / "Small_City_LVL.lod"
    
    if not validate_lod_config(lod_config_path):
        print("✗ LOD配置验证失败")
        return 1
    print("✓ LOD配置验证成功")
    
    # 测试4: 检查生成的网格文件
    print("\n" + "-" * 60)
    print("测试4: 检查生成的网格文件")
    print("-" * 60)
    prefab_dir = project_path / "assets" / "Prefabs" / "Small_City_LVL.glb"
    if not check_file_exists(prefab_dir):
        print(f"✗ Prefab目录不存在: {prefab_dir}")
        return 1
    
    mesh_files = list(prefab_dir.glob("*.mesh"))
    if len(mesh_files) == 0:
        print("✗ 没有生成任何.mesh文件")
        return 1
    else:
        print(f"✓ 找到 {len(mesh_files)} 个网格文件:")
        for mesh_file in mesh_files[:5]:  # 只显示前5个
            size_mb = mesh_file.stat().st_size / (1024 * 1024)
            print(f"  - {mesh_file.name}: {size_mb:.2f} MB")
        if len(mesh_files) > 5:
            print(f"  ... 还有 {len(mesh_files) - 5} 个文件")
    
    # 测试5: 列出所有资产
    print("\n" + "-" * 60)
    print("测试5: 列出所有导入的资产")
    print("-" * 60)
    cmd = f'"{asset_builder}" -p "{project_path}" -l'
    run_command(cmd)
    
    # 总结
    print("\n" + "=" * 60)
    print("测试总结")
    print("=" * 60)
    print("✓ 所有测试通过！")
    print("\n下一步:")
    print("1. 在SkyEngine编辑器中打开项目")
    print("2. 导入或使用LODTestScene.json场景")
    print("3. 运行场景并测试LOD切换")
    print("4. 调整相机距离观察LOD变化")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
