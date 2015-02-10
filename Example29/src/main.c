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
#define NUM_SPHERES 6
#define NUM_LIGHTS 1

#define MAX_RAY_DEPTH 5

// Indices of refraction
#define Air 1.0f
#define Bubble 1.06f

// Air to glass ratio of the indices of refraction (Eta)
#define Eta (Air / Bubble)

// see http://en.wikipedia.org/wiki/Refractive_index Reflectivity
#define R0 (((Air - Bubble) * (Air - Bubble)) / ((Air + Bubble) * (Air + Bubble)))

typedef struct _Material
{
	GLfloat emissiveColor[4];

	GLfloat diffuseColor[4];

	GLfloat specularColor[4];

	GLfloat shininess;

	GLfloat alpha;

	GLfloat reflectivity;

} Material;

typedef struct _Sphere
{
	GLfloat center[4];

	GLfloat radius;

	Material material;

} Sphere;

typedef struct _PointLight
{
	GLfloat position[4];

	GLfloat color[4];

} PointLight;

/**
 * The used shader program.
 */
static GLUSprogram g_program;

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
		// Ground sphere
		{ { 0.0f, -10001.0f, -20.0f, 1.0f }, 10000.0f, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.4f, 0.4f, 0.4f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f } },
		// Transparent sphere
		{ { 0.0f, 0.0f, -10.0f, 1.0f }, 1.0f, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f, 0.2f, 1.0f } },
		// Reflective sphere
		{ { 1.0f, -0.75f, -7.0f, 1.0f }, 0.25f, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f, 1.0f, 0.8f } },
		// Blue sphere
		{ { 2.0f, 1.0f, -16.0f, 1.0f }, 2.0f, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.8f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f, 1.0f, 0.2f } },
		// Green sphere
		{ { -2.0f, 0.25f, -6.0f, 1.0f }, 1.25f, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.8f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f, 1.0f, 0.2f } },
		// Red sphere
		{ { 3.0f, 0.0f, -8.0f, 1.0f }, 1.0f, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.8f, 0.0f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f, 1.0f, 0.2f } }
};

PointLight g_allLights[NUM_LIGHTS] = {
		{{0.0f, 5.0f, -5.0f, 1.0f}, { 1.0f, 1.0f, 1.0f, 1.0f }}
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

	GLfloat eyeDirection[3];

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
		pixelColor[0] = 0.8f;
		pixelColor[1] = 0.8f;
		pixelColor[2] = 0.8f;

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
	if (sphereNear->material.reflectivity > 0.0f && depth < MAX_RAY_DEPTH)
	{
		GLfloat reflectionDirection[3];

		glusVector3Reflectf(reflectionDirection, rayDirection, hitDirection);
		glusVector3Normalizef(reflectionDirection);

		trace(reflectionColor, biasedPositiveHitPosition, reflectionDirection, depth + 1);
	}

	// ... refraction.
	if (sphereNear->material.alpha < 1.0f && depth < MAX_RAY_DEPTH)
	{
		GLfloat refractionDirection[3];

		// If inside, it is from glass to air.
		GLfloat eta = insideSphereNear ? 1.0f / Eta : Eta;

		glusVector3Refractf(refractionDirection, rayDirection, hitDirection, eta);
		glusVector3Normalizef(refractionDirection);

		trace(refractionColor, biasedNegativeHitPosition, refractionDirection, depth + 1);
	}
	else
	{
		fresnel = 1.0f;
	}

	//

	glusVector3MultiplyScalarf(eyeDirection, rayDirection, -1.0f);

	// Diffuse and specular color
	for (i = 0; i < NUM_LIGHTS; i++)
	{
		PointLight* pointLight = &g_allLights[i];

		GLboolean obstacle = GL_FALSE;
		GLfloat lightDirection[3];
		GLfloat incidentLightDirection[3];

		glusPoint4SubtractPoint4f(lightDirection, pointLight->position, hitPosition);
		glusVector3Normalizef(lightDirection);
		glusVector3MultiplyScalarf(incidentLightDirection, lightDirection, -1.0f);

		// Check for obstacles between current hit point surface and point light.
		for (k = 0; k < NUM_SPHERES; k++)
		{
			Sphere* obstacleSphere = &g_allSpheres[k];

			if (obstacleSphere == sphereNear)
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
			GLfloat diffuseIntensity = glusMathMaxf(0.0f, glusVector3Dotf(hitDirection, lightDirection));

			if (diffuseIntensity > 0.0f)
			{
				GLfloat specularReflection[3];

				GLfloat eDotR;

				pixelColor[0] = pixelColor[0] + diffuseIntensity * sphereNear->material.diffuseColor[0] * pointLight->color[0];
				pixelColor[1] = pixelColor[1] + diffuseIntensity * sphereNear->material.diffuseColor[1] * pointLight->color[1];
				pixelColor[2] = pixelColor[2] + diffuseIntensity * sphereNear->material.diffuseColor[2] * pointLight->color[2];

				glusVector3Reflectf(specularReflection, incidentLightDirection, hitDirection);
				glusVector3Normalizef(specularReflection);

				eDotR = glusMathMaxf(0.0f, glusVector3Dotf(eyeDirection, specularReflection));

				if (eDotR > 0.0f && !insideSphereNear)
				{
					GLfloat specularIntensity = powf(eDotR, sphereNear->material.shininess);

					pixelColor[0] = pixelColor[0] + specularIntensity * sphereNear->material.specularColor[0] * pointLight->color[0];
					pixelColor[1] = pixelColor[1] + specularIntensity * sphereNear->material.specularColor[1] * pointLight->color[1];
					pixelColor[2] = pixelColor[2] + specularIntensity * sphereNear->material.specularColor[2] * pointLight->color[2];
				}
			}
		}
	}

	// Emissive color
	pixelColor[0] = pixelColor[0] + sphereNear->material.emissiveColor[0];
	pixelColor[1] = pixelColor[1] + sphereNear->material.emissiveColor[1];
	pixelColor[2] = pixelColor[2] + sphereNear->material.emissiveColor[2];

	// Final color with reflection and refraction
	pixelColor[0] = (1.0f - fresnel) * refractionColor[0] * (1.0f - sphereNear->material.alpha) + pixelColor[0] * (1.0f - sphereNear->material.reflectivity) * sphereNear->material.alpha + fresnel * reflectionColor[0] * sphereNear->material.reflectivity;
	pixelColor[1] = (1.0f - fresnel) * refractionColor[1] * (1.0f - sphereNear->material.alpha) + pixelColor[1] * (1.0f - sphereNear->material.reflectivity) * sphereNear->material.alpha + fresnel * reflectionColor[1] * sphereNear->material.reflectivity;
	pixelColor[2] = (1.0f - fresnel) * refractionColor[2] * (1.0f - sphereNear->material.alpha) + pixelColor[2] * (1.0f - sphereNear->material.reflectivity) * sphereNear->material.alpha + fresnel * reflectionColor[2] * sphereNear->material.reflectivity;
}

static GLboolean renderToPixelBuffer(GLubyte* pixels, const GLint width, const GLint height)
{
	GLint x, y, index;

	GLfloat pixelColor[4];

	// Generate the ray directions depending on FOV, width and height.
	if (!glusRaytracePerspectivef(g_directionBuffer, 0, 30.0f, width, height))
	{
		printf("Error: Could not create direction buffer.\n");

		return GLUS_FALSE;
	}

	// Rendering only once, so direction buffer can be overwritten.
	glusRaytraceLookAtf(g_positionBuffer, g_directionBuffer, g_directionBuffer, 0, width, height, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

	// Ray tracing over all pixels

	for (y = 0; y < HEIGHT; y++)
	{
		for (x = 0; x < WIDTH; x++)
		{
			index = (x + y * WIDTH);

			trace(pixelColor, &g_positionBuffer[index * 4], &g_directionBuffer[index * 3], 0);

			// Resolve to pixel buffer, which is used for the texture.

			pixels[index * BYTES_PER_PIXEL + 0] = (GLubyte)(glusMathMinf(1.0f, pixelColor[0]) * 255.0f);
			pixels[index * BYTES_PER_PIXEL + 1] = (GLubyte)(glusMathMinf(1.0f, pixelColor[1]) * 255.0f);
			pixels[index * BYTES_PER_PIXEL + 2] = (GLubyte)(glusMathMinf(1.0f, pixelColor[2]) * 255.0f);
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

	glusFileLoadText("../Example29/shader/fullscreen.vert.glsl", &vertexSource);
	glusFileLoadText("../Example29/shader/texture.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_program, (const GLchar**)&vertexSource.text, 0, 0, 0, (const GLchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

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

	glusProgramDestroy(&g_program);
}

/**
 * Main entry point.
 */
int main(int argc, char* argv[])
{
	EGLint eglConfigAttributes[] = {
	        EGL_RED_SIZE, 8,
	        EGL_GREEN_SIZE, 8,
	        EGL_BLUE_SIZE, 8,
	        EGL_DEPTH_SIZE, 0,
	        EGL_STENCIL_SIZE, 0,
	        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
	        EGL_NONE
	};

    EGLint eglContextAttributes[] = {
    		EGL_CONTEXT_MAJOR_VERSION, 3,
    		EGL_CONTEXT_MINOR_VERSION, 2,
    		EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
    		EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
    		EGL_NONE
    };

    glusWindowSetInitFunc(init);

    glusWindowSetReshapeFunc(reshape);

    glusWindowSetUpdateFunc(update);

    glusWindowSetTerminateFunc(terminate);

	// To keep the program simple, no resize of the window.
    if (!glusWindowCreate("GLUS Example Window", WIDTH, HEIGHT, GLUS_FALSE, GLUS_TRUE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusWindowRun();

    return 0;
}
