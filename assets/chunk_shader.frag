#version 330 core

in vec3 v_normal;
in vec4 v_color;

out vec4 color;

void main()
{
    vec3 light_dir = normalize(vec3(1.0, 2.0, -3.0));
    float ambient = 0.4;

    float dp = clamp(dot(v_normal, -light_dir), 0.0, 1.0);
    float light_factor = ambient + (1.0-ambient)*dp;
    vec3 light_color = v_color.rgb*light_factor;
    color = vec4(light_color, 1.0);
}

