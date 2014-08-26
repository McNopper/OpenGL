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

#ifndef GLUS_MATH_H_
#define GLUS_MATH_H_

/**
 * Calculates the maximum of two values.
 *
 * @param value0 The first value.
 * @param value1 The second value.
 *
 * @return The maximum of the two values.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMathMaxf(const GLUSfloat value0, const GLUSfloat value1);

/**
 * Calculates the minimum of two values.
 *
 * @param value0 The first value.
 * @param value1 The second value.
 *
 * @return The minimum of the two values.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMathMinf(const GLUSfloat value0, const GLUSfloat value1);

/**
 * Converts radians to degrees.
 *
 * @param radians The angle in radians.
 *
 * @return The angle in degrees.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMathRadToDegf(const GLUSfloat radians);

/**
 * Converts degrees to radians.
 *
 * @param degrees The angle in degrees.
 *
 * @return The angle in radians.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMathDegToRadf(const GLUSfloat degrees);

/**
 * Linear interpolation of two values. If t = 0, value0 is returned. If t = 1, value1 is returned.
 *
 * @param value0 First value.
 * @param value1 Second value.
 * @param t The fraction.
 *
 * @return The interpolated value.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMathMixf(const GLUSfloat value0, const GLUSfloat value1, const GLUSfloat t);

/**
 * Clamps a given value between a minimum and maximum value. If the value is between min and max, the value is returned.
 *
 * @param value The value to be clamped.
 * @param min The minimum border. If the value is lower than min, min is returned.
 * @param max The maximum border. If the value is greater than max, max is returned.
 *
 * @return The clamped value.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMathClampf(const GLUSfloat value, const GLUSfloat min, const GLUSfloat max);

/**
 * Calculates the length of a vector or the distance of a point to the origin by providing the x, y and z coordinates.
 *
 * @param x The x coordinate.
 * @param y The y coordinate.
 * @param z The z coordinate.
 *
 * @return The calculated length.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMathLengthf(const GLUSfloat x, const GLUSfloat y, const GLUSfloat z);

#endif /* GLUS_MATH_H_ */
