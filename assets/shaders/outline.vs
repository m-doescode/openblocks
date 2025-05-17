#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 vPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 scale;
uniform float thickness;

void main()
{
    vec3 distFromEdge = sign(aPos) * 0.5 - aPos;
    vec3 tVec = (scale * sign(aPos) * 0.5 - distFromEdge * thickness) / scale;

    gl_Position = projection * view * model * vec4(tVec, 1);
    vPos = aPos;
}
