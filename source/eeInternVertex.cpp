/////////////////////////////////////////////////////////////////////
// Filename: eeInternVertex.cpp
//
// (C) Copyright 2018 Madness Studio. All Rights Reserved
/////////////////////////////////////////////////////////////////////
#include "eeIntern.h"


VkVertexInputBindingDescription EEInternVertex::getBindingDescription()
{
  VkVertexInputBindingDescription vertexInputBindingDescription;
  vertexInputBindingDescription.binding = 0;
  vertexInputBindingDescription.stride = sizeof(EEInternVertex);
  vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return vertexInputBindingDescription;
}

std::vector<VkVertexInputAttributeDescription> EEInternVertex::getAttrDesc()
{
  std::vector<VkVertexInputAttributeDescription> vertexInputAttrDesc(4);
  vertexInputAttrDesc[0].location = 0;
  vertexInputAttrDesc[0].binding = 0;
  vertexInputAttrDesc[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  vertexInputAttrDesc[0].offset = offsetof(EEInternVertex, pos);

  vertexInputAttrDesc[1].location = 1;
  vertexInputAttrDesc[1].binding = 0;
  vertexInputAttrDesc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  vertexInputAttrDesc[1].offset = offsetof(EEInternVertex, color);

  vertexInputAttrDesc[2].location = 2;
  vertexInputAttrDesc[2].binding = 0;
  vertexInputAttrDesc[2].format = VK_FORMAT_R32G32_SFLOAT;
  vertexInputAttrDesc[2].offset = offsetof(EEInternVertex, uvCoord);

  vertexInputAttrDesc[3].location = 3;
  vertexInputAttrDesc[3].binding = 0;
  vertexInputAttrDesc[3].format = VK_FORMAT_R32G32B32_SFLOAT;
  vertexInputAttrDesc[3].offset = offsetof(EEInternVertex, normal);

  return vertexInputAttrDesc;
}

bool EEInternVertex::operator==(const EEInternVertex& other) const
{
  return pos == other.pos && color == other.color && uvCoord == other.uvCoord && normal == other.normal;
}