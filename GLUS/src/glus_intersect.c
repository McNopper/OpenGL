/*
 * GLUS - OpenGL 3 and 4 Utilities. Copyright (C) 2010 - 2013 Norbert Nopper
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

#include "GL/glus.h"

GLUSint GLUSAPIENTRY glusIntersectRaySpheref(GLUSfloat* tNear, GLUSfloat* tFar, GLUSboolean* insideSphere, const GLUSfloat rayStart[4], const GLUSfloat rayDirection[3], const GLUSfloat sphereCenter[4], const GLUSfloat radius)
{
	// see http://de.wikipedia.org/wiki/Quadratische_Gleichung (German)
	// see Real-Time Collision Detection p177

	GLUSint intersections = 0;
	GLUSfloat m[3];
	GLUSfloat b, c, discriminant, sqrtDiscriminant, t;

	glusPoint4SubtractPoint4f(m, rayStart, sphereCenter);

	b = glusVector3Dotf(m, rayDirection);
	c = glusVector3Dotf(m, m) - radius * radius;

	// Ray pointing away from sphere and ray starts outside sphere.
	if (b > 0.0f && c > 0.0f)
	{
		return intersections;
	}

	discriminant = b * b - c;

	// No solution, so no intersection. Ray passes the sphere.
	if (discriminant < 0.0f)
	{
		return intersections;
	}

	sqrtDiscriminant = sqrtf(discriminant);

	// If we come so far, we have at least one intersection.

	if (insideSphere)
	{
		*insideSphere = GLUS_FALSE;
	}

	t = -b - sqrtDiscriminant;

	// Ray starts inside the sphere, as t is negative.
	if (t < 0.0f)
	{
		// Use origin as intersection point.
		t = 0.0f;

		if (insideSphere)
		{
			*insideSphere = GLUS_TRUE;
		}
	}

	intersections++;

	if (tNear)
	{
		*tNear = t;
	}

	// Tangent point. Only one intersection. So leave.
	if (discriminant == 0.0f)
	{
		// Save in both for convenience.
		if (tFar)
		{
			*tFar = t;
		}

		return intersections;
	}

	//

	t = -b + sqrtDiscriminant;

	intersections++;

	if (tFar)
	{
		*tFar = t;
	}

	return intersections;
}
