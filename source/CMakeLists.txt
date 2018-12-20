#--------------------------------------------------------------------------------------------------
#
# EULER ENGINE Project Setup
#
#--------------------------------------------------------------------------------------------------


# Source Files
set(VULKAN vulkanRenderer.h vulkanRenderer.cpp vulkanSwapChain.h vulkanSwapChain.cpp vulkanBuffer.h vulkanInitializers.h vulkanDevice.h vulkanDevice.cpp vulkanInstance.h vulkanInstance.cpp vulkanDebug.h vulkanDebug.cpp vulkanTools.h vulkanTools.cpp)
set(INTERNS eeInternObject.cpp eeInternMesh.cpp eeInternTexture.cpp eeInternShader.cpp eeInternVertex.cpp eeInternDepthImage.cpp eeInternPipeline.cpp eeIntern.h )
set(EXTERNS tiny_obj_loader.h stb_image.h)
set(WINDOW window.h window.cpp)
set(EULERENGINE eulerengine.h eulerengine.cpp eedefs.h keycodes.h)
set(EULERTEST main.cpp systemClass.h systemClass.cpp)

# Filter
source_group ("EULERENGINE" FILES ${EULERENGINE})
source_group ("EULERENGINE\\EXTERNALS" FILES ${EXTERNS})
source_group ("EULERENGINE\\WINDOW" FILES ${WINDOW})
source_group ("EULERENGINE\\VULKAN" FILES ${VULKAN})
source_group ("EULERENGINE\\VULKAN\\INTERNS" FILES ${INTERNS})
source_group ("" FILES ${EULERTEST})

# Add target
add_library (EulerEngine ${EULERTEST} ${EULERENGINE} ${WINDOW} ${VULKAN} ${INTERNS} ${EXTERNS})

