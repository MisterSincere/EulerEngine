#--------------------------------------------------------------------------------------------------
#
# EULER ENGINE Project Setup
#
#--------------------------------------------------------------------------------------------------


# Source Files
set (EXTERNS stb_image.h)


set (VULKAN		vulkanObject.h		vulkanObject.cpp
				vulkanShader.h		vulkanShader.cpp
				vulkanPipeline.h	vulkanPipeline.cpp
				vulkanMesh.h		vulkanMesh.cpp
				vulkanResources.h	vulkanResources.cpp)	
set	(VULKANCORE	vulkanRenderer.h	vulkanRenderer.cpp
				vulkanSwapchain.h	vulkanSwapchain.cpp
				vulkanDebug.h		vulkanDebug.cpp
				vulkanDevice.h		vulkanDevice.cpp
				vulkanInstance.h	vulkanInstance.cpp
				vulkanTools.h		vulkanTools.cpp
				vulkanInitializers.h)

set (EULERENGINEINTERN  windowClass.h		windowClass.cpp
						graphicsClass.h		graphicsClass.cpp)
set (EULERENGINEOUT		eedefs.h			keycodes.h
						applicationClass.h	applicationClass.cpp)

# Filter
source_group ("EXTERNS" FILES ${EXTERNS})
source_group ("VULKAN" FILES ${VULKAN})
source_group ("VULKAN\\CORE" FILES ${VULKANCORE})
source_group ("INTERN" FILES ${EULERENGINEINTERN})
source_group ("" FILES ${EULERENGINEOUT})

# Add target
add_library (EulerEngine ${EULERENGINEOUT}
						 ${EULERENGINEINTERN}
						 ${VULKAN}
						 ${VULKANCORE}
						 ${EXTERNS})

