/////////////////////////////////////////////////////////////////////
// Filename: eeIntern.h
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#pragma once

#include "vulkanRenderer.h"


#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <vector>


//---------------------------------------------------------------------------------------
// EEInternVertex
//---------------------------------------------------------------------------------------
class EEInternVertex {
public:
  glm::vec3 pos;
  glm::vec3 color;
  glm::vec2 uvCoord;
  glm::vec3 normal;

  EEInternVertex(glm::vec3 pos, glm::vec3 color, glm::vec2 uvCoord, glm::vec3 normal)
    : pos(pos), color(color), uvCoord(uvCoord), normal(normal) {}

  bool operator==(const EEInternVertex&) const;

  static VkVertexInputBindingDescription getBindingDescription();

  static std::vector<VkVertexInputAttributeDescription> getAttrDesc();
};
namespace std
{
  template<> struct hash<EEInternVertex>
  {
    size_t operator()(EEInternVertex const &vert) const
    {
      size_t h1 = hash<glm::vec3>()(vert.pos);
      size_t h2 = hash<glm::vec3>()(vert.color);
      size_t h3 = hash<glm::vec2>()(vert.uvCoord);
      size_t h4 = hash<glm::vec3>()(vert.normal);

      return ((((h1 ^ (h2 << 1)) >> 1) ^ h3) << 1) ^ h4;
    }
  };
}


namespace vk
{
  namespace intern
  {

    //---------------------------------------------------------------------------------------
    // DepthImage
    //---------------------------------------------------------------------------------------
    struct DepthImage
    {
      /* @brief Encapsulates the swapchain this depth image is for */
      vk::VulkanSwapchain* swapchain;

      /* @brief The image buffer */
      VkImage image;
      /* @brief Memory backing up the image buffer */
      VkDeviceMemory imageMemory;
      /* @brief View to the image buffer */
      VkImageView imageView;

      /* @brief The depth format that is used */
      VkFormat depthFormat;

      /* @brief Is true if creation was successfull and release has not yet been called */
      bool isCreated{ false };

      /**
       * Default constructor, checks for settings
       *
       * @param swapchain   The swapchain the depth image is created for
       **/
      DepthImage(vk::VulkanSwapchain* swapchain);

      /**
       * Default constructor just calls the release method.
       **/
      ~DepthImage() { Release(); }

      /**
       *
       **/
      void Create();

      /**
       *
       **/
      void Release();

      /**
       * Returns the attachment description for a depth image with the format
       * depending on the swapchain this instance was created with.
       **/
      VkAttachmentDescription DepthAttachmentDescription();

      /**
       *
       **/
      static VkPipelineDepthStencilStateCreateInfo DepthStencilStateCInfoOpaque();
    };

    

    //---------------------------------------------------------------------------------------
    // Mesh
    //---------------------------------------------------------------------------------------
    struct Mesh
    {

    };

    //---------------------------------------------------------------------------------------
    // Texture
    //---------------------------------------------------------------------------------------
    struct Texture
    {
      /* @brief The renderer this texture is created/destroyed with */
      vk::VulkanRenderer* renderer;

      /* @brief Vulkan handle for the image */
      VkImage image;
      /* @brief Memory backing up the above image */
      VkDeviceMemory imageMemory;
      /* @brief View to the created image */
      VkImageView imageView;
      /* @brief Handle of the sampler for the shader */
      VkSampler sampler;

      /* @brief Encapsulates primitive data of the image */
      struct {
        unsigned char* pixels;
        int width;
        int height;
        int channels;
      } data;

      /* @brief Indicates if the texture was already uploaded */
      bool isUploaded{ false };

      /**
       * Default constructor loads the images data
       *
       * @param renderer  Pointer to the renderer this texture is created with
       * @param filename  Destination of the file to load
       **/
      Texture(vk::VulkanRenderer* renderer, const char* filename);

      /**
       * Default destructor
       **/
      ~Texture();

      /**
       * Creates the vulkan handles (sampler
       **/
      void Upload();
    };

    //---------------------------------------------------------------------------------------
    // Pipeline
    //---------------------------------------------------------------------------------------
    struct Pipeline
    {
      /* @brief Renderer this pipeline is created for */
      vk::VulkanRenderer* renderer;

      /* @brief Vertex stage definition */
      VkPipelineShaderStageCreateInfo vertexShaderStageCInfo;
      /* @brief Fragment stage definition */
      VkPipelineShaderStageCreateInfo fragmentShaderStageCInfo;
      /* @brief The vertex input description uses predefined descriptions below or custom ones set by ini-method */
      VkPipelineVertexInputStateCreateInfo vertexInputCInfo;
      /* @brief Input assembly state obtained from the renderer (using Trianglelist) */
      VkPipelineInputAssemblyStateCreateInfo inputAssemblyCInfo;
      /* @brief Viewport definition obtained from the renderer */
      VkPipelineViewportStateCreateInfo viewportCInfo;
      /* @brief Rasterizer info (clockwise or counterclockwise setting) */
      VkPipelineRasterizationStateCreateInfo rasterizerCInfo;
      /* @brief Multisample information obtained from the renderer */
      VkPipelineMultisampleStateCreateInfo multisampleCInfo;
      /* @brief Color blend state obtained from the renderer (default alpha blending) */
      VkPipelineColorBlendStateCreateInfo blendStateCInfo;
      /* @brief Dynamic states of the pipeline (viewport and scissor) */
      VkPipelineDynamicStateCreateInfo dynamicStateCInfo;
      /* @brief Vector holding the VkPushConstantRanges if set by ini-method */
      std::vector<VkPushConstantRange> pushConstants;

      /* @brief Depth stencil description defined by the used depth image */
      VkPipelineDepthStencilStateCreateInfo depthStencilCInfo = DepthImage::DepthStencilStateCInfoOpaque();
      /* @brief Information about the vertex bindings (default are bindings of the intern vertex; can be customized by ini-method) */
      VkVertexInputBindingDescription predefinedVertexBindingDesc = EEInternVertex::getBindingDescription();
      /* @brief Information about the vertex attributes (default are attributes of the intern vertex; can be customized by ini-method) */
      std::vector<VkVertexInputAttributeDescription> predefinedVertexAttrDescs = EEInternVertex::getAttrDesc();

      /* @brief Is true if the above create infos are initialized */
      bool isInitialized{ false };
      /* @brief Is true if the pipeline / pipelinelayout is created */
      bool isCreated{ false };

      /* @brief Vulkan handle that holds the pipeline layout */
      VkPipelineLayout pipelineLayout;
      /* @brief Vulkan handle to the pipeline */
      VkPipeline pipeline;

      /**
       * Default constructor
       *
       * @param renderer    Pointer to the renderer the pipeline will use
       * @param vertShader  Vertex shader module
       * @param fragShader  Fragment shader module
       **/
      Pipeline(vk::VulkanRenderer* renderer, VkShaderModule vertShader, VkShaderModule fragShader);

      /**
       * Default desctructor
       **/
      ~Pipeline();


      /**
       * Changes important informations of the pipeline according to the shader you wanna use
       *
       * @param shaderCInfo   Struct containing the shader infos this pipeline will be used for
       **/
      void Ini(const EEShaderCreateInfo& shaderCInfo);

      /**
       * Creates the layout and the graphics pipeline according to initialized infos and the descriptor set layout passed in
       *
       * @param descriptorSetLayout   Layout of the descriptor sets (uniform buffers, samplers etc.)
       **/
      void Create(VkDescriptorSetLayout descriptorSetLayout);
    };

    //---------------------------------------------------------------------------------------
    // Shader
    //---------------------------------------------------------------------------------------
    struct Shader
    {
      /* @brief Holds information about a uniform buffer */
      struct UniformBufferDetails {
        VkBuffer buffer;
        VkDeviceMemory memory;
        uint32_t binding;
        uint32_t size;
      };

      /* @brief Holds the uniform buffers and the set they are contained */
      struct DescriptorSetDetails {
        VkDescriptorSet descriptorSet;
        std::vector<UniformBufferDetails*> uniformBuffers;
      };

      /* @brief The renderer this shader will use */
      vk::VulkanRenderer* renderer;
      /* @brief Struct encapsulating information about this shader */
      EEShaderCreateInfo shaderInfo;

      /* @brief The pipeline this shader creates according to the shader properties */
      vk::intern::Pipeline* pipeline;

      /* @brief Shader modules for vertex and fragment shader (geometry is optional) */
      VkShaderModule vertexShaderModule;
      VkShaderModule fragmentShaderModule;
      VkShaderModule geometryShaderModule{ VK_NULL_HANDLE };
      /* @brief Descriptor set layout of this shader */
      VkDescriptorSetLayout descriptorSetLayout;
      /* @brief Vulkan handle of the descriptor pool that provides memory for all descriptor sets */
      VkDescriptorPool descriptorPool;
      /* @brief The info to create a descriptor set for this shader */
      VkDescriptorSetAllocateInfo descriptorSetAllocInfo;
      /* @brief Write infos for the the descriptor sets */
      VkWriteDescriptorSet* writeDescriptorSets = new VkWriteDescriptorSet[2];

      /* @brief Encapsulates the push constant data and its size */
      struct {
        void* data;
        uint32_t size;
      } pushConstant;

      /* @brief Indicates if the handles of this shader were already created */
      bool isCreated{ false };

      /* @brief Amount of already created descriptor sets to check if max defined in shader info is reached */
      uint32_t currentAmountDescriptorSets{ 0u };

      /**
       * Standard constructor
       *
       * @param renderer    The renderer this shader will use
       * @param shaderCInfo Informations about the shader to create
       **/
      Shader(vk::VulkanRenderer* renderer, const EEShaderCreateInfo& shaderCInfo);

      /**
       * Standard desctructor
       **/
      ~Shader();

      /**
       * Will create the descriptor sets and the pipeline
       **/
      void Create();

      /**
       * Creates a descriptor set for an object to use.
       *
       * @param descriptorSetOut  The DescriptorSetDetails that will be filled out by the method
       * @param textures          List of all created textures that can be set for a uniform sampler
       **/
      void CreateDescriptorSet(DescriptorSetDetails& descriptorSetOut, const std::vector<vk::intern::Texture>& textures);
    };


    //---------------------------------------------------------------------------------------
    // Object
    //---------------------------------------------------------------------------------------
    struct Object
    {
      vk::intern::Mesh* mesh;
      vk::intern::Shader* shader;

      /**
       * Default constructor
       **/
      Object(vk::intern::Mesh* mesh, vk::intern::Shader* shader);

      /**
       * Default desctructor
       **/
      ~Object();
    };
  }
}