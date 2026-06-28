#version 460 core

in vec3  v_color;
in float v_opacity;
in vec2  v_u;

out vec4 fragColor;

void main()
{
    float weight = exp(-0.5 * dot(v_u, v_u)) * v_opacity;

    fragColor = vec4(v_color * weight, weight);
}
