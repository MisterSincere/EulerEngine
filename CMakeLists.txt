cmake_minimum_required (VERSION 3.0)
project (EulerEngine)

# Source Files
set(VULKAN source/vulkanRenderer.h source/vulkanRenderer.cpp source/vulkanSwapChain.h source/vulkanSwapChain.cpp source/vulkanBuffer.h source/vulkanInitializers.h source/vulkanDevice.h source/vulkanDevice.cpp source/vulkanInstance.h source/vulkanInstance.cpp source/vulkanDebug.h source/vulkanDebug.cpp source/vulkanDefs.h source/vulkanDefs.cpp)
set(INTERNS source/eeInternDepthImage.cpp source/eeInternPipeline.cpp source/eeIntern.h )

set(WINDOW source/window.h source/window.cpp)

set(EULERENGINE source/eulerengine.h source/eulerengine.cpp source/eedefs.h source/keycodes.h)
set(EULERTEST source/main.cpp source/systemClass.h source/systemClass.cpp)

#FILTER
source_group ("EULERENGINE" FILES ${EULERENGINE})
source_group ("EULERENGINE\\WINDOW" FILES ${WINDOW})
source_group ("EULERENGINE\\VULKAN" FILES ${VULKAN})
source_group ("EULERENGINE\\VULKAN\\INTERNS" FILES ${INTERNS})
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
add_executable (EulerEngine ${EULERTEST} ${EULERENGINE} ${WINDOW} ${VULKAN} ${INTERNS})