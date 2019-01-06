#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 position;

layout(binding = 0) uniform UBO
{
	mat4 ortho;
	mat4 baseView;
	mat4 world;
} ubo;

void main()
{
	gl_Position = ubo.ortho * ubo.baseView * ubo.world * vec4(position, 1.0);
}