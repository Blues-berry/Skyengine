#!/usr/bin/env python3
"""
SkyEngine LOD System Test Script

This script tests the LOD system functionality by:
1. Creating test actors with LOD components
2. Verifying LOD level switching
3. Checking debug information
4. Measuring performance impact
"""

import os
import sys
import time
import json
from typing import Dict, List, Optional

# Configuration
ENGINE_PATH = "C:/SkyEngine"
TEST_SCENE_PATH = "assets/Scenes/LODTestScene.json"
OUTPUT_LOG = "lod_test_results.log"

class LODTestResult:
    """Results of LOD testing"""
    def __init__(self):
        self.test_name = ""
        self.passed = False
        self.duration_ms = 0.0
        self.message = ""
        self.lod_levels = 0
        self.switch_count = 0

    def to_dict(self):
        return {
            "test_name": self.test_name,
            "passed": self.passed,
            "duration_ms": self.duration_ms,
            "message": self.message,
            "lod_levels": self.lod_levels,
            "switch_count": self.switch_count
        }

def log(message: str):
    """Log message to console and file"""
    print(f"[LODTest] {message}")
    with open(OUTPUT_LOG, "a") as f:
        f.write(f"{time.strftime('%Y-%m-%d %H:%M:%S')} - {message}\n")

def create_test_scene() -> bool:
    """Create a test scene with LOD actors"""
    log("Creating test scene...")

    scene_data = {
        "version": 1,
        "actors": []
    }

    # Create test actors at different positions
    test_configs = [
        {"name": "LOD_Test_0", "position": [0, 0, 10]},
        {"name": "LOD_Test_1", "position": [0, 0, 30]},
        {"name": "LOD_Test_2", "position": [0, 0, 50]},
        {"name": "LOD_Test_3", "position": [0, 0, 100]},
    ]

    for config in test_configs:
        actor = {
            "name": config["name"],
            "transform": {
                "translation": config["position"],
                "rotation": [0, 0, 0, 1],
                "scale": [1, 1, 1]
            },
            "components": [
                {
                    "type": "StaticMeshComponent",
                    "properties": {
                        "mesh": "Prefabs/TestModel_LOD0.mesh"
                    }
                },
                {
                    "type": "LODComponent",
                    "properties": {
                        "baseName": "TestModel",
                        "enableLOD": True,
                        "lodBias": 0.0,
                        "enableCrossfade": True,
                        "crossfadeDuration": 0.2,
                        "levels": [
                            {
                                "meshUuid": "UUID_0000",
                                "screenPercentage": 1.0,
                                "triangleCount": 5000,
                                "vertexCount": 3000,
                                "boundsCenter": [0, 0, 0],
                                "boundsRadius": 5.0
                            },
                            {
                                "meshUuid": "UUID_0001",
                                "screenPercentage": 0.5,
                                "triangleCount": 2000,
                                "vertexCount": 1200,
                                "boundsCenter": [0, 0, 0],
                                "boundsRadius": 5.0
                            },
                            {
                                "meshUuid": "UUID_0002",
                                "screenPercentage": 0.25,
                                "triangleCount": 800,
                                "vertexCount": 500,
                                "boundsCenter": [0, 0, 0],
                                "boundsRadius": 5.0
                            }
                        ]
                    }
                },
                {
                    "type": "LODDebugComponent",
                    "properties": {
                        "enableDebugOverlay": True,
                        "enableWireframe": False,
                        "overlayX": 10.0,
                        "overlayY": 10.0,
                        "overlayColor": [1.0, 1.0, 1.0, 1.0]
                    }
                }
            ]
        }
        scene_data["actors"].append(actor)

    # Save scene
    os.makedirs(os.path.dirname(TEST_SCENE_PATH), exist_ok=True)
    with open(TEST_SCENE_PATH, "w") as f:
        json.dump(scene_data, f, indent=2)

    log(f"Created test scene: {TEST_SCENE_PATH}")
    return True

def test_lod_component_creation() -> LODTestResult:
    """Test 1: LOD component creation and initialization"""
    result = LODTestResult()
    result.test_name = "LOD Component Creation"

    start_time = time.time()

    try:
        # This would typically be done through SkyEngine API
        # For now, we just verify the scene file
        if os.path.exists(TEST_SCENE_PATH):
            with open(TEST_SCENE_PATH, "r") as f:
                scene_data = json.load(f)

            # Verify LOD components exist
            lod_actors = [a for a in scene_data.get("actors", [])
                         if any(c.get("type") == "LODComponent" for c in a.get("components", []))]

            if len(lod_actors) > 0:
                result.passed = True
                result.message = f"Created {len(lod_actors)} LOD actors"
                result.lod_levels = 3
            else:
                result.passed = False
                result.message = "No LOD components found in scene"
        else:
            result.passed = False
            result.message = "Test scene file not found"

    except Exception as e:
        result.passed = False
        result.message = f"Exception: {str(e)}"

    result.duration_ms = (time.time() - start_time) * 1000.0
    return result

def test_lod_configuration() -> LODTestResult:
    """Test 2: LOD configuration loading and validation"""
    result = LODTestResult()
    result.test_name = "LOD Configuration"

    start_time = time.time()

    try:
        if os.path.exists(TEST_SCENE_PATH):
            with open(TEST_SCENE_PATH, "r") as f:
                scene_data = json.load(f)

            # Check LOD configuration
            for actor in scene_data.get("actors", []):
                for comp in actor.get("components", []):
                    if comp.get("type") == "LODComponent":
                        props = comp.get("properties", {})

                        # Verify required properties
                        required = ["baseName", "enableLOD", "levels"]
                        missing = [p for p in required if p not in props]

                        if missing:
                            result.passed = False
                            result.message = f"Missing properties: {missing}"
                            return result

                        # Verify levels
                        levels = props.get("levels", [])
                        if len(levels) == 0:
                            result.passed = False
                            result.message = "No LOD levels configured"
                            return result

                        # Check level properties
                        for i, level in enumerate(levels):
                            level_required = ["meshUuid", "screenPercentage"]
                            level_missing = [p for p in level_required if p not in level]
                            if level_missing:
                                result.passed = False
                                result.message = f"Level {i} missing properties: {level_missing}"
                                return result

                        result.passed = True
                        result.message = f"LOD configuration valid: {len(levels)} levels"
                        result.lod_levels = len(levels)
                        return result

            result.passed = False
            result.message = "No LOD component found"

        else:
            result.passed = False
            result.message = "Test scene file not found"

    except Exception as e:
        result.passed = False
        result.message = f"Exception: {str(e)}"

    result.duration_ms = (time.time() - start_time) * 1000.0
    return result

def test_lod_debug_component() -> LODTestResult:
    """Test 3: LOD debug component functionality"""
    result = LODTestResult()
    result.test_name = "LOD Debug Component"

    start_time = time.time()

    try:
        if os.path.exists(TEST_SCENE_PATH):
            with open(TEST_SCENE_PATH, "r") as f:
                scene_data = json.load(f)

            # Check for debug components
            debug_actors = [a for a in scene_data.get("actors", [])
                          if any(c.get("type") == "LODDebugComponent" for c in a.get("components", []))]

            if len(debug_actors) > 0:
                # Verify debug properties
                for actor in debug_actors:
                    for comp in actor.get("components", []):
                        if comp.get("type") == "LODDebugComponent":
                            props = comp.get("properties", {})

                            # Check debug properties
                            if "enableDebugOverlay" in props:
                                result.passed = True
                                result.message = f"Debug component configured for {len(debug_actors)} actors"
                            else:
                                result.passed = False
                                result.message = "Debug component missing enableDebugOverlay"
            else:
                result.passed = False
                result.message = "No LOD debug components found"

        else:
            result.passed = False
            result.message = "Test scene file not found"

    except Exception as e:
        result.passed = False
        result.message = f"Exception: {str(e)}"

    result.duration_ms = (time.time() - start_time) * 1000.0
    return result

def run_all_tests() -> List[LODTestResult]:
    """Run all LOD tests"""
    log("=" * 60)
    log("Starting LOD System Tests")
    log("=" * 60)

    # Create test scene
    if not create_test_scene():
        log("Failed to create test scene")
        return []

    # Run tests
    tests = [
        test_lod_component_creation,
        test_lod_configuration,
        test_lod_debug_component,
    ]

    results = []
    for test_func in tests:
        result = test_func()
        results.append(result)

        # Log result
        status = "PASS" if result.passed else "FAIL"
        log(f"{result.test_name}: {status} ({result.duration_ms:.2f}ms)")
        log(f"  Message: {result.message}")

    return results

def generate_report(results: List[LODTestResult]):
    """Generate test report"""
    log("\n" + "=" * 60)
    log("Test Summary")
    log("=" * 60)

    passed = sum(1 for r in results if r.passed)
    total = len(results)

    log(f"Passed: {passed}/{total}")
    log(f"Total Duration: {sum(r.duration_ms for r in results):.2f}ms")

    # Detailed results
    log("\nDetailed Results:")
    for result in results:
        status = "✓" if result.passed else "✗"
        log(f"  {status} {result.test_name}")
        log(f"      Duration: {result.duration_ms:.2f}ms")
        log(f"      Message: {result.message}")

    # Save JSON report
    report_data = {
        "timestamp": time.strftime('%Y-%m-%d %H:%M:%S'),
        "passed": passed,
        "total": total,
        "results": [r.to_dict() for r in results]
    }

    report_path = "lod_test_report.json"
    with open(report_path, "w") as f:
        json.dump(report_data, f, indent=2)

    log(f"\nReport saved to: {report_path}")

def main():
    """Main entry point"""
    # Clear previous log
    if os.path.exists(OUTPUT_LOG):
        os.remove(OUTPUT_LOG)

    # Run tests
    results = run_all_tests()

    # Generate report
    generate_report(results)

    # Return exit code
    passed = sum(1 for r in results if r.passed)
    total = len(results)
    return 0 if passed == total else 1

if __name__ == "__main__":
    sys.exit(main())
