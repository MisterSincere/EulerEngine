cmake_minimum_required (VERSION 3.0)
project (EulerEngine)

# Source Files
set(VULKAN source/vulkanDebug.h source/vulkanDebug.cpp source/vulkanDefs.h source/vulkanDefs.cpp)

set(EULERTEST source/main.cpp source/systemClass.h source/systemClass.cpp)

#FILTER
source_group ("VULKAN" FILES ${VULKAN})
source_group ("" FILES ${EULERTEST})

link_directories (
	$ENV{VKENGINE_VULKAN}/Lib
	$ENV{VKENGINE_GLFW}/lib-vc2015
	)

include_directories (
	$ENV{VKENGINE_GLM_PATH}
	$ENV{VKENGINE_VULKAN}/Include
	$ENV{VKENGINE_GLFW}/include
	)

# Add files
add_executable (EulerEngine ${EULERTEST} ${VULKAN})