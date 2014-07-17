/**
 * OpenGL 4 - Example 40
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#ifndef RENDER_SPHERE_H_
#define RENDER_SPHERE_H_

#include "GL/glus.h"

GLUSboolean initSphere(GLUSfloat sphereCenter[4], GLUSfloat sphereRadius, GLUSfloat lightDirection[3]);

GLUSvoid reshapeSphere(GLUSfloat viewProjectionMatrix[16]);

GLUSboolean updateSphere(GLUSfloat time);

GLUSvoid terminateSphere(GLUSvoid);

#endif /* RENDER_SPHERE_H_ */
