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

#ifndef GLUS_VIEWPROJECTION_H_
#define GLUS_VIEWPROJECTION_H_

/**
 * Creates a matrix with orthogonal projection.
 * @see http://en.wikipedia.org/wiki/Orthographic_projection_%28geometry%29
 *
 * @param result The resulting matrix.
 * @param left Left corner.
 * @param right Right corner.
 * @param bottom Bottom corner.
 * @param top Top corner.
 * @param nearVal Near corner.
 * @param farVal Far corner.
 *
 * @return GLUS_TRUE, if creation was successful.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusOrthof(GLUSfloat result[16], const GLUSfloat left, const GLUSfloat right, const GLUSfloat bottom, const GLUSfloat top, const GLUSfloat nearVal, const GLUSfloat farVal);

/**
 * Creates a matrix with perspective projection.
 *
 * @param result The resulting matrix.
 * @param left Left corner.
 * @param right Right corner.
 * @param bottom Bottom corner.
 * @param top Top corner.
 * @param nearVal Near corner.
 * @param farVal Far corner.
 *
 * @return GLUS_TRUE, if creation was successful.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusFrustumf(GLUSfloat result[16], const GLUSfloat left, const GLUSfloat right, const GLUSfloat bottom, const GLUSfloat top, const GLUSfloat nearVal, const GLUSfloat farVal);

/**
 * Creates a matrix with perspective projection.
 *
 * @param result The resulting matrix.
 * @param fovy Field of view.
 * @param aspect Aspect ratio.
 * @param zNear Near plane.
 * @param zFar Far plane.
 *
 * @return GLUS_TRUE, if creation was successful.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusPerspectivef(GLUSfloat result[16], const GLUSfloat fovy, const GLUSfloat aspect, const GLUSfloat zNear, const GLUSfloat zFar);

/**
 * Creates a view / camera matrix.
 *
 * @param result The resulting matrix.
 * @param eyeX Eye / camera X position.
 * @param eyeY Eye / camera Y position.
 * @param eyeZ Eye / camera Z position.
 * @param centerX X Position, where the view / camera points to.
 * @param centerY Y Position, where the view / camera points to.
 * @param centerZ Z Position, where the view / camera points to.
 * @param upX Eye / camera X component from up vector.
 * @param upY Eye / camera Y component from up vector.
 * @param upZ Eye / camera Z component from up vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusLookAtf(GLUSfloat result[16], const GLUSfloat eyeX, const GLUSfloat eyeY, const GLUSfloat eyeZ, const GLUSfloat centerX, const GLUSfloat centerY, const GLUSfloat centerZ, const GLUSfloat upX, const GLUSfloat upY, const GLUSfloat upZ);

#endif /* GLUS_VIEWPROJECTION_H_ */
