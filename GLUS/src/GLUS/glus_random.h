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

#ifndef GLUS_RANDOM_H_
#define GLUS_RANDOM_H_

/**
 * Initializes the random generator.
 *
 * @param seed Number for initializing the pseudo-random number generator.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusRandomSetSeed(const GLUSuint seed);

/**
 * Returns a uniform distributed random floating point value in the given range.
 *
 * @param start Smallest possible generated value (inclusive).
 * @param end Largest possible generated value (inclusive).
 *
 * @return The random value.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusRandomUniformGetFloatf(const GLUSfloat start, const GLUSfloat end);

/**
 * Returns a normal distributed random floating point value.
 *
 * @param mean 				Mean.
 * @param standardDeviation Standard deviation.
 *
 * @return The random value.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusRandomNormalGetFloatf(const GLUSfloat mean, const GLUSfloat standardDeviation);

/**
 * Samples two floating point values from a Hammersley point set.
 *
 * @param result 	The resulting random values.
 * @param sample	The sample to take. Has to be in the range 0 <= sample < 2^m.
 * @param m			Order m, which allows 2^m samples. Has to be in the range 0 < m <= 32.
 *
 * @return GLUS_TRUE, if sampling was successful.
 **/
GLUSAPI GLUSboolean GLUSAPIENTRY glusRandomHammersleyf(GLUSfloat result[2], const GLUSuint sample, const GLUSubyte m);

#endif /* GLUS_RANDOM_H_ */
