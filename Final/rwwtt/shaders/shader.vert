#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

void main() {
    gl_Position = vec4(inPosition, 0.f, 1.f);
}
