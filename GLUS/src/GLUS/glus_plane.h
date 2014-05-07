/*
 * GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities. Copyright (C) 2010 - 2014 Norbert Nopper
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

#ifndef GLUS_PLANE_H_
#define GLUS_PLANE_H_

/**
 * Copies a plane.
 *
 * @param result Destination plane.
 * @param plane Source plane.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPlaneCopyf(GLUSfloat result[4], const GLUSfloat plane[4]);

/**
 * Creates a plane. The formula is Ax + By + Cz + D = 0, where A,B,C and D are stored in the resulting array.
 * @see Mathematics For 3D Game Programming & Computer Graphics, Second Edition, Page105
 *
 * @param result The calculated plane.
 * @param point Any point on the plane.
 * @param normal The normal vector of the plane. The vector will be normalized during calculation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPlaneCreatef(GLUSfloat result[4], const GLUSfloat point[4], const GLUSfloat normal[3]);

/**
 * Calculates the signed distance from a plane to a point. If the value is positive, the point is on the side the normal is directing to.
 *
 * @param plane The used plane.
 * @param point The used point.
 *
 * @return The signed distance.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusPlaneDistancePoint4f(const GLUSfloat plane[4], const GLUSfloat point[4]);

/**
 * Calculates a point on the given plane, located around the origin.
 *
 * @param point The resulting point.
 * @param plane The used plane.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPlaneGetPoint4f(GLUSfloat point[4], const GLUSfloat plane[4]);

#endif /* GLUS_PLANE_H_ */
