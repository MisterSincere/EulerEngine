#--------------------------------------------------------------------------------------------------
#
# EULER ENGINE Project Setup
#
#--------------------------------------------------------------------------------------------------


# Source Files
set(EXTERNS stb_image.h)
set(VULKAN vulkanInitializers.h vulkanTools.h vulkanTools.cpp)
set(EULERENGINE applicationClass.h applicationClass.cpp windowClass.h windowClass.cpp graphicsClass.h graphicsClass.cpp eedefs.h keycodes.h)

# Filter
source_group ("EXTERNS" FILES ${EXTERNS})
source_group ("VULKAN" FILES ${VULKAN})
source_group ("" FILES ${EULERENGINE})

# Add target
add_library (EulerEngine ${EULERENGINE} ${VULKAN} ${EXTERNS})

