#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec4 col;

out vec3 v_normal;
out vec4 v_color;

uniform mat4 model;
uniform mat4 transform;

void main()
{
    v_color = col.abgr;
    v_normal = (model * vec4(norm, 1.0)).xyz;
    gl_Position = transform * model * vec4(pos, 1.0);
}


