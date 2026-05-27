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

#version 410 core

// Generates a fullscreen quad from gl_VertexID — no VBO required.
// gl_VertexID: 0=(-1,-1), 1=(1,-1), 2=(-1,1), 3=(1,1)
// v_texCoord is in [-1, 1] so fragment shaders can use it directly as
// the cube-face UV (face direction space).

out vec2 v_texCoord;

void main(void)
{
    vec2 pos    = vec2((gl_VertexID & 1) * 2 - 1, (gl_VertexID >> 1) * 2 - 1);
    v_texCoord  = pos;
    gl_Position = vec4(pos, 0.0, 1.0);
}
