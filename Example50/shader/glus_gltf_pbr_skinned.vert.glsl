/*
 * Example 50 — copy of GLUS glus_gltf_pbr_skinned.vert.glsl (unchanged).
 */

#version 460 core

#define MAX_JOINTS 128

uniform mat4 u_viewProjectionMatrix;
uniform mat4 u_jointMatrices[MAX_JOINTS];

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec4 a_tangent;
layout(location = 3) in vec2 a_texCoord0;
layout(location = 4) in vec4 a_joints0;
layout(location = 5) in vec4 a_weights0;

out vec3 v_worldPos;
out vec3 v_normal;
out vec4 v_tangent;
out vec2 v_texCoord0;

mat4 getSkinMatrix()
{
	return a_weights0.x * u_jointMatrices[int(a_joints0.x)]
	     + a_weights0.y * u_jointMatrices[int(a_joints0.y)]
	     + a_weights0.z * u_jointMatrices[int(a_joints0.z)]
	     + a_weights0.w * u_jointMatrices[int(a_joints0.w)];
}

void main(void)
{
	mat4 skinMatrix   = getSkinMatrix();
	mat3 normalMatrix = mat3(transpose(inverse(skinMatrix)));

	vec4 worldPos = skinMatrix * vec4(a_position, 1.0);
	v_worldPos    = worldPos.xyz;
	v_normal      = normalize(normalMatrix * a_normal);
	v_tangent     = vec4(normalize(normalMatrix * a_tangent.xyz), a_tangent.w);
	v_texCoord0   = a_texCoord0;
	gl_Position   = u_viewProjectionMatrix * worldPos;
}
