/**
 * OpenGL 3 - Example 43
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

/**
 * Properties of the light.
 */
struct LightProperties
{
	GLfloat direction[3];
	GLfloat ambientColor[4];
	GLfloat diffuseColor[4];
	GLfloat specularColor[4];
};

/**
 * Properties of the material, basically all the color factors without the emissive color component.
 */
struct MaterialProperties
{
	GLfloat ambientColor[4];
	GLfloat diffuseColor[4];
	GLfloat specularColor[4];
	GLfloat specularExponent;
};

/**
 * Locations for the light properties.
 */
struct LightLocations
{
	GLint directionLocation;
	GLint ambientColorLocation;
	GLint diffuseColorLocation;
	GLint specularColorLocation;
};

/**
 * Locations for the material properties. With a diffuse texture.
 */
struct MaterialLocations
{
	GLint ambientColorLocation;
	GLint diffuseColorLocation;
	GLint specularColorLocation;
	GLint specularExponentLocation;

	GLint diffuseTextureLocation;
};

static GLfloat g_viewMatrix[16];

static GLUSprogram g_program;

/**
 * The location of the projection matrix.
 */
static GLint g_projectionMatrixLocation;

/**
 * The location of the model view matrix.
 */
static GLint g_modelViewMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

static GLint g_texCoordLocation;

static GLint g_useTextureLocation;

/**
 * The locations for the light properties.
 */
static struct LightLocations g_light;

/**
 * The locations for the material properties.
 */
static struct MaterialLocations g_material;

/**
 * This structure contains the loaded wavefront object file.
 */
static GLUSscene g_scene;

GLUSboolean init(GLUSvoid)
{
	// This is a white light.
	struct LightProperties light = { { 1.0f, 1.0f, 1.0f }, { 0.3f, 0.3f, 0.3f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };

	GLUStextfile vertexSource;
	GLUStextfile fragmentSource;

	GLUStgaimage image;

	GLUSobjectList* objectWalker;
	GLUSgroupList* groupWalker;
	GLUSmaterialList* materialWalker;

	glusFileLoadText("../Example43/shader/phong_textured.vert.glsl", &vertexSource);
	glusFileLoadText("../Example43/shader/phong_textured.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_program, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	//

	g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
	g_modelViewMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewMatrix");
	g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");

	g_light.directionLocation = glGetUniformLocation(g_program.program, "u_light.direction");
	g_light.ambientColorLocation = glGetUniformLocation(g_program.program, "u_light.ambientColor");
	g_light.diffuseColorLocation = glGetUniformLocation(g_program.program, "u_light.diffuseColor");
	g_light.specularColorLocation = glGetUniformLocation(g_program.program, "u_light.specularColor");

	g_material.ambientColorLocation = glGetUniformLocation(g_program.program, "u_material.ambientColor");
	g_material.diffuseColorLocation = glGetUniformLocation(g_program.program, "u_material.diffuseColor");
	g_material.specularColorLocation = glGetUniformLocation(g_program.program, "u_material.specularColor");
	g_material.specularExponentLocation = glGetUniformLocation(g_program.program, "u_material.specularExponent");
	g_material.diffuseTextureLocation = glGetUniformLocation(g_program.program, "u_material.diffuseTexture");

	g_useTextureLocation = glGetUniformLocation(g_program.program, "u_useTexture");

	g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
	g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");
	g_texCoordLocation = glGetAttribLocation(g_program.program, "a_texCoord");

	//
	// Use a helper function to load the wavefront object file.
	//

	glusWavefrontLoadScene("three_objects.obj", &g_scene);

	objectWalker = g_scene.objectList;
	while (objectWalker)
	{
		glGenBuffers(1, &objectWalker->object.verticesVBO);
		glBindBuffer(GL_ARRAY_BUFFER, objectWalker->object.verticesVBO);
		glBufferData(GL_ARRAY_BUFFER, objectWalker->object.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)objectWalker->object.vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &objectWalker->object.normalsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, objectWalker->object.normalsVBO);
		glBufferData(GL_ARRAY_BUFFER, objectWalker->object.numberVertices * 3 * sizeof(GLfloat), (GLfloat*)objectWalker->object.normals, GL_STATIC_DRAW);

		glGenBuffers(1, &objectWalker->object.texCoordsVBO);
		glBindBuffer(GL_ARRAY_BUFFER, objectWalker->object.texCoordsVBO);
		glBufferData(GL_ARRAY_BUFFER, objectWalker->object.numberVertices * 2 * sizeof(GLfloat), (GLfloat*)objectWalker->object.texCoords, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//
		// Set up indices and the VAOs for each group
		//

		glUseProgram(g_program.program);

		groupWalker = objectWalker->object.groups;
		while (groupWalker)
		{
			glGenBuffers(1, &groupWalker->group.indicesVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.indicesVBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.numberIndices * sizeof(GLuint), (GLuint*)groupWalker->group.indices, GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

			//

			glGenVertexArrays(1, &groupWalker->group.vao);
			glBindVertexArray(groupWalker->group.vao);

			glBindBuffer(GL_ARRAY_BUFFER, objectWalker->object.verticesVBO);
			glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(g_vertexLocation);

			glBindBuffer(GL_ARRAY_BUFFER, objectWalker->object.normalsVBO);
			glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(g_normalLocation);

			glBindBuffer(GL_ARRAY_BUFFER, objectWalker->object.texCoordsVBO);
			glVertexAttribPointer(g_texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(g_texCoordLocation);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.indicesVBO);

			glBindVertexArray(0);

			groupWalker = groupWalker->next;
		}

		//
		// Load the textures, if there are available
		//

		materialWalker = objectWalker->object.materials;
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

		objectWalker = objectWalker->next;
	}

	//

	glusMatrix4x4LookAtf(g_viewMatrix, 0.0f, 0.75f, 10.0f, 0.0f, 0.75f, 0.0f, 0.0f, 1.0f, 0.0f);

	//

	glusVector3Normalizef(light.direction);

	// Transform light to camera space, as it is currently in world space.
	glusMatrix4x4MultiplyVector3f(light.direction, g_viewMatrix, light.direction);

	// Set up light
	glUniform3fv(g_light.directionLocation, 1, light.direction);
	glUniform4fv(g_light.ambientColorLocation, 1, light.ambientColor);
	glUniform4fv(g_light.diffuseColorLocation, 1, light.diffuseColor);
	glUniform4fv(g_light.specularColorLocation, 1, light.specularColor);

	//

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);

	return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
	GLfloat projectionMatrix[16];

	glViewport(0, 0, width, height);

	glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat)width / (GLfloat)height, 1.0f, 100.0f);

	// Just pass the projection matrix. The final matrix is calculated in the shader.
	glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);
}

GLUSboolean update(GLUSfloat time)
{
	static GLfloat angle = 0.0f;

	GLfloat modelViewMatrix[16];
	GLfloat normalMatrix[9];

	GLUSobjectList* objectWalker;
	GLUSgroupList* groupWalker;

	glusMatrix4x4Identityf(modelViewMatrix);

	glusMatrix4x4RotateRyf(modelViewMatrix, angle);
	// Scale the model up
	glusMatrix4x4Scalef(modelViewMatrix, 1.0f, 1.0f, 1.0f);

	glusMatrix4x4Multiplyf(modelViewMatrix, g_viewMatrix, modelViewMatrix);

	// Uniform scale, so extracting is sufficient
	glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

	glUniformMatrix4fv(g_modelViewMatrixLocation, 1, GL_FALSE, modelViewMatrix);
	glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	objectWalker = g_scene.objectList;

	while (objectWalker)
	{
		groupWalker = objectWalker->object.groups;
		while (groupWalker)
		{
			// Set up material values.
			glUniform4fv(g_material.ambientColorLocation, 1, groupWalker->group.material->ambient);
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

			glDrawElements(GL_TRIANGLES, groupWalker->group.numberIndices, GL_UNSIGNED_INT, 0);

			groupWalker = groupWalker->next;
		}

		objectWalker = objectWalker->next;
	}

	angle += 30.0f * time;

	return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
	GLUSobjectList* objectWalker;
	GLUSgroupList* groupWalker;
	GLUSmaterialList* materialWalker;

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	objectWalker = g_scene.objectList;

	while (objectWalker)
	{
		if (objectWalker->object.verticesVBO)
		{
			glDeleteBuffers(1, &objectWalker->object.verticesVBO);

			objectWalker->object.verticesVBO = 0;
		}

		if (objectWalker->object.normalsVBO)
		{
			glDeleteBuffers(1, &objectWalker->object.normalsVBO);

			objectWalker->object.normalsVBO = 0;
		}

		if (objectWalker->object.texCoordsVBO)
		{
			glDeleteBuffers(1, &objectWalker->object.texCoordsVBO);

			objectWalker->object.texCoordsVBO = 0;
		}

		groupWalker = objectWalker->object.groups;
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

		materialWalker = objectWalker->object.materials;
		while (materialWalker)
		{
			if (materialWalker->material.diffuseTextureName)
			{
				glDeleteTextures(1, &materialWalker->material.diffuseTextureName);

				materialWalker->material.diffuseTextureName = 0;
			}

			materialWalker = materialWalker->next;
		}

		objectWalker = objectWalker->next;
	}

	glUseProgram(0);

	glusProgramDestroy(&g_program);

	glusWavefrontDestroyScene(&g_scene);
}

int main(int argc, char* argv[])
{
	EGLint eglConfigAttributes[] = {
	        EGL_RED_SIZE, 8,
	        EGL_GREEN_SIZE, 8,
	        EGL_BLUE_SIZE, 8,
	        EGL_DEPTH_SIZE, 24,
	        EGL_STENCIL_SIZE, 0,
	        EGL_SAMPLE_BUFFERS, 1,
	        EGL_SAMPLES, 8,
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

    if (!glusWindowCreate("GLUS Example Window", 640, 480, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusWindowRun();

    return 0;
}
