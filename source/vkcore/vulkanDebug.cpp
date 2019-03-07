/////////////////////////////////////////////////////////////////////
// Filename: vulkanDebug.cpp
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "vulkanDebug.h"

#include <cassert>

using namespace EE;

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugReportFlagsEXT                       flags,
	VkDebugReportObjectTypeEXT                  objectType,
	uint64_t                                    object,
	size_t                                      location,
	int32_t                                     messageCode,
	const char*                                 pLayerPrefix,
	const char*                                 pMessage,
	void*                                       pUserData)
{
	EE_PRINTA("[DBG_REPORT] %s\n", pMessage);
	return VK_FALSE;
}

vulkan::Debug::Debug(Instance const* pInstance)
	: pInstance(pInstance)
{
	assert(pInstance);

	// Acquire functions (pointer)
	fpCreateDebugReportCallbackEXT = VK_FUNCTION(*pInstance, vkCreateDebugReportCallbackEXT);
	fpDestroyDebugReportCallbackEXT = VK_FUNCTION(*pInstance, vkDestroyDebugReportCallbackEXT);
	fpDebugReportMessageEXT = VK_FUNCTION(*pInstance, vkDebugReportMessageEXT);
}

vulkan::Debug::~Debug()
{
	if (callback != VK_NULL_HANDLE) {
		fpDestroyDebugReportCallbackEXT(*pInstance, callback, pInstance->pAllocator);
	}
}

void vulkan::Debug::Create()
{
	assert(fpCreateDebugReportCallbackEXT && fpDestroyDebugReportCallbackEXT);

	VkDebugReportCallbackCreateInfoEXT cinfo;
	cinfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	cinfo.pNext = nullptr;
	cinfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
	cinfo.pfnCallback = debugCallback;
	cinfo.pUserData = nullptr;

	VK_CHECK(fpCreateDebugReportCallbackEXT(*pInstance, &cinfo, pInstance->pAllocator, &callback));
}