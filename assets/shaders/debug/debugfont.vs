#version 330 core
in vec3 aPos;
in vec2 aTexCoord;

out vec3 vPos;
out vec2 vTexCoord;
void main()
{
    gl_Position = vec4(aPos, 1.0);
    vPos = aPos;
    vTexCoord = aTexCoord;
}
