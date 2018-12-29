#--------------------------------------------------------------------------------------------------
#
# EULER ENGINE Project Setup
#
#--------------------------------------------------------------------------------------------------


# Source Files
set (EXTERNS stb_image.h)

set (VULKAN	vulkanRenderer.h	vulkanRenderer.cpp
			vulkanSwapchain.h	vulkanSwapchain.cpp
			vulkanDebug.h		vulkanDebug.cpp
			vulkanDevice.h		vulkanDevice.cpp
			vulkanInstance.h	vulkanInstance.cpp
			vulkanTools.h		vulkanTools.cpp
			vulkanInitializers.h)

set (EULERENGINEINTERN  windowClass.h windowClass.cpp graphicsClass.h graphicsClass.cpp eedefs.h keycodes.h)
set (EULERENGINEOUT eeObject.h		   eeObject.cpp
					applicationClass.h applicationClass.cpp)

# Filter
source_group ("EXTERNS" FILES ${EXTERNS})
source_group ("VULKAN" FILES ${VULKAN})
source_group ("INTERN" FILES ${EULERENGINEINTERN})
source_group ("" FILES ${EULERENGINEOUT})

# Add target
add_library (EulerEngine ${EULERENGINEOUT} ${EULERENGINEINTERN} ${VULKAN} ${EXTERNS})

