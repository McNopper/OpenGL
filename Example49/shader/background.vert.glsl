#version 460 core

uniform mat4 u_viewProjectionMatrix;

layout(location = 0) in vec4 a_vertex;

out vec3 v_ray;

void main(void)
{
    // Sphere vertex position used directly as cubemap lookup direction.
    v_ray = normalize(a_vertex.xyz);

    // xyww trick: lock background to the far plane so it never clips.
    gl_Position = (u_viewProjectionMatrix * a_vertex).xyww;
}
