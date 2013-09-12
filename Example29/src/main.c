/**
 * OpenGL 3 - Example 29
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 *
 * see http://www.scratchapixel.com/lessons/3d-basic-lessons/lesson-1-writing-a-simple-raytracer/
 */

#include <stdio.h>

#include "GL/glus.h"

#define WIDTH 640
#define HEIGHT 480
#define BYTES_PER_PIXEL 3
#define NUM_SPHERES 3

#define MAX_RAY_DEPTH 5

// Indices of refraction
#define Air 1.000293f
#define Glass 1.51714f

// Air to glass ratio of the indices of refraction (Eta)
#define Eta (Air / Glass)

// see http://en.wikipedia.org/wiki/Refractive_index Reflectivity
#define R0 (((Air - Glass) * (Air - Glass)) / ((Air + Glass) * (Air + Glass)))

typedef struct _Sphere
{
	GLfloat center[4];

	GLfloat radius;

	GLfloat diffuseColor[4];

	GLfloat emissiveColor[4];

	GLfloat reflectivity;

	GLfloat transparency;

} Sphere;

/**
 * The used shader program.
 */
static GLUSshaderprogram g_program;

/**
 * VAO is needed. Otherwise glDrawArrays will not draw.
 */
static GLuint g_vao;

/**
 * The location of the texture in the shader program.
 */
static GLint g_textureLocation;

/**
 * The texture, which is filled with the ray traced picture.
 */
static GLuint g_texture = 0;

/**
 * Buffer, which contains the ray directions.
 */
static GLfloat g_directionBuffer[WIDTH * HEIGHT * 3];

/**
 * Buffer, which contains the ray position.
 */
static GLfloat g_positionBuffer[WIDTH * HEIGHT * 4];

Sphere g_allSpheres[NUM_SPHERES] = {
		// Ground as sphere
		{ { 0.0f, -10001.0f, -20.0f }, 10000.0f, { 0.4f, 0.4f, 0.4f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, 0.0f, 0.0f },
		// One sphere
		{ { 0.0f, 0.0f, -10.0f }, 1.0f, { 0.8f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, 0.0f, 0.0f },
		// Light as sphere using emissive light
		{ { -10.0f, 10.0f, 10.0f }, 10.0f, { 0.0f, 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 0.0f, 0.0f }
};

static GLvoid trace(GLfloat pixelColor[4], const GLfloat rayPosition[4], const GLfloat rayDirection[3], const GLint depth)
{
	const GLfloat bias = 1e-4f;

	GLint i, k;

	GLfloat tNear = INFINITY;
	Sphere* sphereNear = 0;
	GLboolean insideSphereNear = GL_FALSE;

	GLfloat ray[3];

	GLfloat hitPosition[4];
	GLfloat hitDirection[3];

	GLfloat biasedPositiveHitPosition[4];
	GLfloat biasedNegativeHitPosition[4];
	GLfloat biasedHitDirection[3];

	//

	pixelColor[0] = 0.0f;
	pixelColor[1] = 0.0f;
	pixelColor[2] = 0.0f;
	pixelColor[3] = 1.0f;

	//

	for (i = 0; i < NUM_SPHERES; i++)
	{
		GLfloat t0 = INFINITY;
		GLfloat t1 = INFINITY;
		GLboolean insideSphere = GL_FALSE;

		Sphere* currentSphere = &g_allSpheres[i];

		GLint numberIntersections = glusIntersectRaySpheref(&t0, &t1, &insideSphere, rayPosition, rayDirection, currentSphere->center, currentSphere->radius);

		if (numberIntersections)
		{
			// If intersection happened inside the sphere, take second intersection point, as this one is on the surface.
			if (insideSphere)
			{
				t0 = t1;
			}

			// Found a sphere, which is closer.
			if (t0 < tNear)
			{
				tNear = t0;
				sphereNear = currentSphere;
				insideSphereNear = insideSphere;
			}
		}
	}

	//

	// No intersection, return background color / ambient light.
	if (!sphereNear)
	{
		pixelColor[0] = 0.0f;
		pixelColor[1] = 0.0f;
		pixelColor[2] = 0.0f;

		return;
	}

	// Calculate ray hit position ...
	glusVector3MultiplyScalarf(ray, rayDirection, tNear);
	glusPoint4AddVector3f(hitPosition, rayPosition, ray);

	// ... and normal
	glusPoint4SubtractPoint4f(hitDirection, hitPosition, sphereNear->center);
	glusVector3Normalizef(hitDirection);

	// If inside the sphere, reverse hit vector, as ray comes from inside.
	if (insideSphereNear)
	{
		glusVector3MultiplyScalarf(hitDirection, hitDirection, -1.0f);
	}

	//

	// Biasing, to avoid artifacts.
	glusVector3MultiplyScalarf(biasedHitDirection, hitDirection, bias);
	glusPoint4AddVector3f(biasedPositiveHitPosition, hitPosition, biasedHitDirection);
	glusPoint4SubtractVector3f(biasedNegativeHitPosition, hitPosition, biasedHitDirection);

	//

	GLfloat reflectionColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	GLfloat refractionColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};

	GLfloat fresnel = glusVector3Fresnelf(rayDirection, hitDirection, R0);

	// Reflection ...
	if (sphereNear->reflectivity > 0.0f && depth < MAX_RAY_DEPTH)
	{
		GLfloat reflectionDirection[3];

		glusVector3Reflectf(reflectionDirection, rayDirection, hitDirection);
		glusVector3Normalizef(reflectionDirection);

		trace(reflectionColor, biasedPositiveHitPosition, reflectionDirection, depth + 1);
	}

	// ... refraction
	if (sphereNear->transparency > 0.0f && depth < MAX_RAY_DEPTH)
	{
		GLfloat refractionDirection[3];

		// If inside, it is from glass to air.
		GLfloat eta = insideSphereNear ? 1.0f / Eta : Eta;

		glusVector3Refractf(refractionDirection, rayDirection, hitDirection, eta);
		glusVector3Normalizef(refractionDirection);

		trace(refractionColor, biasedNegativeHitPosition, refractionDirection, depth + 1);
	}

	// TODO Add reflection and refraction depending on fresnel value

	// Diffuse
	for (i = 0; i < NUM_SPHERES; i++)
	{
		Sphere* lightSphere = &g_allSpheres[i];

		if (lightSphere == sphereNear)
		{
			continue;
		}

		// Treat any emissive color as a point light
		if (lightSphere->emissiveColor[0] > 0.0f || lightSphere->emissiveColor[1] > 0.0f || lightSphere->emissiveColor[2] > 0.0f)
		{
			GLboolean obstacle = GL_FALSE;
			GLfloat lightDirection[3];

			glusPoint4SubtractPoint4f(lightDirection, lightSphere->center, hitPosition);
			glusVector3Normalizef(lightDirection);

			// Check for obstacles between current hit point surface and light.
			for (k = 0; k < NUM_SPHERES; k++)
			{
				Sphere* obstacleSphere = &g_allSpheres[k];

				if (obstacleSphere == sphereNear || obstacleSphere == lightSphere)
				{
					continue;
				}

				if (glusIntersectRaySpheref(0, 0, 0, biasedPositiveHitPosition, lightDirection, obstacleSphere->center, obstacleSphere->radius))
				{
					obstacle = GL_TRUE;

					break;
				}
			}

			// If no obstacle, illuminate hit point surface.
			if (!obstacle)
			{
				GLfloat intensity = glusMaxf(0.0f, glusVector3Dotf(hitDirection, lightDirection));

				pixelColor[0] = pixelColor[0] + intensity * sphereNear->diffuseColor[0] * lightSphere->emissiveColor[0];
				pixelColor[1] = pixelColor[1] + intensity * sphereNear->diffuseColor[1] * lightSphere->emissiveColor[1];
				pixelColor[2] = pixelColor[2] + intensity * sphereNear->diffuseColor[2] * lightSphere->emissiveColor[2];
			}
		}
	}

	// Emissive
	pixelColor[0] = pixelColor[0] + sphereNear->emissiveColor[0];
	pixelColor[1] = pixelColor[1] + sphereNear->emissiveColor[1];
	pixelColor[2] = pixelColor[2] + sphereNear->emissiveColor[2];
}

static GLboolean renderToPixelBuffer(GLubyte* pixels, const GLint width, const GLint height)
{
	GLint x, y, index;

	GLfloat pixelColor[4];

	// Generate the ry directions depending on FOV, width and height.
	if (!glusRaytracePerspectivef(g_directionBuffer, 30.0f, width, height))
	{
		printf("Error: Could not create direction buffer.\n");

		return GLUS_FALSE;
	}

	// Rendering only once, so direction buffer can be overwritten.
	glusRaytraceLookAtf(g_positionBuffer, g_directionBuffer, g_directionBuffer, width, height, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

	// Ray tracing over all pixels

	for (y = 0; y < HEIGHT; y++)
	{
		for (x = 0; x < WIDTH; x++)
		{
			index = (x + y * WIDTH);

			trace(pixelColor, &g_positionBuffer[index * 4], &g_directionBuffer[index * 3], 0);

			// Resolve to pixel buffer, which is used for the texture.

			pixels[index * BYTES_PER_PIXEL + 0] = (GLubyte)(glusMinf(1.0f, pixelColor[0]) * 255.0f);
			pixels[index * BYTES_PER_PIXEL + 1] = (GLubyte)(glusMinf(1.0f, pixelColor[1]) * 255.0f);
			pixels[index * BYTES_PER_PIXEL + 2] = (GLubyte)(glusMinf(1.0f, pixelColor[2]) * 255.0f);
		}
	}

	return GLUS_TRUE;
}

/**
 * Function for initialization.
 */
GLUSboolean init(GLUSvoid)
{
	GLubyte pixels[WIDTH * HEIGHT * BYTES_PER_PIXEL];

	//

	GLUStextfile vertexSource;
	GLUStextfile fragmentSource;

	// Render (CPU) into pixel buffer

	if (!renderToPixelBuffer(pixels, WIDTH, HEIGHT))
	{
		printf("Error: Could not render to pixel buffer.\n");

		return GLUS_FALSE;
	}

	// Load full screen rendering shaders

	glusLoadTextFile("../Example29/shader/fullscreen.vert.glsl", &vertexSource);
	glusLoadTextFile("../Example29/shader/texture.frag.glsl", &fragmentSource);

	glusBuildProgramFromSource(&g_program, (const GLchar**)&vertexSource.text, 0, 0, 0, (const GLchar**)&fragmentSource.text);

	glusDestroyTextFile(&vertexSource);
	glusDestroyTextFile(&fragmentSource);

	g_textureLocation = glGetUniformLocation(g_program.program, "u_texture");

	// Generate texture

	glGenTextures(1, &g_texture);
	glBindTexture(GL_TEXTURE_2D, g_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

	//

	glUseProgram(g_program.program);

	//

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

	//

	glBindTexture(GL_TEXTURE_2D, g_texture);
	glUniform1i(g_textureLocation, 0);

	return GLUS_TRUE;
}

/**
 * Function is called before first update and every time when the window is resized.
 *
 * @param w	width of the window
 * @param h	height of the window
 */
GLUSvoid reshape(GLUSint width, GLUSint height)
{
	glViewport(0, 0, width, height);
}

/**
 * Function to render and display content. Swapping of the buffers is automatically done.
 *
 * @return true for continuing, false to exit the application
 */
GLUSboolean update(GLUSfloat time)
{
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	return GLUS_TRUE;
}

/**
 * Function to clean up things.
 */
GLUSvoid terminate(GLUSvoid)
{
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_texture)
	{
		glDeleteTextures(1, &g_texture);

		g_texture = 0;
	}

	//

	glBindVertexArray(0);

	if (g_vao)
	{
		glDeleteVertexArrays(1, &g_vao);

		g_vao = 0;
	}

	//

	glUseProgram(0);

	glusDestroyProgram(&g_program);
}

/**
 * Main entry point.
 */
int main(int argc, char* argv[])
{
	glusInitFunc(init);

	glusReshapeFunc(reshape);

	glusUpdateFunc(update);

	glusTerminateFunc(terminate);

	glusPrepareContext(3, 2, GLUS_FORWARD_COMPATIBLE_BIT);

	// To keep the program simple, no resize of the window.
	glusPrepareNoResize(GLUS_TRUE);

	if (!glusCreateWindow("GLUS Example Window", WIDTH, HEIGHT, 0, 0, GLUS_FALSE))
	{
		printf("Could not create window!\n");
		return -1;
	}

	glusRun();

	return 0;
}
