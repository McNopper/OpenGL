/**
 * OpenGL 4 - Example 31
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

#define TEXTURE_WIDTH 1024
#define TEXTURE_HEIGHT 768

#define OBJECTS_COLUMNS 16

#define POINT_LIGHT_COUNT 16
#define POINT_LIGHT_RADIUS 2.0f

/**
 * Properties of the material, basically all the color factors without the emissive and ambient color component.
 */
struct MaterialProperties
{
	GLfloat diffuseColor[4];
	GLfloat specularColor[4];
	GLfloat specularExponent;
};

/**
 * Locations for the material properties. With a diffuse texture.
 */
struct MaterialLocations
{
	GLint diffuseColorLocation;
	GLint specularColorLocation;
	GLint specularExponentLocation;

	GLint diffuseTextureLocation;
};

static GLfloat g_projectionMatrix[16];

static GLfloat g_positionMatrix[16];

static GLfloat g_directionMatrix[16];

//
//
//

static GLUSprogram g_programPointLight;

static GLint g_projectionMatrixPointLightLocation;

static GLint g_viewMatrixPointLightLocation;

static GLint g_modelMatrixPointLightLocation;

static GLint g_positionMatrixPointLightLocation;

static GLint g_biasMatrixPointLightLocation;

static GLint g_radiusPointLightLocation;

static GLint g_diffusePointLightLocation;
static GLint g_specularPointLightLocation;
static GLint g_positionPointLightLocation;
static GLint g_normalPointLightLocation;

static GLint g_vertexPointLightLocation;

static GLuint g_verticesPointLightVBO;

static GLuint g_indicesPointLightVBO;

static GLuint g_vaoPointLight;

static GLuint g_numberIndicesPointLight;

//
//
//

static GLUSprogram g_programDeferredShading;

static GLint g_projectionMatrixLocation;

static GLint g_viewMatrixLocation;

static GLint g_modelMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

static GLint g_texCoordLocation;

static GLint g_useTextureLocation;

/**
 * The locations for the material properties.
 */
static struct MaterialLocations g_material;

/**
 * This structure contains the loaded wavefront object file.
 */
static GLUSwavefront g_wavefront;

//
// Textures and buffers for deferred shading
//

static GLuint g_dsDiffuseTexture;

static GLuint g_dsSpecularTexture;

/**
 * The specular exponent is stored into w of the position texture.
 */
static GLuint g_dsPositionTexture;

static GLuint g_dsNormalTexture;

static GLuint g_dsDepthTexture;

static GLuint g_dsFBO;

GLUSboolean init(GLUSvoid)
{
	GLUStextfile vertexSource;
	GLUStextfile fragmentSource;

	GLUStgaimage image;

	GLUSgroupList* groupWalker;
	GLUSmaterialList* materialWalker;

	GLUSshape sphere;

	//
	// Each point light is rendered as a sphere.
	//

	glusFileLoadText("../Example31/shader/point_light.vert.glsl", &vertexSource);
	glusFileLoadText("../Example31/shader/point_light.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_programPointLight, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	//

	g_projectionMatrixPointLightLocation = glGetUniformLocation(g_programPointLight.program, "u_projectionMatrix");
	g_viewMatrixPointLightLocation = glGetUniformLocation(g_programPointLight.program, "u_viewMatrix");
	g_modelMatrixPointLightLocation = glGetUniformLocation(g_programPointLight.program, "u_modelMatrix");
	g_positionMatrixPointLightLocation = glGetUniformLocation(g_programPointLight.program, "u_positionMatrix");
	g_biasMatrixPointLightLocation = glGetUniformLocation(g_programPointLight.program, "u_biasMatrix");
	g_radiusPointLightLocation = glGetUniformLocation(g_programPointLight.program, "u_lightRadius");

	g_diffusePointLightLocation = glGetUniformLocation(g_programPointLight.program, "u_diffuse");
	g_specularPointLightLocation = glGetUniformLocation(g_programPointLight.program, "u_specular");
	g_positionPointLightLocation = glGetUniformLocation(g_programPointLight.program, "u_position");
	g_normalPointLightLocation = glGetUniformLocation(g_programPointLight.program, "u_normal");

	g_vertexPointLightLocation = glGetAttribLocation(g_programPointLight.program, "a_vertex");

	// Use a helper function to create a cube.
	glusShapeCreateSpheref(&sphere, POINT_LIGHT_RADIUS, 32);

	g_numberIndicesPointLight = sphere.numberIndices;

	glGenBuffers(1, &g_verticesPointLightVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_verticesPointLightVBO);

	// Transfer the vertices from CPU to GPU.
	glBufferData(GL_ARRAY_BUFFER, sphere.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)sphere.vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &g_indicesPointLightVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesPointLightVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.numberIndices * sizeof(GLuint), (GLuint*)sphere.indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glusShapeDestroyf(&sphere);

	glGenVertexArrays(1, &g_vaoPointLight);
	glBindVertexArray(g_vaoPointLight);

	glBindBuffer(GL_ARRAY_BUFFER, g_verticesPointLightVBO);
	glVertexAttribPointer(g_vertexPointLightLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_vertexPointLightLocation);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesPointLightVBO);

	glBindVertexArray(0);

	//
	// The point lights are moving forth and back between the columns and rows.
	// Here, a random position and moving direction of the point light is generated.
	//

	glusRandomSetSeed(13);

	// Note: If more than 16 lights are used, make sure to store them in another matrix or buffer.
	for (GLint i = 0; i < POINT_LIGHT_COUNT; i++)
	{
		g_positionMatrix[i] = glusRandomUniformf(0.0f, (float)POINT_LIGHT_COUNT - 2.0f);

		g_directionMatrix[i] = rand() % 2 == 0 ? 1.0f : -1.0f;
	}

	//
	//
	//

	glusFileLoadText("../Example31/shader/deferred_shading.vert.glsl", &vertexSource);
	glusFileLoadText("../Example31/shader/deferred_shading.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_programDeferredShading, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	//

	g_projectionMatrixLocation = glGetUniformLocation(g_programDeferredShading.program, "u_projectionMatrix");
	g_viewMatrixLocation = glGetUniformLocation(g_programDeferredShading.program, "u_viewMatrix");
	g_modelMatrixLocation = glGetUniformLocation(g_programDeferredShading.program, "u_modelMatrix");
	g_normalMatrixLocation = glGetUniformLocation(g_programDeferredShading.program, "u_normalMatrix");

	g_material.diffuseColorLocation = glGetUniformLocation(g_programDeferredShading.program, "u_material.diffuseColor");
	g_material.specularColorLocation = glGetUniformLocation(g_programDeferredShading.program, "u_material.specularColor");
	g_material.specularExponentLocation = glGetUniformLocation(g_programDeferredShading.program, "u_material.specularExponent");
	g_material.diffuseTextureLocation = glGetUniformLocation(g_programDeferredShading.program, "u_material.diffuseTexture");

	g_useTextureLocation = glGetUniformLocation(g_programDeferredShading.program, "u_useTexture");

	g_vertexLocation = glGetAttribLocation(g_programDeferredShading.program, "a_vertex");
	g_normalLocation = glGetAttribLocation(g_programDeferredShading.program, "a_normal");
	g_texCoordLocation = glGetAttribLocation(g_programDeferredShading.program, "a_texCoord");

	//
	// Use a helper function to load the wavefront object file.
	//

	glusWavefrontLoad("ChessPawn.obj", &g_wavefront);

	glGenBuffers(1, &g_wavefront.verticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.verticesVBO);
	glBufferData(GL_ARRAY_BUFFER, g_wavefront.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)g_wavefront.vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &g_wavefront.normalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.normalsVBO);
	glBufferData(GL_ARRAY_BUFFER, g_wavefront.numberVertices * 3 * sizeof(GLfloat), (GLfloat*)g_wavefront.normals, GL_STATIC_DRAW);

	glGenBuffers(1, &g_wavefront.texCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.texCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, g_wavefront.numberVertices * 2 * sizeof(GLfloat), (GLfloat*)g_wavefront.texCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//
	// Set up indices and the VAOs for each group
	//

	glUseProgram(g_programDeferredShading.program);

	groupWalker = g_wavefront.groups;
	while (groupWalker)
	{
		glGenBuffers(1, &groupWalker->group.indicesVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.indicesVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.numberIndices * sizeof(GLuint), (GLuint*)groupWalker->group.indices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//

		glGenVertexArrays(1, &groupWalker->group.vao);
		glBindVertexArray(groupWalker->group.vao);

		glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.verticesVBO);
		glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(g_vertexLocation);

		glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.normalsVBO);
		glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(g_normalLocation);

		glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.texCoordsVBO);
		glVertexAttribPointer(g_texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(g_texCoordLocation);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.indicesVBO);

		glBindVertexArray(0);

		groupWalker = groupWalker->next;
	}

	//
	// Load the textures, if there are available
	//

	materialWalker = g_wavefront.materials;
	while (materialWalker)
	{
		if (materialWalker->material.diffuseTextureFilename[0] != '\0')
		{
			// Load the image.
			glusImageLoadTga(materialWalker->material.diffuseTextureFilename, &image);

			// Generate and bind a texture.
			glGenTextures(1, &materialWalker->material.diffuseTextureName);
			glBindTexture(GL_TEXTURE_2D, materialWalker->material.diffuseTextureName);

			// Transfer the image data from the CPU to the GPU.
			glTexImage2D(GL_TEXTURE_2D, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);

			// Setting the texture parameters.
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glGenerateMipmap(GL_TEXTURE_2D);

			glBindTexture(GL_TEXTURE_2D, 0);
		}

		materialWalker = materialWalker->next;
	}

	//
	// Setting up the deferred shading geometry buffer.
	//

	glGenTextures(1, &g_dsDiffuseTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_dsDiffuseTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	//

	glGenTextures(1, &g_dsSpecularTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_dsSpecularTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	//

	glGenTextures(1, &g_dsPositionTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_dsPositionTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	//

	glGenTextures(1, &g_dsNormalTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_dsNormalTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGB, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	//

	glGenTextures(1, &g_dsDepthTexture);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_dsDepthTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);

	//

	glGenFramebuffers(1, &g_dsFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, g_dsFBO);

	// Attach the color buffer ...
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_dsDiffuseTexture, 0);

	// Attach the normal buffer ...
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_dsSpecularTexture, 0);

	// Attach the color buffer ...
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, g_dsPositionTexture, 0);

	// Attach the normal buffer ...
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, g_dsNormalTexture, 0);

	// ... and the depth buffer,
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_dsDepthTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("GL_FRAMEBUFFER_COMPLETE error 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));

		return GLUS_FALSE;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//
	//
	//

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);

	// The color buffer is accumulated for each light, so use this blend function when doing the lighting pass.
	glBlendFunc(GL_ONE, GL_ONE);

	return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
	glViewport(0, 0, width, height);

	glusMatrix4x4Perspectivef(g_projectionMatrix, 40.0f, (GLfloat)width / (GLfloat)height, 1.0f, 100.0f);
}

GLUSboolean update(GLUSfloat time)
{
	static GLfloat biasMatrix[] = { 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f };

	static GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };

	static GLfloat angle = 0.0f;

	GLfloat viewMatrix[16];
	GLfloat modelMatrix[16];
	GLfloat normalMatrix[9];

	GLUSgroupList* groupWalker;

	//
	// Do the deferred shading by rendering into the geometry buffers.
	//

	glBindFramebuffer(GL_FRAMEBUFFER, g_dsFBO);
	glDrawBuffers(4, drawBuffers);

	reshape(TEXTURE_WIDTH, TEXTURE_HEIGHT);

	glUseProgram(g_programDeferredShading.program);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//

	// Just pass the projection matrix. The final matrix is calculated in the shader.
	glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, g_projectionMatrix);

	// Orbit camera around models
	glusMatrix4x4LookAtf(viewMatrix, sinf(angle) * 10.0f, 4.0f, cosf(angle) * 10.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	glUniformMatrix4fv(g_viewMatrixLocation, 1, GL_FALSE, viewMatrix);

	//

	glusMatrix4x4Identityf(modelMatrix);

	glusMatrix4x4Translatef(modelMatrix, -(float)OBJECTS_COLUMNS / 2.0f + 0.5f, 0.0f, (float)OBJECTS_COLUMNS / 2.0f - 0.5f);

	// Scale the model up
	glusMatrix4x4Scalef(modelMatrix, 10.0f, 10.0f, 10.0f);

	// Uniform scale, so extracting is sufficient
	glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelMatrix);

	glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, modelMatrix);
	glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

	//

	glActiveTexture(GL_TEXTURE0);

	groupWalker = g_wavefront.groups;
	while (groupWalker)
	{
		// Set up material values.
		glUniform4fv(g_material.diffuseColorLocation, 1, groupWalker->group.material->diffuse);
		glUniform4fv(g_material.specularColorLocation, 1, groupWalker->group.material->specular);
		glUniform1f(g_material.specularExponentLocation, groupWalker->group.material->shininess);

		// Enable only texturing, if the material has a texture
		if (groupWalker->group.material->diffuseTextureName)
		{
			glUniform1i(g_useTextureLocation, 1);
			glUniform1i(g_material.diffuseTextureLocation, 0);
			glBindTexture(GL_TEXTURE_2D, groupWalker->group.material->diffuseTextureName);
		}
		else
		{
			glUniform1i(g_useTextureLocation, 0);
			glUniform1i(g_material.diffuseTextureLocation, 0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glBindVertexArray(groupWalker->group.vao);

		glDrawElementsInstanced(GL_TRIANGLES, groupWalker->group.numberIndices, GL_UNSIGNED_INT, 0, OBJECTS_COLUMNS * OBJECTS_COLUMNS);

		groupWalker = groupWalker->next;
	}

	//
	// Render now to display framebuffer.
	//

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	reshape(TEXTURE_WIDTH, TEXTURE_HEIGHT);

	glUseProgram(g_programPointLight.program);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniformMatrix4fv(g_projectionMatrixPointLightLocation, 1, GL_FALSE, g_projectionMatrix);

	glUniformMatrix4fv(g_viewMatrixPointLightLocation, 1, GL_FALSE, viewMatrix);

	//

	glusMatrix4x4Identityf(modelMatrix);

	glusMatrix4x4Translatef(modelMatrix, -7.0f, 0.0f, 7.0f);

	glUniformMatrix4fv(g_modelMatrixPointLightLocation, 1, GL_FALSE, modelMatrix);

	//

	// Update position matrix of the point lights. Each matrix filed is used to calculate the final position.
	for (GLint i = 0; i < POINT_LIGHT_COUNT; i++)
	{
		g_positionMatrix[i] += g_directionMatrix[i] * time * 5.0f;

		if (g_positionMatrix[i] > (float)POINT_LIGHT_COUNT - 2.0f)
		{
			g_positionMatrix[i] = ((float)POINT_LIGHT_COUNT - 2.0f) - (g_positionMatrix[i] - ((float)POINT_LIGHT_COUNT - 2.0f));
			g_directionMatrix[i] *= -1.0f;
		}
		else if (g_positionMatrix[i] < 0.0f)
		{
			g_positionMatrix[i] = -g_positionMatrix[i];
			g_directionMatrix[i] *= -1.0f;
		}
	}

	glUniformMatrix4fv(g_positionMatrixPointLightLocation, 1, GL_FALSE, g_positionMatrix);

	glUniformMatrix4fv(g_biasMatrixPointLightLocation, 1, GL_FALSE, biasMatrix);

	glUniform1f(g_radiusPointLightLocation, POINT_LIGHT_RADIUS);

	// Blend, as color is accumulated for each point light.
	glEnable(GL_BLEND);
	// No depth test, as the complete sphere of the point light should be processed.
	glDisable(GL_DEPTH_TEST);

	// Enable all the geometry buffer textures.

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_dsDiffuseTexture);
	glUniform1i(g_diffusePointLightLocation, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_dsSpecularTexture);
	glUniform1i(g_specularPointLightLocation, 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_dsPositionTexture);
	glUniform1i(g_positionPointLightLocation, 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_dsNormalTexture);
	glUniform1i(g_normalPointLightLocation, 3);

	glBindVertexArray(g_vaoPointLight);

	glDrawElementsInstanced(GL_TRIANGLES, g_numberIndicesPointLight, GL_UNSIGNED_INT, 0, POINT_LIGHT_COUNT);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);

	//

	// 20 seconds for one turn
	angle += 2.0f * GLUS_PI * time / 20.0f;

	return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
	GLUSgroupList* groupWalker;
	GLUSmaterialList* materialWalker;

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (g_verticesPointLightVBO)
	{
		glDeleteBuffers(1, &g_verticesPointLightVBO);

		g_verticesPointLightVBO = 0;
	}

	if (g_wavefront.verticesVBO)
	{
		glDeleteBuffers(1, &g_wavefront.verticesVBO);

		g_wavefront.verticesVBO = 0;
	}

	if (g_wavefront.normalsVBO)
	{
		glDeleteBuffers(1, &g_wavefront.normalsVBO);

		g_wavefront.normalsVBO = 0;
	}

	if (g_wavefront.texCoordsVBO)
	{
		glDeleteBuffers(1, &g_wavefront.texCoordsVBO);

		g_wavefront.texCoordsVBO = 0;
	}

	glBindVertexArray(0);

	if (g_vaoPointLight)
	{
		glDeleteVertexArrays(1, &g_vaoPointLight);

		g_vaoPointLight = 0;
	}

	groupWalker = g_wavefront.groups;
	while (groupWalker)
	{
		if (groupWalker->group.indicesVBO)
		{
			glDeleteBuffers(1, &groupWalker->group.indicesVBO);

			groupWalker->group.indicesVBO = 0;
		}

		if (groupWalker->group.vao)
		{
			glDeleteVertexArrays(1, &groupWalker->group.vao);

			groupWalker->group.vao = 0;
		}

		groupWalker = groupWalker->next;
	}

	materialWalker = g_wavefront.materials;
	while (materialWalker)
	{
		if (materialWalker->material.diffuseTextureName)
		{
			glDeleteTextures(1, &materialWalker->material.diffuseTextureName);

			materialWalker->material.diffuseTextureName = 0;
		}

		materialWalker = materialWalker->next;
	}

	glUseProgram(0);

	glusProgramDestroy(&g_programDeferredShading);

	glusProgramDestroy(&g_programPointLight);

	glusWavefrontDestroy(&g_wavefront);

	//

	//

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_dsDiffuseTexture)
	{
		glDeleteTextures(1, &g_dsDiffuseTexture);

		g_dsDiffuseTexture = 0;
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_dsSpecularTexture)
	{
		glDeleteTextures(1, &g_dsSpecularTexture);

		g_dsSpecularTexture = 0;
	}

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_dsPositionTexture)
	{
		glDeleteTextures(1, &g_dsPositionTexture);

		g_dsPositionTexture = 0;
	}

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_dsNormalTexture)
	{
		glDeleteTextures(1, &g_dsNormalTexture);

		g_dsNormalTexture = 0;
	}

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_dsDepthTexture)
	{
		glDeleteTextures(1, &g_dsDepthTexture);

		g_dsDepthTexture = 0;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (g_dsFBO)
	{
		glDeleteFramebuffers(1, &g_dsFBO);

		g_dsFBO = 0;
	}
}

int main(int argc, char* argv[])
{
	EGLint eglConfigAttributes[] = {
	        EGL_RED_SIZE, 8,
	        EGL_GREEN_SIZE, 8,
	        EGL_BLUE_SIZE, 8,
	        EGL_DEPTH_SIZE, 24,
	        EGL_STENCIL_SIZE, 0,
	        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
	        EGL_NONE
	};

    EGLint eglContextAttributes[] = {
    		EGL_CONTEXT_MAJOR_VERSION, 4,
    		EGL_CONTEXT_MINOR_VERSION, 1,
    		EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
    		EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
    		EGL_NONE
    };

    glusWindowSetInitFunc(init);

    glusWindowSetReshapeFunc(reshape);

    glusWindowSetUpdateFunc(update);

    glusWindowSetTerminateFunc(terminate);

    // Again, makes programming for this example easier.
    if (!glusWindowCreate("GLUS Example Window", TEXTURE_WIDTH, TEXTURE_HEIGHT, GLUS_FALSE, GLUS_TRUE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusWindowRun();

    return 0;
}
