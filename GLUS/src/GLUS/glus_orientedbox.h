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

#ifndef GLUS_ORIENTED_BOX_H_
#define GLUS_ORIENTED_BOX_H_

/**
 * Copies an oriented box.
 *
 * @param resultCenter	    Destination center.
 * @param resultHalfExtend  Destination half extend.
 * @param resultOrientation Destination orientation.
 * @param center	   	    Source center.
 * @param halfExtend	    Source half extend.
 * @param orientation	    Source orientation.
 */
GLUSvoid GLUSAPIENTRY glusOrientedBoxCopyf(GLUSfloat resultCenter[4], GLUSfloat resultHalfExtend[3], GLUSfloat resultOrientation[3], const GLUSfloat center[4], const GLUSfloat halfExtend[3], const GLUSfloat orientation[3]);

/**
 * Calculates the signed distance from an oriented box to a point. If the value is negative, the point is inside the box.
 *
 * @param center	  The center of the box.
 * @param halfExtend  The length from the center point to the planes of the box.
 * @param orientation The orientation of the box.
 * @param point		  The used point.
 *
 * @return The signed distance.
 */
GLUSfloat GLUSAPIENTRY glusOrientedBoxDistancePoint4f(const GLUSfloat center[4], const GLUSfloat halfExtend[3], const GLUSfloat orientation[3], const GLUSfloat point[4]);

#endif /* GLUS_ORIENTED_BOX_H_ */
