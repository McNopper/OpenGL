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

#ifndef GLUS_INTERSECT_H_
#define GLUS_INTERSECT_H_

/**
 * Intersecting ray against sphere.
 *
 * @param tNear			t of near intersection point if number intersections greater than zero.
 * @param tFar			t of far intersection point if number intersections greater than zero.
 * @param insideSphere	Set to GLUS_TRUE, if ray starts inside sphere.
 * @param rayStart		Point, where the ray starts.
 * @param rayDirection	Ray direction vector. Has to be normalized.
 * @param sphereCenter	Center of the sphere given as a point.
 * @param radius		Radius of the sphere.
 *
 * @return Number of intersection points.
 */
GLUSAPI GLUSint GLUSAPIENTRY glusIntersectRaySpheref(GLUSfloat* tNear, GLUSfloat* tFar, GLUSboolean* insideSphere, const GLUSfloat rayStart[4], const GLUSfloat rayDirection[3], const GLUSfloat sphereCenter[4], const GLUSfloat radius);

#endif /* GLUS_INTERSECT_H_ */
