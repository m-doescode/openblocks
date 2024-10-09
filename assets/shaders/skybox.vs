#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 vPos;
out vec3 vNormal;
out vec2 vTexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vec3(view * vec4(aPos, 0.0)), 1.0);
    vPos = vec3(vec4(aPos, 1.0));
    vNormal = aNormal;
    vTexCoords = aTexCoords;
}
