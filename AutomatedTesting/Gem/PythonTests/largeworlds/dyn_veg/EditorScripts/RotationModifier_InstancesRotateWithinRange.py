"""
Copyright (c) Contributors to the Open 3D Engine Project.
For complete copyright and license terms please see the LICENSE at the root of this distribution.

SPDX-License-Identifier: Apache-2.0 OR MIT
"""

import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import azlmbr.math as math
import azlmbr.legacy.general as general
import azlmbr.bus as bus
import azlmbr.areasystem as areasystem

sys.path.append(os.path.join(azlmbr.paths.devroot, 'AutomatedTesting', 'Gem', 'PythonTests'))
import editor_python_test_tools.hydra_editor_utils as hydra
from editor_python_test_tools.editor_test_helper import EditorTestHelper
from largeworlds.large_worlds_utils import editor_dynveg_test_helper as dynveg


class TestRotationModifier_InstancesRotateWithinRange(EditorTestHelper):
    def __init__(self):
        EditorTestHelper.__init__(self, log_prefix="RotationModifier_InstancesRotateWithinRange", args=["level"])

    def run_test(self):
        """
        Summary: Range Min/Max in the Vegetation Rotation Modifier component can be set for all axes,
            and functions as expected when fed a gradient signal

        Vegetation Entity: Set in the middle of the level it holds a child entity and the following components:
            Vegetation Asset List
            Box Shape (size: <10, 10, 10>)
            Vegetation Layer Spawner
            Vegetation Rotation Modifier
                Rotation X (gradient: child, Range Min: Variable, Range Max: Variable)
                Rotation Y (gradient: child, Range Min: Variable, Range Max: Variable)
                Rotation Z (gradient: child, Range Min: Variable, Range Max: Variable)


        Child Entity: Child to Vegetation Entity has the following components:
            Box Shape (size: <10, 10, 10>)
            Gradient Transform Modifier
            Constant Gradient

        Expected Behavior: The vegetation area adjusts rotation based on the Constant Gradient component
            and the min and max values for each component. Min max of each axis is checked

        Test Steps:
        1) Create level
        2) Set up vegetation entities
        3) X-axis Check
        4) Y-axis Check
        5) Z-axis Check

        Note:
        - Any passed and failed tests are written to the Editor.log file.
                Parsing the file or running a log_monitor are required to observe the test results.

        :return: None
        """

        # Test Constants
        LEVEL_CENTER = math.Vector3(512.0, 512.0, 32.0)
        constant_gradient_value = 0.15

        # Helper Functions
        def change_range_max(axis, value):
            spawner_entity.get_set_test(3, f"Configuration|Rotation {axis}|Range Max", value)

        def change_range_min(axis, value):
            spawner_entity.get_set_test(3, f"Configuration|Rotation {axis}|Range Min", value)

        def get_expected_rotation(min, max, gradient_value):
            return min + ((max - min) * gradient_value)

        def validate_rotation(center, radius, num_expected, rot_degrees_vector):
            # Verify that every instance in the given area has the expected rotation. 
            box = math.Aabb_CreateCenterRadius(center, radius)
            instances = areasystem.AreaSystemRequestBus(bus.Broadcast, 'GetInstancesInAabb', box)
            num_found = len(instances)
            result = (num_found == num_expected)
            print(f'instance count validation: {result} (found={num_found}, expected={num_expected})')
            expected_rotation = math.Quaternion()
            expected_rotation.SetFromEulerDegrees(rot_degrees_vector)
            for instance in instances:
                result = result and instance.rotation.IsClose(expected_rotation)
                print(f'instance rotation validation: {result} (rotation={instance.rotation} expected={expected_rotation})')
            return result


        # Main Script
        # 1) Create Level
        self.test_success = self.create_level(
            self.args["level"],
            heightmap_resolution=1024,
            heightmap_meters_per_pixel=1,
            terrain_texture_resolution=4096,
            use_terrain=False,
        )
        general.set_current_view_position(512.0, 480.0, 38.0)

        # 2) Set up vegetation entities
        asset_path = os.path.join("Slices", "PurpleFlower.dynamicslice")
        spawner_entity = dynveg.create_vegetation_area("Spawner Entity", LEVEL_CENTER, 2.0, 2.0, 2.0, asset_path)

        additional_components = [
            "Vegetation Rotation Modifier"
        ]
        for component in additional_components:
            spawner_entity.add_component(component)

        # Create surface to spawn vegetation on
        dynveg.create_surface_entity("Surface Entity", LEVEL_CENTER, 10.0, 10.0, 1.0)

        # Create Gradient Entity
        gradient_entity = hydra.Entity("Gradient Entity")
        gradient_entity.create_entity(
            LEVEL_CENTER,
            ["Constant Gradient"],
            parent_id=spawner_entity.id
        )
        if gradient_entity.id.IsValid():
            self.log(f"'{gradient_entity.name}' created")
        gradient_entity.get_set_test(0, "Configuration|Value", constant_gradient_value)

        # Vegetation Rotation Modifier
        for axis in ["X", "Y", "Z"]:
            spawner_entity.get_set_test(
                3, f"Configuration|Rotation {axis}|Gradient|Gradient Entity Id", gradient_entity.id
            )

        # Set up constants used across all the rotation checks

        # Choose an area large enough to contain all of the instances we spawned.
        area_center = LEVEL_CENTER
        area_radius = 20.0
        # We're spawning a 2x2 area, which will have 3 rows of 3 instances due to default vegetation system spacing, so 
        # we should have a total of 9 instances.
        num_expected = 9
        
        # 3) X-axis check
        general.idle_wait(3.0)  # Allow mesh to load

        # baseline, verify that we initially have no rotation
        change_range_min("Z", 0.0)
        change_range_max("Z", 0.0)
        rotation_success = self.wait_for_condition(
            lambda: validate_rotation(area_center, area_radius, num_expected, math.Vector3(0.0, 0.0, 0.0)),
            5.0)
        self.test_success = self.test_success and rotation_success

        # Adjust x-axis range min / max to (-180, 0).  
        # Because we have a constant gradient of 0.25, our actual rotation should be (min + (max - min) * gradient),
        # or (-180 + (0 - -180) * 0.25)
        change_range_min("X", -180.0)
        rotation_degrees = get_expected_rotation(-180.0, 0.0, constant_gradient_value)
        rotation_success = self.wait_for_condition(
            lambda: validate_rotation(area_center, area_radius, num_expected, math.Vector3(rotation_degrees, 0.0, 0.0)),
            5.0)
        self.test_success = self.test_success and rotation_success

        # Set the min / max to (0, 90), with an expected result of (0 + (90 - 0) * 0.25)
        change_range_min("X", 0.0)
        change_range_max("X", 90.0)
        rotation_degrees = get_expected_rotation(0.0, 90.0, constant_gradient_value)
        rotation_success = self.wait_for_condition(
            lambda: validate_rotation(area_center, area_radius, num_expected, math.Vector3(rotation_degrees, 0.0, 0.0)),
            5.0)
        self.test_success = self.test_success and rotation_success

        change_range_max("X", 0.0)

        # 4) Y-axis check
        change_range_min("Y", -180.0)
        rotation_degrees = get_expected_rotation(-180.0, 0.0, constant_gradient_value)
        rotation_success = self.wait_for_condition(
            lambda: validate_rotation(area_center, area_radius, num_expected, math.Vector3(0.0, rotation_degrees, 0.0)),
            5.0)
        self.test_success = self.test_success and rotation_success

        change_range_min("Y", 0.0)
        change_range_max("Y", 90.0)
        rotation_degrees = get_expected_rotation(0.0, 90.0, constant_gradient_value)
        rotation_success = self.wait_for_condition(
            lambda: validate_rotation(area_center, area_radius, num_expected, math.Vector3(0.0, rotation_degrees, 0.0)),
            5.0)
        self.test_success = self.test_success and rotation_success

        change_range_max("Y", 0.0)

        # 5) Z-axis check
        change_range_min("Z", -180.0)
        rotation_degrees = get_expected_rotation(-180.0, 0.0, constant_gradient_value)
        rotation_success = self.wait_for_condition(
            lambda: validate_rotation(area_center, area_radius, num_expected, math.Vector3(0.0, 0.0, rotation_degrees)),
            5.0)
        self.test_success = self.test_success and rotation_success

        change_range_min("Z", 0.0)
        change_range_max("Z", 90.0)
        rotation_degrees = get_expected_rotation(0.0, 90.0, constant_gradient_value)
        rotation_success = self.wait_for_condition(
            lambda: validate_rotation(area_center, area_radius, num_expected, math.Vector3(0.0, 0.0, rotation_degrees)),
            5.0)
        self.test_success = self.test_success and rotation_success


test = TestRotationModifier_InstancesRotateWithinRange()
test.run()
