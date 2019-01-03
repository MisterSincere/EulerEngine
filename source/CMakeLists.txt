#--------------------------------------------------------------------------------------------------
#
# EULER ENGINE Project Setup
#
#--------------------------------------------------------------------------------------------------


# Source Files
set (EXTERNS vkcore/stb_image.h)


set (VULKAN		vkcore/vulkanObject.h		vkcore/vulkanObject.cpp
				vkcore/vulkanShader.h		vkcore/vulkanShader.cpp
				vkcore/vulkanPipeline.h		vkcore/vulkanPipeline.cpp
				vkcore/vulkanMesh.h			vkcore/vulkanMesh.cpp
				vkcore/vulkanResources.h	vkcore/vulkanResources.cpp)	
set	(VULKANCORE	vkcore/vulkanRenderer.h		vkcore/vulkanRenderer.cpp
				vkcore/vulkanSwapchain.h	vkcore/vulkanSwapchain.cpp
				vkcore/vulkanDebug.h		vkcore/vulkanDebug.cpp
				vkcore/vulkanDevice.h		vkcore/vulkanDevice.cpp
				vkcore/vulkanInstance.h		vkcore/vulkanInstance.cpp
				vkcore/vulkanTools.h		vkcore/vulkanTools.cpp
				vkcore/vulkanInitializers.h)

set (GFX		gfx/EEField.h		gfx/EEField.cpp
				gfx/EEIDrawable.h)

set (EULERENGINEINTERN  Window.h		Window.cpp
						Graphics.h		Graphics.cpp)
set (EULERENGINEOUT		eedefs.h		keycodes.h
						EEApplication.h	EEApplication.cpp)

# Filter
source_group ("EXTERNS" FILES ${EXTERNS})
source_group ("GFX" FILES ${GFX})
source_group ("INTERN" FILES ${EULERENGINEINTERN})
source_group ("VULKAN" FILES ${VULKAN})
source_group ("VULKAN\\CORE" FILES ${VULKANCORE})
source_group ("" FILES ${EULERENGINEOUT})

# Add target
add_library (EulerEngine ${EULERENGINEOUT}
						 ${EULERENGINEINTERN}
						 ${VULKAN}
						 ${VULKANCORE}
						 ${EXTERNS}
						 ${GFX})

include_directories (EulerEngine ${CMAKE_CURRENT_SOURCE_DIR})