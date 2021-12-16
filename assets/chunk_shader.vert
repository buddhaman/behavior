#version 330 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec4 a_color;

out vec3 v_normal;
out vec4 v_color;

uniform mat4 model;
uniform mat4 transform;

void main()
{
    v_color = a_color.abgr;
    v_normal = a_normal;
    vec4 pos = transform * model * vec4(256.0*a_pos, 1.0);
    gl_Position = pos;
}

