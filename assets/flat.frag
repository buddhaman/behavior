#version 330 core

in vec3 v_normal;
in vec4 v_color;

out vec4 color;

void main()
{
    color = v_color;
}

