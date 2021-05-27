#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord0;
layout(location = 3) in vec2 texCoord1;
layout(location = 4) in vec4 jointIndices;
layout(location = 5) in vec4 jointWeights;

layout(location = 0) out vec2 fragTexCoord0;
layout(location = 1) out vec2 fragTexCoord1;

layout(push_constant) uniform MVP {
    mat4 proj;
    mat4 view;
    mat4 model;
} mvp;

#define MAX_NUM_JOINTS 128

layout(std430, set = 2, binding = 0) readonly buffer JointMatrices {
    mat4 jointMatrices[];
};

void main() {
    mat4 skinMat = jointWeights.x * jointMatrices[int(jointIndices.x)] +
                   jointWeights.y * jointMatrices[int(jointIndices.y)] +
                   jointWeights.z * jointMatrices[int(jointIndices.z)] +
                   jointWeights.w * jointMatrices[int(jointIndices.w)];

    gl_Position = mvp.proj * mvp.view * mvp.model * skinMat * vec4(position, 1.0);
    fragTexCoord0 = texCoord0;
    fragTexCoord1 = texCoord1;
}
