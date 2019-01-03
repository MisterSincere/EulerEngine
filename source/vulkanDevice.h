/////////////////////////////////////////////////////////////////////
// Filename: vulkanDevice.h
//
// (C) Copyright 2019 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanDebug.h" //< vulkanInstance.h vulkanTools.h, vulkanInitializers.h, vulkan.h

namespace EE
{
	namespace vulkan
	{
		///////////////////////////
		// FOREWARD DECLARATIONS //
		///////////////////////////
		struct Device;

		/* @brief Describes different queue types in flags that can be set */
		enum QueueTypeFlagBits {
			PRESENT_FAMILY	= 0x01,
			GRAPHICS_FAMILY = 0x02,
			TRANSFER_FAMILY = 0x04,
			COMPUTE_FAMILY	= 0x08,
		};
		typedef uint32_t QueueTypes;


		//-------------------------------------------------------------------
		// ExecBuffer
		//-------------------------------------------------------------------
		/* The use of this struct wrapping around a simple command buffer is
		 * that the Vulkan Device will control which queue this command buffer is submitted to.
		 * The execution can be modified by passing in a pointer to a submit info.
		 * A nullptr will use a default submit info and wait for completed execution.
		 */
		struct ExecBuffer
		{
			/* @note Pending state needs to be checked manually since this is not an updateable structure.
			 * Also worth noticing is that recording/executable state have both the created bit set.
			 */
			enum STATE {
				ALLOCATED		= 0x01,
				CREATED			= 0x02,
				RECORDING		= 0x04 | CREATED, //0x06
				EXECUTABLE	= 0x08 | CREATED	//0x0A
			};

			/* @brief Pointer to the device this buffer uses and executes on */
			Device const* pDevice;
			/* @brief Acquired queue from the device this buffer will execute on */
			VkQueue queue;
			/* @brief The command buffer that this struct is wrapping */
			VkCommandBuffer cmdBuffer;
			/* @brief Fence that is signaled when the execution has finished */
			VkFence fence;

			/* @brief Indicates the current available use of this buffer */
			STATE currentState{ ALLOCATED };

			operator VkCommandBuffer() { return cmdBuffer; }

			/**
			 * Default constructor
			 **/
			ExecBuffer() {}

			/**
			 * Does already create everything needed for recording. After this constructor
			 * the buffer is in the CREATED state
			 * @note For further information check the Create() since this is the one used
			 **/
			ExecBuffer(Device const* pDevice, VkCommandBufferLevel level, bool begin = false, VkCommandBufferUsageFlags usageFlags = 0);

			/* @brief Destructor: calls Release() */
			~ExecBuffer();

			/**
			 * Creates this execbuffer and puts it into the created state making it
			 * possible to begin recording etc.
			 *
			 * @param pDevice			Pointer to the device on which this buffer will execute
			 * @param level				Desired level of the cmd buffer
			 * @param begin				If true the state of this buffer is RECORDING (defaults)
			 * @param usageFlags	When beginning this buffer you can pass some usageFlags too (defaults)
			 **/
			void Create(Device const* pDevice, VkCommandBufferLevel level, bool begin = false, VkCommandBufferUsageFlags usageFlags = 0);

			/* @brief If created it waits 'till end of execution and destroys F***ING EVERYTHING */
			void Release();

			/* @brief Changes state to RECORDING */
			void BeginRecording(VkCommandBufferUsageFlags usageFlags);

			/* @brief Changes state to EXECUTABLE since Execute() can now be called */
			void EndRecording();

			/**
			 * Executes either with a predefined submitinfo or the one passed in.
			 * Will return immediately, unless wait is true
			 *
			 * @param pSubmitInfo		Pointer to an optional submit info (defaults)
			 * @param wait					If true this method will only return after execution finish
			 **/
			void Execute(VkSubmitInfo* pSubmitInfo = nullptr, bool wait = true);

			/**
			 * Is only returning after the fence has been signaled or the time passed in has been expired.
			 *
			 * @param timeout		Maximal time to wait
			 **/
			void Wait(uint64_t timeout = DEFAULT_FENCE_TIMEOUT) const;
		};


		//-------------------------------------------------------------------
		// Device
		//-------------------------------------------------------------------
		struct Device
		{
			Instance* pInstance;
			EE::Window* pWindow;

			/* @brief The representation of the picked physical device (gpu) */
			VkPhysicalDevice physicalDevice;
			/* @brief The allocation callbacks used */
			VkAllocationCallbacks const* pAllocator;
			/* @brief Logical representation of the gpu (application's view of the gpu) */
			VkDevice logicalDevice;
			/* @brief Properties of the gpu including limits that the application may/should check against */
			VkPhysicalDeviceProperties properties;
			/* @brief Struct holding booleans indicating which features the gpu does supported */
			VkPhysicalDeviceFeatures supportedFeatures;
			/* @brief Struct holding booleans of the features that were activated on this device */
			VkPhysicalDeviceFeatures enabledFeatures;
			/* @brief Memory types and heaps of the physical device */
			VkPhysicalDeviceMemoryProperties memoryProperties;
			/*@brief Queue family properties of the physical device */
			std::vector<VkQueueFamilyProperties> queueFamilyProperties;

			/* @brief List of the device level layers that are supported */
			std::vector<VkLayerProperties> supportedLayers;
			/* @brief List of the names alias layers that were enabled on this device */
			std::vector<char const*> enabledLayers;

			/* @brief Lists of device level extensions that are supported */
			std::vector<VkExtensionProperties> supportedExtensions;
			/* @brief List of the names alias extensions that were enabled on this device */
			std::vector<char const*> enabledExtensions;

			/* @brief Default pool for commands executing on graphic queues */
			VkCommandPool cmdPoolGraphics{ VK_NULL_HANDLE };

			/* @brief Contains the queue family indices */
			struct {
				uint32_t graphics;
				uint32_t graphicsCount;
				uint32_t compute;
				uint32_t computeCount;
				uint32_t transfer;
				uint32_t transferCount;
				uint32_t present;
				uint32_t presentCount;
			} queueIndices;

			operator VkDevice() const { return logicalDevice; }

			/**
			 * Default constructor:
			 *   - picks physical device compatible to the passed in window
			 *   - stores data of the picked physical device (features, layer, extensions etc.)
			 *
			 * @param pInstance		Pointer to the (ee) instance struct this device will use
			 * @param pWindow			Pointer to the window this device will render on
			 * @param pAllocator	Pointer to different allocator callbacks (defaulting to nullptr for lazy alloc)
			 **/
			Device(
				EE::vulkan::Instance*				 pInstance,
				EE::Window*									 pWindow,
				VkAllocationCallbacks const* pAllocator);

			/* Destructor */
			~Device();

			/**
			 * Creates the logical device based on the picked physical device and queries the
			 * queue indices.
			 *
			 * @param desiredFeatures				Struct containing booleans indicating the features to enable
			 * @param additionalLayers			Layers that are desired to be activated on device level
			 * @param additionalExtensions	Extensions that are desired to be activated on device level
			 * @param requestedQueueTypes		Bitmask specifying the queue types that will be requested from the 
			 *															logical device (defaults to graphics, compute and transfer queues)
			 *
			 * @return The vulkan result of the creation call
			 **/
			VkResult Create(
				VkPhysicalDeviceFeatures const& desiredFeatures,
				std::vector<char const*> const& additionalLayers,
				std::vector<char const*> const& additionalExtensions,
				VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);

			/**
			 * Creates a buffer on this device
			 *
			 * @param usageFlags					Usage of this buffer
			 * @param memoryPropertyFlags	Memory properties
			 * @param size								Desired size of this buffer
			 * @param pBufferOut							Pointer to the buffer handle that is created
			 * @param pBufferMemoryOut		Pointer to the buffer's memory handle that is allocated
			 * @param pData								Pointer to the data that the buffer should be filled with
			 *
			 * @return A vk result, so on success VK_SUCCESS
			 **/
			VkResult CreateBuffer(
				VkBufferUsageFlags		usageFlags,
				VkMemoryPropertyFlags memoryProperties,
				VkDeviceSize					size,
				VkBuffer*							pBufferOut,
				VkDeviceMemory*				pBufferMemoryOut,
				void const*						pData = nullptr) const;

			/**
			 * Creates a device local buffer and uploads the data to it via a staging buffer.
			 *
			 * @param pData							Pointer to the data the buffer should contain
			 * @param bufferSize				Size in bytes of the data
			 * @param usageFlags				Usage that the buffer should have
			 * @param pBufferOut				Pointer to where the created buffer will be stored
			 * @param pBufferMemoryOut	Pointer to where the buffer memory will be stored
			 **/
			void CreateDeviceLocalBuffer(
				void const*				 pData,
				VkDeviceSize			 bufferSize,
				VkBufferUsageFlags usageFlags,
				VkBuffer*					 pBufferOut,
				VkDeviceMemory*		 pBufferMemoryOut) const;

			/**
			 * Creates a command pool to allocate command buffers from
			 *
			 * @param queueFamilyIndex		Family index of the queues the allocated buffers will execute on
			 * @param createFlags					Command pool creation flags (defaults)
			 *
			 * @note Command buffers allocated from the created pool can only be submitted to a queue with the
			 *			 same family index
			 *
			 * @return Vulkan handle of the created command pool
			 **/
			VkCommandPool CreateCommandPool(
				uint32_t								 queueFamilyIndex,
				VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) const;
			

			/**
			 * Returns a queue of the family passed in.
			 *
			 * @param requestedFamily		The Family that is desired (just one type please)
			 *
			 * @return The desired queue
			 **/
			VkQueue AcquireQueue(QueueTypeFlagBits requestedFamily) const;

			/** 
			 * Gets the index of the queue family that supports the passed in type
			 *
			 * @param queueFlags			The flags determining the queue to find
			 *
			 * @return	Index of the desired queue family
			 *
			 * @throw Throws an exception if no queue family index could be found according to the desired one
			 **/
			uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags) const;

			/**
			 * Gets the index of the memory type that has all the requested property flags
			 *
			 * @param typeBits			Bitmask representing the memory types that are supported by the
			 *											resource that the memory will be for
			 * @param properties		Bitmask of properties for the memory type that is requested
			 * @param memTypeFound	Pointer to a boolean that is set to true if a matching type was found (defaults)
			 *
			 * @throw		Throws runtime error if no type was found and memTypeFound was a nullptr
			 **/
			uint32_t GetMemoryType(
				uint32_t							typeBits,
				VkMemoryPropertyFlags properties,
				VkBool32*							memTypeFound = nullptr) const;

			/**
			 * Checks if the passed in format is supported with the additional configs on this device
			 *
			 * @param format				Format to check
			 * @param tiling				Tiling the image of the format will have
			 * @param featureFlags	Additional options according to the tiling that should be supported
			 *
			 * @return Is true if the format is supported in the passed in config
			 **/
			bool IsFormatSupported(
				VkFormat						 format,
				VkImageTiling				 tiling,
				VkFormatFeatureFlags featureFlags) const;

			/**
			 * Checks if a layer is supported by the current physical device
			 *
			 * @param layer				The name of the layer to check
			 *
			 * @return Is true if the layer is supported
			 **/
			bool LayerSupported(char const* layer) const;

			/**
			 * Checks if an extension is supported by the current physical device
			 *
			 * @param extension		The name of the extension to check
			 *
			 * @return Is true if the extension is supported
			 **/
			bool ExtensionSupported(char const* extension) const;

			/**
			 * Checks every available gpu if it is adequate to the window
			 * 
			 * @param instance		The instance we want to check against
			 *
			 * @return		Vulkan handle of the physical device alias gpu that was picked
			 **/
			VkPhysicalDevice PickPhysicalDevice(VkInstance instance) const;


			/*@brief Delete dangerous move/copy constructors etc. */
			Device(Device const&) = delete;
		};
	}
}