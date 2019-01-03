#--------------------------------------------------------------------------------------------------
#
# EULER ENGINE Project Setup
#
#--------------------------------------------------------------------------------------------------


# Source Files
set (EXTERNS vkcore/stb_image.h)

set (SHADER ../assets/shader/colorShader2D.vert ../assets/shader/colorShader2D.frag)

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

set (GFX		gfx/EERectangle.h		gfx/EERectangle.cpp)

set (EULERENGINEINTERN  Window.h		Window.cpp
						Graphics.h		Graphics.cpp)
set (EULERENGINEOUT		eedefs.h		keycodes.h
						EEApplication.h	EEApplication.cpp)

# Filter
source_group ("EXTERNS" FILES ${EXTERNS})
source_group ("GFX" FILES ${GFX})
source_group ("GFX\\SHADER" FILES ${SHADER})
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
						 ${GFX}
						 ${SHADER})

						 
add_definitions("-DEE_CMAKE_ASSETS_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/../assets/\"")

include_directories (EulerEngine ${CMAKE_CURRENT_SOURCE_DIR})


file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/../assets/shader/runCompiler64.bat
"$ENV{EE_VULKAN_PATH}\\Bin\\glslangValidator.exe -V colorShader2D.vert -o color2DVert.spv
$ENV{EE_VULKAN_PATH}\\Bin\\glslangValidator.exe -V colorShader2D.frag -o color2DFrag.spv
PAUSE")