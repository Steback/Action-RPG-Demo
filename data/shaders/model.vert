#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(push_constant) uniform MVP {
    mat4 matrix;
} mvp;

void main() {
    gl_Position = mvp.matrix * vec4(position, 1.0);
    fragColor = color;
    fragTexCoord = texCoord;
}
