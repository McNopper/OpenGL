/*
 * GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities. Copyright (C) since 2010 Norbert Nopper
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#version 460 core

#define MAX_JOINTS 128

uniform mat4 u_viewProjectionMatrix;
uniform mat4 u_jointMatrices[MAX_JOINTS];

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec4 a_tangent; // xyz=tangent, w=handedness
layout(location = 3) in vec2 a_texCoord0;
layout(location = 4) in vec4 a_joints0;  // joint indices as floats
layout(location = 5) in vec4 a_weights0; // blend weights

out vec3 v_worldPos;
out vec3 v_normal;
out vec4 v_tangent;
out vec2 v_texCoord0;

mat4 getSkinMatrix()
{
    return a_weights0.x * u_jointMatrices[int(a_joints0.x)] + a_weights0.y * u_jointMatrices[int(a_joints0.y)] + a_weights0.z * u_jointMatrices[int(a_joints0.z)] + a_weights0.w * u_jointMatrices[int(a_joints0.w)];
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
