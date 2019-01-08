#--------------------------------------------------------------------------------------------------
#
# EULER ENGINE Project Setup
#
#--------------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.2.3)

# Source Files
set (EXTERNS vkcore/stb_image.h)

set (SHADER ../assets/shader/color2D.vert ../assets/shader/color2D.frag
			../assets/shader/font.vert	  ../assets/shader/font.frag)

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
#add_definitions("-DEE_PRINT_INFORMATIONS")

include_directories (EulerEngine ${CMAKE_CURRENT_SOURCE_DIR})


# function that stores just the shader type appendix in the result
function(get_shader_type shader_string result)
	string(FIND ${shader_string} ".vert" POSITION REVERSE)
	if (${POSITION} EQUAL -1)
		string(FIND ${shader_string} ".frag" POSITION REVERSE)
		if (${POSITION} EQUAL -1)
			string(FIND ${shader_string} ".geom" POSITION REVERSE)
			if (${POSITION} EQUAL -1)
				message("unknown shader appendix")
				set(${result} "" PARENT_SCOPE)
			else()
				set(${result} "Geom" PARENT_SCOPE)
			endif()
		else()
			set(${result} "Frag" PARENT_SCOPE)
		endif()
	else()
		set(${result} "Vert" PARENT_SCOPE)
	endif()
endfunction()


# The default string including placeholders to replace with the shader in file and output file
set(DEF_STRING "$ENV{EE_VULKAN_PATH}\\Bin\\glslangValidator.exe -V ${CMAKE_CURRENT_SOURCE_DIR}/__IN_FILE__  -o ${CMAKE_CURRENT_SOURCE_DIR}/../assets/shader/__OUT_FILE__.spv\n")

foreach(CUR_SHADER ${SHADER})
	if(index EQUAL 0)
		continue()
	endif()
	
	# get just the shader file name of the the curshader var
	string(FIND ${CUR_SHADER} "/" INDEX_FIRST REVERSE)
	string(FIND ${CUR_SHADER} "." INDEX_LAST REVERSE)
	math(EXPR INDEX_FIRST "${INDEX_FIRST} + 1")
	math(EXPR LENGTH "${INDEX_LAST} - ${INDEX_FIRST}")
	string(SUBSTRING ${CUR_SHADER} ${INDEX_FIRST} ${LENGTH} OUTPUT_FILE_NAME)
	# append shader type dependend suffix
	get_shader_type(${CUR_SHADER} TYPE_SUFFIX)
	string(APPEND OUTPUT_FILE_NAME ${TYPE_SUFFIX})
	
	set(TMP_STRING ${DEF_STRING})
	string(REPLACE "__IN_FILE__" ${CUR_SHADER} TMP_STRING ${TMP_STRING})
	string(REPLACE "__OUT_FILE__" ${OUTPUT_FILE_NAME} TMP_STRING ${TMP_STRING})
	string(APPEND WRITE_STRING ${TMP_STRING})
endforeach()

# let us after all that string bullshittery finally store that lil peace of hard work and good will (hunting)
file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/../assets/shader/runCompiler64.bat ${WRITE_STRING})


