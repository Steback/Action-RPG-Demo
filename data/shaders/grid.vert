#version 450

layout(push_constant) uniform MVP {
    mat4 model;
    mat4 view;
    mat4 proj;
} view;

layout(location = 1) out vec3 nearPoint;
layout(location = 2) out vec3 farPoint;
layout(location = 3) out mat4 fragView;
layout(location = 7) out mat4 fragProj;

// Grid position are in xy clipped space
vec3 gridPlane[6] = {
    vec3(1, 1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
    vec3(-1, -1, 0), vec3(1, 1, 0), vec3(1, -1, 0)
};

vec3 unprojectPoint(float x, float y, float z, mat4 view, mat4 proj) {
    mat4 viewInv = inverse(view);
    mat4 projInv = inverse(proj);
    vec4 unprojectedPoint = viewInv * projInv * vec4(x, y, z, 1.0);

    return unprojectedPoint.xyz / unprojectedPoint.w;
}

void main() {
    vec3 p = gridPlane[gl_VertexIndex].xyz;
    nearPoint = unprojectPoint(p.x, p.y, 0.0, view.view, view.proj);
    farPoint = unprojectPoint(p.x, p.y, 1.0, view.view, view.proj);

    fragProj = view.proj;
    fragView = view.view;

    gl_Position = vec4(p, 1.0);
}
