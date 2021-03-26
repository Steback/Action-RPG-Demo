#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;
layout(location = 1) in vec3 nearPoint;
layout(location = 2) in vec3 farPoint;
layout(location = 3) in mat4 fragView;
layout(location = 7) in mat4 fragProj;

vec4 grid(vec3 fragPos3D, float scale) {
    vec2 coord = fragPos3D.xz * scale;
    vec2 derivative = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / derivative;
    float line = min(grid.x, grid.y);
    float minimumz = min(derivative.y, 1);
    float minimumx = min(derivative.x, 1);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));

    if (fragPos3D.x > -0.1 * minimumx && fragPos3D.x < 0.1 * minimumx) color.z = 1.0;

    if (fragPos3D.z > -0.1 * minimumz && fragPos3D.z < 0.1 * minimumz) color.x = 1.0;

    return color;
}

float computeDepth(vec3 pos) {
    vec4 clipSpacePos = fragProj * fragView * vec4(pos, 1.0);

    return (clipSpacePos.z / clipSpacePos.w);
}

void main() {
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    vec3 fragPos3D = nearPoint + t * (farPoint - nearPoint);
    gl_FragDepth = computeDepth(fragPos3D);
    outColor = grid(fragPos3D, 10) * float(t > 0);
}
