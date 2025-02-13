#
# Copyright (c) Contributors to the Open 3D Engine Project.
# For complete copyright and license terms please see the LICENSE at the root of this distribution.
#
# SPDX-License-Identifier: Apache-2.0 OR MIT
#
#

file(TO_CMAKE_PATH "$ENV{ATOM_PIX_PATH}" ATOM_PIX_PATH_CMAKE_FORMATTED)

if(EXISTS "${ATOM_PIX_PATH_CMAKE_FORMATTED}/include/WinPixEventRuntime/pix3.h")
    ly_add_external_target(
        NAME pix
        VERSION
        3RDPARTY_ROOT_DIRECTORY ${ATOM_PIX_PATH_CMAKE_FORMATTED}
        INCLUDE_DIRECTORIES include
    )
endif()


