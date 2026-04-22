/**
 * OpenGL 4 - Example 46
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 *
 * Demonstrates real-time global illumination via Voxel Cone Tracing (VCT).
 *
 * The scene is first voxelised into a 256^3 RGBA16F radiance volume (mip-mapped).
 * Each frame the Sponza atrium is rendered with a cone-tracing fragment shader
 * that traces hemisphere cones for indirect diffuse, a specular cone and an
 * ambient-occlusion estimate, all sourced from the voxel texture.
 *
 * Reference:
 * Cyril Crassin, Fabrice Neyret, Miguel Sainz, Simon Green, Elmar Eisemann,
 * "Interactive Indirect Illumination Using Voxel Cone Tracing",
 * Pacific Graphics 2011.
 * https://research.nvidia.com/sites/default/files/pubs/2011-09_Interactive-Indirect-Illumination/GIVoxels-pg2011-authors.pdf
 *
 * Camera controls:
 *   W / S        - move forward / backward
 *   A / D        - strafe left / right
 *   Cursor left  - rotate left
 *   Cursor right - rotate right
 *   Cursor up    - look up
 *   Cursor down  - look down
 *   Space        - toggle sphere orbit on/off
 */

#include <math.h>
#include <stdio.h>

#include "GL/glus.h"

#define WINDOW_WIDTH    1024
#define WINDOW_HEIGHT   768

// Resolution of the 3-D voxel grid (must be a power of two).
#define VCT_GRID_SIZE   256

// Number of mip levels = log2(VCT_GRID_SIZE) + 1.
#define VCT_MIPLEVELS   9

// World space is normalised so the scene fits inside [-1, 1]^3.
#define VCT_WORLD_SIZE  2.0f

// Sponza bounding box: X [-192.09, 179.99], Y [-12.64, 142.94], Z [-118.28, 110.54]
// Scale = 2 / max_range = 2 / 372.08 ~= 0.005375
// Translate = -center = (6.05, -65.15, 3.87) applied before scale.
#define SPONZA_SCALE    0.005375f
#define SPONZA_TX       6.05f
#define SPONZA_TY      -65.15f
#define SPONZA_TZ       3.87f

// Attribute locations shared by both shader programs.
#define LOCATION_VERTEX   0
#define LOCATION_NORMAL   1
#define LOCATION_TEXCOORD 2

// Texture/image unit bindings.
#define BINDING_VOXEL_GRID   0
#define BINDING_DIFFUSE_TEX  1

// -----------------------------------------------------------------------
// Emissive sphere tuning
// -----------------------------------------------------------------------

// Visual radius of the emissive sphere in normalised world space [-1,1]^3.
// Smaller = tighter point-like source; larger = more diffuse emitter.
#define SPHERE_RADIUS        0.02f

// Emissive colour of the sphere (warm orange).
// Hue and saturation control the tint cast onto nearby surfaces via GI.
#define SPHERE_COLOR_R       1.0f
#define SPHERE_COLOR_G       0.8f
#define SPHERE_COLOR_B       0.2f

// Radius of the circular orbit in the XZ plane (world space).
// Larger = sphere sweeps a wider area, illuminating more of the scene.
#define SPHERE_ORBIT_RADIUS  0.243f

// Fixed Y height of the orbit centre. Keep above the floor (~-0.40).
// Lower = more floor/column illumination; higher = lights walls/ceiling.
#define SPHERE_ORBIT_Y      -0.26f

// Seconds for one complete revolution. Smaller = faster.
#define SPHERE_ORBIT_PERIOD  7.5f

//

// Pass 1: voxelisation (vert + geom + frag, writes to image3D).
static GLUSprogram g_voxelizeProgram;

static GLint g_voxelize_modelMatrixLoc;
static GLint g_voxelize_lightPosLoc;
static GLint g_voxelize_lightColorLoc;
static GLint g_voxelize_diffuseColorLoc;
static GLint g_voxelize_hasDiffuseTextureLoc;
static GLint g_voxelize_voxelGridSizeLoc;
static GLint g_voxelize_halfPixelSizeLoc;
static GLint g_voxelize_isEmissiveLoc;

//

// Pass 2: VCT rendering (vert + frag, samples sampler3D).
static GLUSprogram g_vctProgram;

static GLint g_vct_modelMatrixLoc;
static GLint g_vct_mvpMatrixLoc;
static GLint g_vct_cameraPosLoc;
static GLint g_vct_lightPosLoc;
static GLint g_vct_lightColorLoc;
static GLint g_vct_diffuseColorLoc;
static GLint g_vct_specularColorLoc;
static GLint g_vct_shininessLoc;
static GLint g_vct_hasDiffuseTextureLoc;
static GLint g_vct_voxelGridWorldSizeLoc;
static GLint g_vct_voxelDimensionsLoc;

//

// Scene data.

//

static GLUSwavefront g_wavefront;

static GLuint g_voxelGrid;

// Moving emissive sphere.
static GLUSshape g_sphere;
static GLuint    g_sphereVerticesVBO  = 0;
static GLuint    g_sphereNormalsVBO   = 0;
static GLuint    g_sphereTexCoordsVBO = 0;
static GLuint    g_sphereIndicesVBO   = 0;
static GLuint    g_sphereVAO          = 0;

// Accumulated time used to animate the sphere orbit.
static GLfloat  g_totalTime    = 0.0f;
// When GLUS_TRUE the sphere orbit is frozen (toggled with Space).
static GLboolean g_spherePaused = GLUS_FALSE;

// Model matrix (uniform scale + translate).
static GLfloat g_modelMatrix[16];

//

// Camera state.
// Eye position in normalised world space,
// standing on the ground floor of the Sponza corridor, looking along +X.
//
static GLfloat g_eye[3] = { 0.0f, -0.30f, 0.0f };

// Horizontal yaw in degrees.  0 = looking along -Z; 90 = looking along +X.
static GLfloat g_yaw = 90.0f;

// Vertical pitch in degrees.  Positive = looking up.  Clamped to [-89, 89].
static GLfloat g_pitch = 0.0f;

// Key states for continuous camera movement.
static GLboolean g_moveForward  = GLUS_FALSE;
static GLboolean g_moveBackward = GLUS_FALSE;
static GLboolean g_strafeLeft   = GLUS_FALSE;
static GLboolean g_strafeRight  = GLUS_FALSE;
static GLboolean g_turnLeft     = GLUS_FALSE;
static GLboolean g_turnRight    = GLUS_FALSE;
static GLboolean g_turnUp       = GLUS_FALSE;
static GLboolean g_turnDown     = GLUS_FALSE;

//

static GLint g_windowWidth  = WINDOW_WIDTH;
static GLint g_windowHeight = WINDOW_HEIGHT;

GLUSvoid key(const GLUSboolean pressed, const GLUSint k)
{
	if (k == 'w') g_moveForward  = pressed;
	if (k == 's') g_moveBackward = pressed;
	if (k == 'a') g_strafeLeft   = pressed;
	if (k == 'd') g_strafeRight  = pressed;

	// Cursor left/right rotate the camera. GLFW_KEY_LEFT=263, GLFW_KEY_RIGHT=262.
	if (k == 263) g_turnLeft  = pressed;
	if (k == 262) g_turnRight = pressed;

	// Cursor up/down tilt the camera. GLFW_KEY_UP=265, GLFW_KEY_DOWN=264.
	if (k == 265) g_turnUp   = pressed;
	if (k == 264) g_turnDown = pressed;

	// Space (key 32) toggles the sphere orbit on/off.
	if (k == 32 && pressed) g_spherePaused = !g_spherePaused;
}

//
// Init.
//

GLUSboolean init(GLUSvoid)
{
	GLUStextfile vertexSource;
	GLUStextfile geometrySource;
	GLUStextfile fragmentSource;
	GLUStgaimage image;

	GLUSgroupList*    groupWalker;
	GLUSmaterialList* materialWalker;

	//
	// Build voxelisation program (vert + geom + frag).
	//

	glusFileLoadText("../Example46/shader/voxelize.vert.glsl", &vertexSource);
	glusFileLoadText("../Example46/shader/voxelize.geom.glsl", &geometrySource);
	glusFileLoadText("../Example46/shader/voxelize.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_voxelizeProgram,
	                           (const GLUSchar**) &vertexSource.text,
	                           0, 0,
	                           (const GLUSchar**) &geometrySource.text,
	                           (const GLUSchar**) &fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&geometrySource);
	glusFileDestroyText(&fragmentSource);

	glUseProgram(g_voxelizeProgram.program);

	g_voxelize_modelMatrixLoc        = glGetUniformLocation(g_voxelizeProgram.program, "u_modelMatrix");
	g_voxelize_lightPosLoc           = glGetUniformLocation(g_voxelizeProgram.program, "u_lightPos");
	g_voxelize_lightColorLoc         = glGetUniformLocation(g_voxelizeProgram.program, "u_lightColor");
	g_voxelize_diffuseColorLoc       = glGetUniformLocation(g_voxelizeProgram.program, "u_diffuseColor");
	g_voxelize_hasDiffuseTextureLoc  = glGetUniformLocation(g_voxelizeProgram.program, "u_hasDiffuseTexture");
	g_voxelize_voxelGridSizeLoc      = glGetUniformLocation(g_voxelizeProgram.program, "u_voxelGridSize");
	g_voxelize_halfPixelSizeLoc      = glGetUniformLocation(g_voxelizeProgram.program, "u_halfPixelSize");

	// Static uniforms set once.
	glUniform3f(g_voxelize_lightPosLoc,   0.0f,  0.38f, 0.0f);
	glUniform3f(g_voxelize_lightColorLoc, 0.0f,  0.0f,  0.0f);
	glUniform1i(g_voxelize_voxelGridSizeLoc, VCT_GRID_SIZE);
	glUniform2f(g_voxelize_halfPixelSizeLoc, 1.0f / (GLfloat)VCT_GRID_SIZE, 1.0f / (GLfloat)VCT_GRID_SIZE);
	g_voxelize_isEmissiveLoc = glGetUniformLocation(g_voxelizeProgram.program, "u_isEmissive");
	glUniform1i(g_voxelize_isEmissiveLoc, 0);

	glUseProgram(0);

	//
	// Build VCT rendering program (vert + frag).
	//

	glusFileLoadText("../Example46/shader/vct.vert.glsl", &vertexSource);
	glusFileLoadText("../Example46/shader/vct.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_vctProgram,
	                           (const GLUSchar**) &vertexSource.text,
	                           0, 0, 0,
	                           (const GLUSchar**) &fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	glUseProgram(g_vctProgram.program);

	g_vct_modelMatrixLoc        = glGetUniformLocation(g_vctProgram.program, "u_modelMatrix");
	g_vct_mvpMatrixLoc          = glGetUniformLocation(g_vctProgram.program, "u_mvpMatrix");
	g_vct_cameraPosLoc          = glGetUniformLocation(g_vctProgram.program, "u_cameraPos");
	g_vct_lightPosLoc           = glGetUniformLocation(g_vctProgram.program, "u_lightPos");
	g_vct_lightColorLoc         = glGetUniformLocation(g_vctProgram.program, "u_lightColor");
	g_vct_diffuseColorLoc       = glGetUniformLocation(g_vctProgram.program, "u_diffuseColor");
	g_vct_specularColorLoc      = glGetUniformLocation(g_vctProgram.program, "u_specularColor");
	g_vct_shininessLoc          = glGetUniformLocation(g_vctProgram.program, "u_shininess");
	g_vct_hasDiffuseTextureLoc  = glGetUniformLocation(g_vctProgram.program, "u_hasDiffuseTexture");
	g_vct_voxelGridWorldSizeLoc = glGetUniformLocation(g_vctProgram.program, "u_voxelGridWorldSize");
	g_vct_voxelDimensionsLoc    = glGetUniformLocation(g_vctProgram.program, "u_voxelDimensions");

	// Static uniforms set once.
	glUniform3f(g_vct_lightPosLoc,          0.0f,  0.38f, 0.0f);
	glUniform3f(g_vct_lightColorLoc,        0.0f,  0.0f,  0.0f);
	glUniform1f(g_vct_voxelGridWorldSizeLoc, VCT_WORLD_SIZE);
	glUniform1i(g_vct_voxelDimensionsLoc,    VCT_GRID_SIZE);

	glUseProgram(0);

	//
	// Load the Sponza wavefront model.
	//

	if (!glusWavefrontLoad("sponza.obj", &g_wavefront))
	{
		printf("Could not load sponza.obj\n");
		return GLUS_FALSE;
	}

	// Vertices VBO (4 floats per vertex).
	glGenBuffers(1, &g_wavefront.verticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.verticesVBO);
	glBufferData(GL_ARRAY_BUFFER,
	             g_wavefront.numberVertices * 4 * sizeof(GLfloat),
	             (GLfloat*) g_wavefront.vertices,
	             GL_STATIC_DRAW);

	// Normals VBO (3 floats per vertex).
	glGenBuffers(1, &g_wavefront.normalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.normalsVBO);
	glBufferData(GL_ARRAY_BUFFER,
	             g_wavefront.numberVertices * 3 * sizeof(GLfloat),
	             (GLfloat*) g_wavefront.normals,
	             GL_STATIC_DRAW);

	// Texture coordinates VBO (2 floats per vertex).
	glGenBuffers(1, &g_wavefront.texCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.texCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER,
	             g_wavefront.numberVertices * 2 * sizeof(GLfloat),
	             (GLfloat*) g_wavefront.texCoords,
	             GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//
	// Create per-group index buffers and VAOs.
	// Both programs use the same attribute locations so a single VAO per group suffices.
	//

	glUseProgram(g_vctProgram.program);

	groupWalker = g_wavefront.groups;
	while (groupWalker)
	{
		glGenBuffers(1, &groupWalker->group.indicesVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.indicesVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		             groupWalker->group.numberIndices * sizeof(GLuint),
		             (GLuint*) groupWalker->group.indices,
		             GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glGenVertexArrays(1, &groupWalker->group.vao);
		glBindVertexArray(groupWalker->group.vao);

		glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.verticesVBO);
		glVertexAttribPointer(LOCATION_VERTEX, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(LOCATION_VERTEX);

		glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.normalsVBO);
		glVertexAttribPointer(LOCATION_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(LOCATION_NORMAL);

		glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.texCoordsVBO);
		glVertexAttribPointer(LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(LOCATION_TEXCOORD);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.indicesVBO);

		glBindVertexArray(0);

		groupWalker = groupWalker->next;
	}

	glUseProgram(0);

	//
	// Load per-material diffuse textures.
	//

	glActiveTexture(GL_TEXTURE0 + BINDING_DIFFUSE_TEX);

	materialWalker = g_wavefront.materials;
	while (materialWalker)
	{
		if (materialWalker->material.diffuseTextureFilename[0] != '\0')
		{
			if (!glusImageLoadTga(materialWalker->material.diffuseTextureFilename, &image))
			{
				// Non-fatal: some materials have no texture.
				printf("Warning: could not load texture %s\n",
				       materialWalker->material.diffuseTextureFilename);
				materialWalker = materialWalker->next;
				continue;
			}

			glGenTextures(1, &materialWalker->material.diffuseTextureName);
			glBindTexture(GL_TEXTURE_2D, materialWalker->material.diffuseTextureName);

			glTexImage2D(GL_TEXTURE_2D, 0,
			             image.format, image.width, image.height, 0,
			             image.format, GL_UNSIGNED_BYTE, image.data);

			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			// Anisotropic filtering reduces texture seams and shimmer on
			// surfaces viewed at steep angles (walls, columns, floor).
			// Core since OpenGL 4.6 — no extension check required.
			{
				GLfloat maxAniso = 1.0f;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAniso);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
			}

			glBindTexture(GL_TEXTURE_2D, 0);

			glusImageDestroyTga(&image);
		}

		materialWalker = materialWalker->next;
	}

	glActiveTexture(GL_TEXTURE0);

	//
	// Create the RGBA16F 3-D voxel radiance texture.
	//

	glGenTextures(1, &g_voxelGrid);
	glBindTexture(GL_TEXTURE_3D, g_voxelGrid);

	glTexStorage3D(GL_TEXTURE_3D, VCT_MIPLEVELS, GL_RGBA16F,
	               VCT_GRID_SIZE, VCT_GRID_SIZE, VCT_GRID_SIZE);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// CLAMP_TO_BORDER with a zero border prevents edge-texel radiance from
	// bleeding into coarse mip levels when cone traces step near the grid boundary.
	{
		static const GLfloat zeroBorder[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, zeroBorder);
	}

	glBindTexture(GL_TEXTURE_3D, 0);

	// Model matrix: translate to centre the scene, then scale to fit into [-1, 1]^3.
	// GLUS operations post-multiply: M = Scale * Translate, so M*v = Scale*(v + translate).
	glusMatrix4x4Identityf(g_modelMatrix);
	glusMatrix4x4Scalef(g_modelMatrix, SPONZA_SCALE, SPONZA_SCALE, SPONZA_SCALE);
	glusMatrix4x4Translatef(g_modelMatrix, SPONZA_TX, SPONZA_TY, SPONZA_TZ);

	//
	// Create sphere geometry (radius 0.05, 24 slices) for the moving emissive orb.
	// The sphere lives directly in normalised world space [-1,1]^3, so its model
	// matrix is a pure translation updated every frame.
	//

	glusShapeCreateSpheref(&g_sphere, SPHERE_RADIUS, 24);

	glGenBuffers(1, &g_sphereVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_sphereVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, g_sphere.numberVertices * 4 * sizeof(GLfloat),
	             g_sphere.vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &g_sphereNormalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_sphereNormalsVBO);
	glBufferData(GL_ARRAY_BUFFER, g_sphere.numberVertices * 3 * sizeof(GLfloat),
	             g_sphere.normals, GL_STATIC_DRAW);

	glGenBuffers(1, &g_sphereTexCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_sphereTexCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, g_sphere.numberVertices * 2 * sizeof(GLfloat),
	             g_sphere.texCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &g_sphereIndicesVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_sphereIndicesVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_sphere.numberIndices * sizeof(GLuint),
	             (GLuint*) g_sphere.indices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glGenVertexArrays(1, &g_sphereVAO);
	glBindVertexArray(g_sphereVAO);

	glBindBuffer(GL_ARRAY_BUFFER, g_sphereVerticesVBO);
	glVertexAttribPointer(LOCATION_VERTEX,   4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(LOCATION_VERTEX);

	glBindBuffer(GL_ARRAY_BUFFER, g_sphereNormalsVBO);
	glVertexAttribPointer(LOCATION_NORMAL,   3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(LOCATION_NORMAL);

	glBindBuffer(GL_ARRAY_BUFFER, g_sphereTexCoordsVBO);
	glVertexAttribPointer(LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(LOCATION_TEXCOORD);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_sphereIndicesVBO);

	glBindVertexArray(0);

	//
	// Global OpenGL state.
	//

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return GLUS_TRUE;
}

/* =========================================================================
 * Reshape
 * ====================================================================== */

GLUSvoid reshape(GLUSint width, GLUSint height)
{
	g_windowWidth  = width;
	g_windowHeight = height;
}

//
// Update (called every frame).
//

GLUSboolean update(GLUSfloat time)
{
	static const GLfloat clearValue[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	GLUSgroupList* groupWalker;

	GLfloat viewMatrix[16];
	GLfloat projectionMatrix[16];
	GLfloat mvpMatrix[16];
	GLfloat vpMatrix[16];
	GLfloat sphereModelMatrix[16];
	GLfloat sphereMvpMatrix[16];

	GLfloat moveSpeed;
	GLfloat turnSpeed;
	GLfloat yawRad;
	GLfloat pitchRad;
	GLfloat fwdX;
	GLfloat fwdZ;
	GLfloat rightX;
	GLfloat rightZ;

	if (!g_spherePaused) g_totalTime += time;

	//
	// Camera update.
	//

	moveSpeed = 0.5f * time;
	turnSpeed = 60.0f * time;

	if (g_turnLeft)  g_yaw -= turnSpeed;
	if (g_turnRight) g_yaw += turnSpeed;

	if (g_turnUp)   g_pitch += turnSpeed;
	if (g_turnDown) g_pitch -= turnSpeed;

	if (g_pitch >  89.0f) g_pitch =  89.0f;
	if (g_pitch < -89.0f) g_pitch = -89.0f;

	yawRad   = g_yaw   * GLUS_PI / 180.0f;
	pitchRad = g_pitch * GLUS_PI / 180.0f;
	fwdX   =  sinf(yawRad) * cosf(pitchRad);
	fwdZ   = -cosf(yawRad) * cosf(pitchRad);
	rightX =  cosf(yawRad);
	rightZ =  sinf(yawRad);

	if (g_moveForward)  { g_eye[0] += fwdX   * moveSpeed; g_eye[2] += fwdZ   * moveSpeed; }
	if (g_moveBackward) { g_eye[0] -= fwdX   * moveSpeed; g_eye[2] -= fwdZ   * moveSpeed; }
	if (g_strafeLeft)   { g_eye[0] -= rightX * moveSpeed; g_eye[2] -= rightZ * moveSpeed; }
	if (g_strafeRight)  { g_eye[0] += rightX * moveSpeed; g_eye[2] += rightZ * moveSpeed; }

	glusMatrix4x4LookAtf(viewMatrix,
	                     g_eye[0],        g_eye[1],                      g_eye[2],
	                     g_eye[0] + fwdX, g_eye[1] + sinf(pitchRad),     g_eye[2] + fwdZ,
	                     0.0f, 1.0f, 0.0f);

	glusMatrix4x4Perspectivef(projectionMatrix,
	                          60.0f,
	                          (GLfloat) g_windowWidth / (GLfloat) g_windowHeight,
	                          0.001f, 10.0f);

	glusMatrix4x4Multiplyf(vpMatrix, projectionMatrix, viewMatrix);
	glusMatrix4x4Multiplyf(mvpMatrix, vpMatrix, g_modelMatrix);

	// Sphere orbits a fixed world-space centre near the Sponza floor,
	// independent of camera position or orientation.
	{
		GLfloat angle = g_totalTime * 2.0f * GLUS_PI / SPHERE_ORBIT_PERIOD;

		glusMatrix4x4Identityf(sphereModelMatrix);
		glusMatrix4x4Translatef(sphereModelMatrix,
		                        SPHERE_ORBIT_RADIUS * cosf(angle),
		                        SPHERE_ORBIT_Y,
		                        SPHERE_ORBIT_RADIUS * sinf(angle));
	}

	//
	// Voxelisation pass (runs every frame so the moving sphere updates the grid).
	//

	// Ensure any previous frame's imageStore and texture-fetch operations on the
	// voxel grid are complete before we overwrite it with glClearTexImage.
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_UPDATE_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

	// Clear the base mip level of the voxel grid before voxelising.
	glBindTexture(GL_TEXTURE_3D, g_voxelGrid);
	glClearTexImage(g_voxelGrid, 0, GL_RGBA, GL_FLOAT, clearValue);
	glBindTexture(GL_TEXTURE_3D, 0);

	// State for voxelisation: no colour output, no depth test, no culling.
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glViewport(0, 0, VCT_GRID_SIZE, VCT_GRID_SIZE);

	glUseProgram(g_voxelizeProgram.program);

	// Voxelise Sponza with normal Lambertian lighting.
	glUniform1i(g_voxelize_isEmissiveLoc, 0);
	glUniformMatrix4fv(g_voxelize_modelMatrixLoc, 1, GL_FALSE, g_modelMatrix);

	// Bind the voxel grid as a write-only image at binding point 0.
	glBindImageTexture(BINDING_VOXEL_GRID, g_voxelGrid, 0, GL_TRUE, 0,
	                   GL_WRITE_ONLY, GL_RGBA16F);

	glActiveTexture(GL_TEXTURE0 + BINDING_DIFFUSE_TEX);

	groupWalker = g_wavefront.groups;
	while (groupWalker)
	{
		if (groupWalker->group.material)
		{
			glUniform4fv(g_voxelize_diffuseColorLoc, 1,
			             groupWalker->group.material->diffuse);

			if (groupWalker->group.material->diffuseTextureName)
			{
				glBindTexture(GL_TEXTURE_2D,
				              groupWalker->group.material->diffuseTextureName);
				glUniform1i(g_voxelize_hasDiffuseTextureLoc, 1);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, 0);
				glUniform1i(g_voxelize_hasDiffuseTextureLoc, 0);
			}
		}
		else
		{
			glUniform4f(g_voxelize_diffuseColorLoc, 0.8f, 0.8f, 0.8f, 1.0f);
			glBindTexture(GL_TEXTURE_2D, 0);
			glUniform1i(g_voxelize_hasDiffuseTextureLoc, 0);
		}

		glBindVertexArray(groupWalker->group.vao);
		glDrawElements(GL_TRIANGLES,
		               groupWalker->group.numberIndices,
		               GL_UNSIGNED_INT, 0);

		groupWalker = groupWalker->next;
	}

	// Voxelise sphere as emissive: its colour is stored directly as radiance so
	// VCT cone traces from neighbouring surfaces pick it up as indirect light.
	glUniformMatrix4fv(g_voxelize_modelMatrixLoc, 1, GL_FALSE, sphereModelMatrix);
	glUniform4f(g_voxelize_diffuseColorLoc, SPHERE_COLOR_R, SPHERE_COLOR_G, SPHERE_COLOR_B, 1.0f);
	glUniform1i(g_voxelize_hasDiffuseTextureLoc, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glUniform1i(g_voxelize_isEmissiveLoc, 1);

	glBindVertexArray(g_sphereVAO);
	glDrawElements(GL_TRIANGLES, g_sphere.numberIndices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);

	glBindImageTexture(BINDING_VOXEL_GRID, 0, 0, GL_TRUE, 0,
	                   GL_WRITE_ONLY, GL_RGBA16F);

	// Ensure all image writes are visible before mip generation.
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
	                GL_TEXTURE_FETCH_BARRIER_BIT);

	// Generate mip chain for cone tracing.
	glBindTexture(GL_TEXTURE_3D, g_voxelGrid);
	glGenerateMipmap(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D, 0);

	// Restore rendering state.
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glViewport(0, 0, g_windowWidth, g_windowHeight);

	//
	// VCT rendering pass.
	//

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(g_vctProgram.program);

	glUniformMatrix4fv(g_vct_modelMatrixLoc, 1, GL_FALSE, g_modelMatrix);
	glUniformMatrix4fv(g_vct_mvpMatrixLoc,   1, GL_FALSE, mvpMatrix);
	glUniform3fv(g_vct_cameraPosLoc, 1, g_eye);

	// Bind the voxel grid as a trilinear-mipmapped sampler at unit 0.
	glActiveTexture(GL_TEXTURE0 + BINDING_VOXEL_GRID);
	glBindTexture(GL_TEXTURE_3D, g_voxelGrid);

	glActiveTexture(GL_TEXTURE0 + BINDING_DIFFUSE_TEX);

	groupWalker = g_wavefront.groups;
	while (groupWalker)
	{
		if (groupWalker->group.material)
		{
			glUniform4fv(g_vct_diffuseColorLoc,  1, groupWalker->group.material->diffuse);
			glUniform4fv(g_vct_specularColorLoc, 1, groupWalker->group.material->specular);
			glUniform1f(g_vct_shininessLoc,         groupWalker->group.material->shininess);

			if (groupWalker->group.material->diffuseTextureName)
			{
				glBindTexture(GL_TEXTURE_2D,
				              groupWalker->group.material->diffuseTextureName);
				glUniform1i(g_vct_hasDiffuseTextureLoc, 1);
			}
			else
			{
				glBindTexture(GL_TEXTURE_2D, 0);
				glUniform1i(g_vct_hasDiffuseTextureLoc, 0);
			}
		}
		else
		{
			glUniform4f(g_vct_diffuseColorLoc,  0.8f, 0.8f, 0.8f, 1.0f);
			glUniform4f(g_vct_specularColorLoc, 0.0f, 0.0f, 0.0f, 1.0f);
			glUniform1f(g_vct_shininessLoc, 10.0f);
			glBindTexture(GL_TEXTURE_2D, 0);
			glUniform1i(g_vct_hasDiffuseTextureLoc, 0);
		}

		glBindVertexArray(groupWalker->group.vao);
		glDrawElements(GL_TRIANGLES,
		               groupWalker->group.numberIndices,
		               GL_UNSIGNED_INT, 0);

		groupWalker = groupWalker->next;
	}

	glBindVertexArray(0);

	// Render the emissive sphere with the VCT shader.
	glusMatrix4x4Multiplyf(sphereMvpMatrix, vpMatrix, sphereModelMatrix);
	glUniformMatrix4fv(g_vct_modelMatrixLoc, 1, GL_FALSE, sphereModelMatrix);
	glUniformMatrix4fv(g_vct_mvpMatrixLoc,   1, GL_FALSE, sphereMvpMatrix);
	glUniform4f(g_vct_diffuseColorLoc,  SPHERE_COLOR_R, SPHERE_COLOR_G, SPHERE_COLOR_B, 1.0f);
	glUniform4f(g_vct_specularColorLoc, 1.0f, 0.9f, 0.5f, 1.0f);
	glUniform1f(g_vct_shininessLoc, 128.0f);
	glUniform1i(g_vct_hasDiffuseTextureLoc, 0);
	glActiveTexture(GL_TEXTURE0 + BINDING_DIFFUSE_TEX);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(g_sphereVAO);
	glDrawElements(GL_TRIANGLES, g_sphere.numberIndices, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0 + BINDING_VOXEL_GRID);
	glBindTexture(GL_TEXTURE_3D, 0);
	glActiveTexture(GL_TEXTURE0);

	return GLUS_TRUE;
}

//
// Terminate.
//

GLUSvoid terminate(GLUSvoid)
{
	GLUSgroupList*    groupWalker;
	GLUSmaterialList* materialWalker;

	// Voxel grid texture.
	glBindTexture(GL_TEXTURE_3D, 0);

	if (g_voxelGrid)
	{
		glDeleteTextures(1, &g_voxelGrid);

		g_voxelGrid = 0;
	}

	//

	glBindBuffer(GL_ARRAY_BUFFER, 0);

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

	// Per-group VAOs and index buffers.
	glBindVertexArray(0);

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

	// Per-material textures.
	glBindTexture(GL_TEXTURE_2D, 0);

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

	//

	glUseProgram(0);

	// Sphere GPU resources.
	if (g_sphereVAO)          { glDeleteVertexArrays(1, &g_sphereVAO);          g_sphereVAO          = 0; }
	if (g_sphereIndicesVBO)   { glDeleteBuffers(1, &g_sphereIndicesVBO);         g_sphereIndicesVBO   = 0; }
	if (g_sphereTexCoordsVBO) { glDeleteBuffers(1, &g_sphereTexCoordsVBO);       g_sphereTexCoordsVBO = 0; }
	if (g_sphereNormalsVBO)   { glDeleteBuffers(1, &g_sphereNormalsVBO);         g_sphereNormalsVBO   = 0; }
	if (g_sphereVerticesVBO)  { glDeleteBuffers(1, &g_sphereVerticesVBO);        g_sphereVerticesVBO  = 0; }
	glusShapeDestroyf(&g_sphere);

	glusProgramDestroy(&g_voxelizeProgram);
	glusProgramDestroy(&g_vctProgram);

	glusWavefrontDestroy(&g_wavefront);
}

/* =========================================================================
 * Entry point
 * ====================================================================== */

int main(int argc, char* argv[])
{
	EGLint eglConfigAttributes[] = {
	        EGL_RED_SIZE,   8,
	        EGL_GREEN_SIZE, 8,
	        EGL_BLUE_SIZE,  8,
	        EGL_DEPTH_SIZE, 24,
	        EGL_STENCIL_SIZE, 0,
	        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
	        EGL_NONE
	};

	EGLint eglContextAttributes[] = {
	        EGL_CONTEXT_MAJOR_VERSION, 4,
	        EGL_CONTEXT_MINOR_VERSION, 6,
	        EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
	        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
	        EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
	        EGL_NONE
	};

	glusWindowSetInitFunc(init);
	glusWindowSetReshapeFunc(reshape);
	glusWindowSetUpdateFunc(update);
	glusWindowSetTerminateFunc(terminate);
	glusWindowSetKeyFunc(key);

	if (!glusWindowCreate("GLUS Example Window", WINDOW_WIDTH, WINDOW_HEIGHT,
	                      GLUS_FALSE, GLUS_TRUE,
	                      eglConfigAttributes, eglContextAttributes, 0))
	{
		printf("Could not create window!\n");
		return -1;
	}

	glusWindowRun();

	return 0;
}
