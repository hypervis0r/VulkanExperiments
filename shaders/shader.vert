#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(location = 0) in vec3 vao_inPosition;
layout(location = 1) in vec3 vao_inColor;
layout(location = 2) in vec2 vao_inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(vao_inPosition, 1.0);
    fragColor = vao_inColor;
    fragTexCoord = vao_inTexCoord;
}