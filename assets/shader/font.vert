#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 vPosition;
layout(location = 1) in vec2 vTexCoord;

layout(binding = 0) uniform UBO {
	mat4 ortho;
	mat4 baseView;
	mat4 world;
} ubo;

layout(location = 0) out vec2 fTexCoord;

void main() {
	gl_Position = ubo.ortho * ubo.baseView * ubo.world * vec4(vPosition, 0.0, 1.0);
	fTexCoord = vTexCoord;
}