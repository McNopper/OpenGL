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

// Converts a latitude-longitude (equirectangular) panorama to one face of a
// cubemap by mapping each texel through the canonical OpenGL cubemap direction
// convention (Table 8.19).
//
// see http://gl.ict.usc.edu/Data/HighResProbes/

#define GLUS_PI 3.1415926535897932384626433832795

uniform sampler2D u_panoramaTexture;
uniform int       u_face;

in vec2  v_texCoord;
out vec4 fragColor;

// Equirectangular UV from a normalised direction.
vec2 panorama(vec3 ray)
{
    return vec2(0.5 + 0.5 * atan(ray.x, -ray.z) / GLUS_PI,
                1.0 - acos(clamp(ray.y, -1.0, 1.0)) / GLUS_PI);
}

// Convert a cube-face UV in [-1, 1]^2 to the corresponding 3-D direction.
// Face order follows OpenGL Table 8.19.
vec3 cubemapDirection(int face, vec2 uv)
{
    switch (face)
    {
    case 0: return normalize(vec3(1.0, -uv.y, -uv.x));  // +X
    case 1: return normalize(vec3(-1.0, -uv.y, uv.x));  // -X
    case 2: return normalize(vec3(uv.x, 1.0, uv.y));    // +Y
    case 3: return normalize(vec3(uv.x, -1.0, -uv.y));  // -Y
    case 4: return normalize(vec3(uv.x, -uv.y, 1.0));   // +Z
    case 5: return normalize(vec3(-uv.x, -uv.y, -1.0)); // -Z
    }
    return vec3(0.0);
}

void main(void)
{
    vec3 dir = cubemapDirection(u_face, v_texCoord);

    fragColor = vec4(texture(u_panoramaTexture, panorama(dir)).rgb, 1.0);
}
