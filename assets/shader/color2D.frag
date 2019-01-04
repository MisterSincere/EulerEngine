#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform UBO
{
	vec3 color;
} ubo;

void main()
{
	outColor = vec4(ubo.color, 1.0);
}