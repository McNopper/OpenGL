#version 460 core

in vec4  v_color;
out vec4 fragColor;

void main()
{
    // Discard fragments outside the inscribed circle to get round points.
    vec2 coord = gl_PointCoord * 2.0 - 1.0;
    if (dot(coord, coord) > 1.0)
        discard;

    fragColor = v_color;
}
