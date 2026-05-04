/*
 * Example 50 — copy of GLUS glus_gltf_pbr.vert.glsl (unchanged).
 * Kept locally so Example50 ships its own shaders alongside the modified
 * fragment shader that performs Rec.709 -> Rec.2020 working-space conversion.
 */

#version 460 core

uniform mat4 u_modelMatrix;
uniform mat4 u_viewProjectionMatrix;
uniform mat3 u_normalMatrix;

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec4 a_tangent;
layout(location = 3) in vec2 a_texCoord0;

out vec3 v_worldPos;
out vec3 v_normal;
out vec4 v_tangent;
out vec2 v_texCoord0;

void main(void)
{
	vec4 worldPos = u_modelMatrix * vec4(a_position, 1.0);
	v_worldPos    = worldPos.xyz;
	v_normal      = normalize(u_normalMatrix * a_normal);
	v_tangent     = vec4(normalize(u_normalMatrix * a_tangent.xyz), a_tangent.w);
	v_texCoord0   = a_texCoord0;
	gl_Position   = u_viewProjectionMatrix * worldPos;
}
