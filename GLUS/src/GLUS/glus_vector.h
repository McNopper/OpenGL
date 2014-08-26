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

#ifndef GLUS_VECTOR_H_
#define GLUS_VECTOR_H_

/**
 * Copies a 3D Vector.
 *
 * @param result The destination vector.
 * @param vector The source vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3Copyf(GLUSfloat result[3], const GLUSfloat vector[3]);

/**
 * Copies a 2D Vector.
 *
 * @param result The destination vector.
 * @param vector The source vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector2Copyf(GLUSfloat result[2], const GLUSfloat vector[2]);

/**
 * Adds a 3D Vector to another.
 *
 * @param result The final vector.
 * @param vector0 The first vector.
 * @param vector1 The second vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3AddVector3f(GLUSfloat result[3], const GLUSfloat vector0[3], const GLUSfloat vector1[3]);

/**
 * Adds a 2D Vector to another.
 *
 * @param result The final vector.
 * @param vector0 The first vector.
 * @param vector1 The second vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector2AddVector2f(GLUSfloat result[2], const GLUSfloat vector0[2], const GLUSfloat vector1[2]);

/**
 * Subtracts a 3D Vector from another.
 *
 * @param result The final vector.
 * @param vector0 The vector subtracted by vector1.
 * @param vector1 The vector subtracted from vector0.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3SubtractVector3f(GLUSfloat result[3], const GLUSfloat vector0[3], const GLUSfloat vector1[3]);

/**
 * Subtracts a 2D Vector from another.
 *
 * @param result The final vector.
 * @param vector0 The vector subtracted by vector1.
 * @param vector1 The vector subtracted from vector0.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector2SubtractVector2f(GLUSfloat result[2], const GLUSfloat vector0[2], const GLUSfloat vector1[2]);

/**
 * Multiplies a 3D Vector by a scalar.
 *
 * @param result The final vector.
 * @param vector The used vector for multiplication.
 * @param scalar The scalar.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3MultiplyScalarf(GLUSfloat result[3], const GLUSfloat vector[3], const GLUSfloat scalar);

/**
 * Multiplies a 2D Vector by a scalar.
 *
 * @param result The final vector.
 * @param vector The used vector for multiplication.
 * @param scalar The scalar.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector2MultiplyScalarf(GLUSfloat result[2], const GLUSfloat vector[2], const GLUSfloat scalar);

/**
 * Calculates the length of a 3D Vector.
 *
 * @param vector The used vector.
 *
 * @return The length of the vector.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusVector3Lengthf(const GLUSfloat vector[3]);

/**
 * Calculates the length of a 2D Vector.
 *
 * @param vector The used vector.
 *
 * @return The length of the vector.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusVector2Lengthf(const GLUSfloat vector[2]);

/**
 * Normalizes the given 3D Vector.
 *
 * @param vector The vector to normalize.
 *
 * @return GLUS_TRUE, if normalization succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusVector3Normalizef(GLUSfloat vector[3]);

/**
 * Normalizes the given 2D Vector.
 *
 * @param vector The vector to normalize.
 *
 * @return GLUS_TRUE, if normalization succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusVector2Normalizef(GLUSfloat vector[2]);

/**
 * Calculates the dot product of two 3D vectors.
 *
 * @param vector0 The first vector.
 * @param vector1 The second vector.
 *
 * @return The dot product.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusVector3Dotf(const GLUSfloat vector0[3], const GLUSfloat vector1[3]);

/**
 * Calculates the dot product of two 2D vectors.
 *
 * @param vector0 The first vector.
 * @param vector1 The second vector.
 *
 * @return The dot product.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusVector2Dotf(const GLUSfloat vector0[2], const GLUSfloat vector1[2]);

/**
 * Calculates the cross product of two 3D vectors: vector0 x vector1.
 *
 * @param result The resulting vector from the cross product.
 * @param vector0 The first vector.
 * @param vector1 The second vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3Crossf(GLUSfloat result[3], const GLUSfloat vector0[3], const GLUSfloat vector1[3]);

/**
 * Creates a quaternion out of a 3D vector.
 *
 * @param result The resulting quaternion.
 * @param vector The used vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3GetQuaternionf(GLUSfloat result[4], const GLUSfloat vector[3]);

/**
 * Creates a 3D point, given as homogeneous coordinates, out of a 3D vector.
 *
 * @param result The resulting point.
 * @param vector The used vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3GetPoint4f(GLUSfloat result[4], const GLUSfloat vector[3]);

/**
 * Creates a 2D point, given as homogeneous coordinates, out of a 2D vector.
 *
 * @param result The resulting point.
 * @param vector The used vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector2GetPoint3f(GLUSfloat result[3], const GLUSfloat vector[2]);

/**
 * Creates a 3D vector orthogonal to u on the u/v plane.
 *
 * @param result The resulting vector.
 * @param u 	 The vector to calculate a orthogonal vector to.
 * @param v 	 The vector, which is projected to the orthogonal line of u.
 *
 * @return GLUS_TRUE, if a vector could be calculated.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusVector3GramSchmidtOrthof(GLUSfloat result[3], const GLUSfloat u[3], const GLUSfloat v[3]);

/**
 * Creates a 2D vector orthogonal to u.
 *
 * @param result The resulting vector.
 * @param u 	 The vector to calculate a orthogonal vector to.
 * @param v 	 The vector, which is projected to the orthogonal line of u.
 *
 * @return GLUS_TRUE, if a vector could be calculated.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusVector2GramSchmidtOrthof(GLUSfloat result[2], const GLUSfloat u[2], const GLUSfloat v[2]);

/**
 * Calculate the reflection direction for an incident vector.
 *
 * @param result	The reflection vector.
 * @param incident	The incident vector.
 * @param normal	The normal vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3Reflectf(GLUSfloat result[3], const GLUSfloat incident[3], const GLUSfloat normal[3]);

/**
 * Calculate the reflection direction for an incident vector.
 *
 * @param result	The reflection vector.
 * @param incident	The incident vector.
 * @param normal	The normal vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector2Reflectf(GLUSfloat result[2], const GLUSfloat incident[2], const GLUSfloat normal[2]);

/**
 * Calculate the refraction direction for an incident vector.
 *
 * @param result	The refraction vector.
 * @param incident	The incident vector.
 * @param normal	The normal vector.
 * @param eta		The ratio of indices of refraction.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3Refractf(GLUSfloat result[3], const GLUSfloat incident[3], const GLUSfloat normal[3], const float eta);

/**
 * Calculate the refraction direction for an incident vector.
 *
 * @param result	The refraction vector.
 * @param incident	The incident vector.
 * @param normal	The normal vector.
 * @param eta		The ratio of indices of refraction.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector2Refractf(GLUSfloat result[2], const GLUSfloat incident[2], const GLUSfloat normal[2], const float eta);

/**
 * Calculate the specular reflection coefficient, based on Schlick's approximation for the Fresnel term.
 *
 * @param incident	The incident vector.
 * @param normal	The normal vector.
 * @param R0		The reflection coefficient for light incoming parallel to the normal.
 *
 * @return Specular reflection coefficient.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusVector3Fresnelf(const GLUSfloat incident[3], const GLUSfloat normal[3], const GLUSfloat R0);

/**
 * Calculate the specular reflection coefficient, based on Schlick's approximation for the Fresnel term.
 *
 * @param incident	The incident vector.
 * @param normal	The normal vector.
 * @param R0		The reflection coefficient for light incoming parallel to the normal.
 *
 * @return Specular reflection coefficient.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusVector2Fresnelf(const GLUSfloat incident[2], const GLUSfloat normal[2], const GLUSfloat R0);

/**
 * Copies a Vector with N elements.
 *
 * @param result The destination vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVectorNCopyc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Converts a complex number Vector with N elements to a float Vector.
 *
 * @param result The destination vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVectorNComplexToFloatc(GLUSfloat* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Converts a float Vector with N elements to a complex Vector.
 *
 * @param result The destination vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVectorNFloatToComplexc(GLUScomplex* result, const GLUSfloat* vector, const GLUSint n);

/**
 * Multiplies a Vector with N elements by a scalar.
 *
 * @param result The final vector.
 * @param vector The used vector for multiplication.
 * @param n 	 The number of elements.
 * @param scalar The scalar.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVectorNMultiplyScalarc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n, const GLUSfloat scalar);

/**
 * Calculates the complex conjugate of a Vector with N elements.
 *
 * @param result The vector, containing the complex conjugate.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVectorNConjugatec(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

#endif /* GLUS_VECTOR_H_ */
