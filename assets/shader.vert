#version 330 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 col;

out vec4 v_color;

uniform vec2 offset;
uniform vec2 scale;

void main()
{
    v_color = col.abgr;
    gl_Position = vec4((pos+offset)*scale, 0.0, 1.0);
}


