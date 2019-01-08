#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 fTexCoord;

layout(binding = 1) uniform sampler2D tex;
layout(binding = 2) uniform UBO {
	vec3 textColor;
} ubo;

layout(location = 0) out vec4 outColor;

void main() {
	vec4 texColor = texture(tex, fTexCoord);

	outColor = vec4(ubo.textColor, texColor.r);
}