/////////////////////////////////////////////////////////////////////
// Filename: vulkanTools.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include <vector>

#include "vulkanInitializers.h" //< includes vulkan.h
#include "eedefs.h"

////////////
// MACROS //
////////////
/* @brief Checks the result and prints the its code if it is an error (can be coerced by defining EE_FORCE_VK_CHECK) */
#if defined(_DEBUG) | defined(EE_FORCE_VK_CHECK)
# define VK_CHECK(f) \
{\
  VkResult res = f;\
  if(res != VK_SUCCESS) {\
    printf_s("VK_CHECK: VkResult is \"%s\" %d at line %d in %s\n", ::EE::vulkan::tools::vkResultToStr(res), res, __LINE__, __FILE__);\
    assert(res == VK_SUCCESS);\
  }\
}
#else
# define VK_CHECK(f) f;
#endif

/* @brief Object specific */
#define EE_INFO_EXEC_BUFFER(msg, ...) EE_INFO("[EXEC_BUFFER:%p] %s\n", this, msg,##__VA_ARGS__)
#define EE_PRINT_EXEC_BUFFER(msg, ...) EE_PRINTA("[EXEC_BUFFER:%p] %s\n", this, msg,##__VA_ARGS__)

/* @brief If no flag is set for example when passed as a param, this is more readable */
#define VK_FLAGS_NONE 0

/* @brief Default fence timeout in nanoseconds */
#define DEFAULT_FENCE_TIMEOUT 100000000000

/* @brief Gets the function pointer of name and calls it with the variadic arguments passed in */
#define VK_FUNCTION_CALL(instance, name, ...) (reinterpret_cast<PFN_##name>(vkGetInstanceProcAddr(instance, #name)))(__VA_ARGS__);

/* @brief Just gets the function with the name passed in */
#define VK_FUNCTION(instance, name) reinterpret_cast<PFN_##name>(vkGetInstanceProcAddr(instance, #name));

///////////////////////////
// FOREWARD DECLARATIONS //
///////////////////////////
enum EEFormat;
enum EEShaderStage;
enum EEDescriptorType;

namespace EE {

	namespace vulkan {
		struct SurfaceDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		namespace tools {

			extern VkFormat eeToVk(EEFormat format);
			extern VkShaderStageFlags eeToVk(EEShaderStage shaderStage);
			extern VkDescriptorType eeToVk(EEDescriptorType descriptorType);

			extern bool isStencilFormat(VkFormat format);

			extern char const* vkResultToStr(VkResult);

			extern void imageBarrier(
				VkCommandBuffer				cmdBuffer,
				VkImage               image,
				VkImageAspectFlags    aspectMask,
				uint32_t              mipLevels,
				VkImageLayout         oldLayout,
				VkImageLayout         newLayout,
				VkPipelineStageFlags  srcStageMask,
				VkPipelineStageFlags  dstStageMask);

			extern void bufferImageCopy(
				VkCommandBuffer     cmdBuffer,
				VkBuffer            srcBuffer,
				VkImage             dstImage,
				VkImageLayout       dstImageLayout,
				VkImageAspectFlags  aspectMask,
				uint32_t            width,
				uint32_t            height);

			extern void generateMipmaps(
				VkCommandBuffer cmdBuffer,
				VkImage         image,
				VkFormat        imageFormat,
				uint32_t        width,
				uint32_t        height,
				uint32_t        mipLevels);
		}
	}
}
