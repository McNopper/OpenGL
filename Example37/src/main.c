/**
 * OpenGL 3 - Example 37
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 *
 * see http://9bitscience.blogspot.com/2013/07/raymarching-distance-fields_14.html
 * see http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
 */

#include <stdio.h>

#include "GL/glus.h"

#define WIDTH 640
#define HEIGHT 480

#define BYTES_PER_PIXEL 3

#define NUM_BOXES 2
#define NUM_SPHERES 3
#define NUM_LIGHTS 1

// Distance, when marching should stop.
#define MAX_DISTANCE 50.0f
// Maximum ray marching steps.
#define MAX_STEPS 256

// Distance, when a hit occurred.
#define EPSILON 0.01f
// Sample point offset.
#define GAMMA 0.001f

typedef struct _Material
{
	GLfloat emissiveColor[4];

	GLfloat diffuseColor[4];

	GLfloat specularColor[4];

	GLfloat shininess;

} Material;

typedef struct _Sphere
{
	GLfloat center[4];

	GLfloat radius;

} Sphere;

typedef struct _Box
{
	GLfloat center[4];

	GLfloat halfExtend[3];

	GLfloat orientation[3];

} Box;

typedef struct _Primitive
{
	void* data;

	GLfloat(*distanceFunction)(const GLfloat point[4], const struct _Primitive* primitive);

	Material material;

} Primitive;

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
 * The texture, which is filled with the ray marched picture.
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

// Distance functions for sphere and oriented box.
// see http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm

static GLfloat distanceFunctionSphere(const GLfloat point[4], const Primitive* primitive)
{
	Sphere* sphere = (Sphere*)primitive->data;

	return glusSphereDistancePoint4f(sphere->center, sphere->radius, point);
}

static GLfloat distanceFunctionOrientedBox(const GLfloat point[4], const Primitive* primitive)
{
	Box* box = (Box*)primitive->data;

	return glusOrientedBoxDistancePoint4f(box->center, box->halfExtend, box->orientation, point);
}

Sphere g_allSpheres[NUM_SPHERES] = {
		{ { 2.0f, 1.0f, -14.0f, 1.0f }, 2.0f },
		{ { -2.0f, 0.25f, -6.0f, 1.0f }, 1.25f },
		{ { 3.0f, 0.0f, -8.0f, 1.0f }, 1.0f }
};

Box g_allBoxes[NUM_BOXES] = {
		{ { 0.0f, -2.0f, -10.0f, 1.0f }, { 10.0f, 1.0f, 20.0f }, {0.0f, 0.0f, 0.0f} },
		{ { -1.0f, -0.8f, -10.0f, 1.0f }, { 0.5f, 0.2f, 1.0f }, {0.0f, 20.0f, 0.0f} }
};

Primitive g_allPrimitives[NUM_SPHERES + NUM_BOXES] = {
		// Blue sphere
		{&g_allSpheres[0], distanceFunctionSphere, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.8f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f } },
		// Green sphere
		{&g_allSpheres[1], distanceFunctionSphere, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.8f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f } },
		// Red sphere
		{&g_allSpheres[2], distanceFunctionSphere, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.8f, 0.0f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f } },

		// Grey ground box
		{&g_allBoxes[0], distanceFunctionOrientedBox, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f }  },
		// Turquoise box
		{&g_allBoxes[1], distanceFunctionOrientedBox, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.8f, 0.8f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f }  }
};

PointLight g_allLights[NUM_LIGHTS] = {
		{{0.0f, 5.0f, -5.0f, 1.0f}, { 1.0f, 1.0f, 1.0f, 1.0f }}
};

static GLvoid march(GLfloat pixelColor[4], const GLfloat rayPosition[4], const GLfloat rayDirection[3], const GLint depth)
{
	GLint i, k, m, o;

	Primitive* primitiveNear = 0;

	GLfloat marchPosition[4];
	GLfloat marchDirection[3];

	GLfloat hitPosition[4];
	GLfloat hitDirection[3];

	GLfloat eyeDirection[3];

	GLfloat t = 0.0f;

	//

	pixelColor[0] = 0.0f;
	pixelColor[1] = 0.0f;
	pixelColor[2] = 0.0f;
	pixelColor[3] = 1.0f;

	//

	for (i = 0; i < MAX_STEPS; i++)
	{
		GLfloat distance = INFINITY;
		GLfloat currentDistance;
		Primitive* closestPrimitive = 0;

		glusVector3MultiplyScalarf(marchDirection, rayDirection, t);
		glusPoint4AddVector3f(marchPosition, rayPosition, marchDirection);

		for (k = 0; k < NUM_SPHERES + NUM_BOXES; k++)
		{
			Primitive* currentPrimitive = &g_allPrimitives[k];

			currentDistance = currentPrimitive->distanceFunction(marchPosition, currentPrimitive);

			if (currentDistance < distance)
			{
				distance = currentDistance;

				closestPrimitive = currentPrimitive;
			}
		}

		if (distance < EPSILON)
		{
			GLfloat samplePoints[4 * 6];

			// Copy hit position ...
			glusPoint4Copyf(hitPosition, marchPosition);

			// ... and calculate normal by sampling around the hit position.
			for (k = 0; k < 6; k++)
			{
				samplePoints[k*4+0] = hitPosition[0];
				samplePoints[k*4+1] = hitPosition[1];
				samplePoints[k*4+2] = hitPosition[2];
				samplePoints[k*4+3] = hitPosition[3];

				samplePoints[k*4+k/2] += GAMMA * (k%2 == 0 ? 1.0f : -1.0f);
			}
			for (k = 0; k < 3; k++)
			{
				hitDirection[k] = closestPrimitive->distanceFunction(&samplePoints[k*2*4 + 0], closestPrimitive) - closestPrimitive->distanceFunction(&samplePoints[k*2*4 + 4], closestPrimitive);
			}

			glusVector3Normalizef(hitDirection);

			primitiveNear = closestPrimitive;

			break;
		}

		// If t is larger than a given distance, assume that there is the nothing.
		if (t > MAX_DISTANCE)
		{
			break;
		}

		t += distance;
	}

	//

	// No intersection, return background color / ambient light.
	if (!primitiveNear)
	{
		pixelColor[0] = 0.8f;
		pixelColor[1] = 0.8f;
		pixelColor[2] = 0.8f;

		return;
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
		for (k = 0; k < NUM_SPHERES + NUM_BOXES; k++)
		{
			Primitive* obstaclePrimitive = &g_allPrimitives[k];

			if (obstaclePrimitive == primitiveNear)
			{
				continue;
			}

			t = 0.0f;

			for (m = 0; m < MAX_STEPS; m++)
			{
				GLfloat distance = INFINITY;
				GLfloat currentDistance;
				Primitive* closestPrimitive = 0;

				glusVector3MultiplyScalarf(marchDirection, lightDirection, -t);
				glusPoint4AddVector3f(marchPosition, pointLight->position, marchDirection);

				for (o = 0; o < NUM_SPHERES + NUM_BOXES; o++)
				{
					Primitive* currentPrimitive = &g_allPrimitives[o];

					currentDistance = currentPrimitive->distanceFunction(marchPosition, currentPrimitive);

					if (currentDistance < distance && currentDistance >= 0.0f)
					{
						distance = currentDistance;

						closestPrimitive = currentPrimitive;
					}
					else if (currentDistance > distance && currentDistance < 0.0f)
					{
						distance = currentDistance;

						closestPrimitive = currentPrimitive;
					}
				}

				if (distance < EPSILON)
				{
					if (closestPrimitive != primitiveNear)
					{
						obstacle = GL_TRUE;
					}

					break;
				}

				if (t > MAX_DISTANCE)
				{
					break;
				}

				t += distance;
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

				pixelColor[0] = pixelColor[0] + diffuseIntensity * primitiveNear->material.diffuseColor[0] * pointLight->color[0];
				pixelColor[1] = pixelColor[1] + diffuseIntensity * primitiveNear->material.diffuseColor[1] * pointLight->color[1];
				pixelColor[2] = pixelColor[2] + diffuseIntensity * primitiveNear->material.diffuseColor[2] * pointLight->color[2];

				glusVector3Reflectf(specularReflection, incidentLightDirection, hitDirection);
				glusVector3Normalizef(specularReflection);

				eDotR = glusMathMaxf(0.0f, glusVector3Dotf(eyeDirection, specularReflection));

				if (eDotR > 0.0f)
				{
					GLfloat specularIntensity = powf(eDotR, primitiveNear->material.shininess);

					pixelColor[0] = pixelColor[0] + specularIntensity * primitiveNear->material.specularColor[0] * pointLight->color[0];
					pixelColor[1] = pixelColor[1] + specularIntensity * primitiveNear->material.specularColor[1] * pointLight->color[1];
					pixelColor[2] = pixelColor[2] + specularIntensity * primitiveNear->material.specularColor[2] * pointLight->color[2];
				}
			}
		}
	}

	// Emissive color
	pixelColor[0] = pixelColor[0] + primitiveNear->material.emissiveColor[0];
	pixelColor[1] = pixelColor[1] + primitiveNear->material.emissiveColor[1];
	pixelColor[2] = pixelColor[2] + primitiveNear->material.emissiveColor[2];
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

	// Ray marching over all pixels

	for (y = 0; y < HEIGHT; y++)
	{
		for (x = 0; x < WIDTH; x++)
		{
			index = (x + y * WIDTH);

			march(pixelColor, &g_positionBuffer[index * 4], &g_directionBuffer[index * 3], 0);

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

	glusFileLoadText("../Example37/shader/fullscreen.vert.glsl", &vertexSource);
	glusFileLoadText("../Example37/shader/texture.frag.glsl", &fragmentSource);

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
