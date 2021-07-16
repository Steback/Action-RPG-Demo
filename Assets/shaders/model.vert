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

layout (set = 2, binding = 0) uniform UBONode {
    mat4 matrix;
    mat4 jointMatrix[MAX_NUM_JOINTS];
    float jointCount;
} node;

void main() {
    vec4 locPos;

    if (node.jointCount > 0.0) {
        // Mesh is skinned
        mat4 skinMat =  jointWeights.x * node.jointMatrix[int(jointIndices.x)] +
                        jointWeights.y * node.jointMatrix[int(jointIndices.y)] +
                        jointWeights.z * node.jointMatrix[int(jointIndices.z)] +
                        jointWeights.w * node.jointMatrix[int(jointIndices.w)];

        locPos = mvp.model * node.matrix * skinMat * vec4(position, 1.0);
    } else {
        locPos = mvp.model * node.matrix * vec4(position, 1.0);
    }

    gl_Position = mvp.proj * mvp.view * locPos;
    fragTexCoord0 = texCoord0;
    fragTexCoord1 = texCoord1;
}
