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

#ifndef GLUS_ANIMATION_H_
#define GLUS_ANIMATION_H_

/**
 * Keyframe interpolation: output equals the previous keyframe value (no blending).
 */
#define GLUS_ANIMATION_STEP 0

/**
 * Keyframe interpolation: linear interpolation between adjacent keyframe values.
 * Quaternion tracks use normalized linear interpolation (nlerp).
 */
#define GLUS_ANIMATION_LINEAR 1

/**
 * Keyframe interpolation: cubic Hermite spline.
 * Each keyframe stores three consecutive values: inTangent, vertex, outTangent.
 * The values array stride per keyframe is therefore 3 * components
 * (9 floats for vec3, 12 floats for quaternion).
 */
#define GLUS_ANIMATION_CUBICSPLINE 2

/**
 * Sample a vec3 keyframe track at time t.
 *
 * @param result       Output vec3.
 * @param times        Array of count keyframe times in seconds (ascending).
 * @param values       Keyframe values. Layout depends on interpolation:
 *                       STEP / LINEAR:   count * 3 floats (one vec3 per keyframe).
 *                       CUBICSPLINE:     count * 9 floats (inTangent[3], vertex[3],
 *                                        outTangent[3] per keyframe).
 * @param count        Number of keyframes.
 * @param interpolation GLUS_ANIMATION_STEP, GLUS_ANIMATION_LINEAR, or
 *                       GLUS_ANIMATION_CUBICSPLINE.
 * @param t            Current time in seconds.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusAnimationSampleVec3f(GLUSfloat result[3], const GLUSfloat* times, const GLUSfloat* values, GLUSint count, GLUSint interpolation, GLUSfloat t);

/**
 * Sample a quaternion keyframe track at time t.
 *
 * Quaternions are stored as (x, y, z, w).  For LINEAR interpolation the two
 * adjacent quaternions are blended with nlerp; the sign of the second quaternion
 * is flipped when needed to ensure the shortest-arc rotation.  CUBICSPLINE output
 * is normalized after evaluation.
 *
 * @param result       Output quaternion (x, y, z, w).
 * @param times        Array of count keyframe times in seconds (ascending).
 * @param values       Keyframe values. Layout depends on interpolation:
 *                       STEP / LINEAR:   count * 4 floats (one quaternion per keyframe).
 *                       CUBICSPLINE:     count * 12 floats (inTangent[4], vertex[4],
 *                                        outTangent[4] per keyframe).
 * @param count        Number of keyframes.
 * @param interpolation GLUS_ANIMATION_STEP, GLUS_ANIMATION_LINEAR, or
 *                       GLUS_ANIMATION_CUBICSPLINE.
 * @param t            Current time in seconds.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusAnimationSampleQuaternionf(GLUSfloat result[4], const GLUSfloat* times, const GLUSfloat* values, GLUSint count, GLUSint interpolation, GLUSfloat t);

#endif /* GLUS_ANIMATION_H_ */
