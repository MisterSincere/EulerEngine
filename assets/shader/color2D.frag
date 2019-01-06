#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform UBO
{
	vec4 bgColor;
} ubo;

void main()
{
	outColor = ubo.bgColor;
}