/**
 * OpenGL 4 - Example 14
 *
 * Method for Real-Time LOD Terrain Rendering on Modern GPU
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include <math.h>

#include <stdlib.h>

#include "GL/glus.h"

/**
 * gl_Position will be transformed to the buffer
 */

static const char* TRANSFORM_VARYING = "v_vertex";

//
// Map metrics etc.
//

/**
 * One turn / cycle around takes x seconds.
 */
static const GLfloat TURN_DURATION = 20.0f;

/**
 * The circle around radius.
 */
static const GLfloat TURN_RADIUS = 6000.0f;

/**
 * The space when traveling horizontal form one pixel to the next in meters.
 */
static const GLfloat HORIZONTAL_PIXEL_SPACING = 60.0f;

/**
 * The space when traveling all color pixel in meters.
 */
static const GLfloat VERTICAL_PIXEL_RANGE = 10004.0f;

/**
 * The scale to convert the real world meters in virtual world scale.
 */
static const GLfloat METERS_TO_VIRTUAL_WORLD_SCALE = 5.0f;

/**
 * Detail level at the beginning of the map.
 */
static const GLuint MINIMUM_DETAIL_LEVEL = 4;

/**
 * Additional detail level in the first pass. Adjust in GeometryPassOne max_vertices = 4^(firstPassDetailLevel+1)
 */
static const GLuint DETAIL_LEVEL_FIRST_PASS = 2;

/**
 * FOV radius
 */
static const GLfloat FOV_RADIUS = 10000.0f;

/**
 * Number of quadrants when going to the next detail level.
 */
static const GLint QUADRANT_STEP = 2;

/**
 * The height map of the terrain. Width and height does not have to be the same but they have to be power of two plus one.
 */
static const char* HEIGHT_MAP = "grand_canyon_height.tga";

/**
 * The color map of the terrain.
 */
static const char* COLOR_MAP = "grand_canyon_color.tga";

/**
 * The normal map of the terrain. Width and height does not have to be the same but they have to be power of two plus one.
 */
static const char* NORMAL_MAP = "grand_canyon_normal.tga";

/**
 * Flag, if animation is on or off
 */
static GLboolean g_animationOn = GLUS_TRUE;

/**
 * The extend of the map in s coordinate direction. This is basically the width of the texture.
 */
static GLfloat g_sMapExtend;

/**
 * The extend of the map in t coordinate direction. Basically the height.
 */
static GLfloat g_tMapExtend;

/**
 * Number of points in s direction.
 */
static GLuint g_sNumPoints;

/**
 * Number of points in t direction.
 */
static GLuint g_tNumPoints;

//
// Buffer objects etc.
//

static GLuint g_vaoPassOne = 0;

static GLuint g_verticesPassOneVBO = 0;

static GLuint g_indicesPassOneVBO = 0;

static GLuint g_vaoPassTwo = 0;

static GLuint g_verticesPassTwoVBO = 0;

//

static GLUSprogram g_programPassOne;

static GLint g_halfDetailStepPassOneLocation;

static GLint g_detailLevelPassOneLocation;

static GLint g_positionTextureSpacePassOneLocation;

static GLint g_fovRadiusPassOneLocation;

static GLint g_leftNormalTextureSpacePassOneLocation;

static GLint g_rightNormalTextureSpacePassOneLocation;

static GLint g_backNormalTextureSpacePassOneLocation;

static GLUSprogram g_shaderProgramPassTwo;

//

static GLint g_tmvpPassTwoLocation;

static GLint g_maxTessellationLevelPassTwoLocation;

static GLint g_quadrantStepPassTwoLocation;

static GLint g_positionTextureSpacePassTwoLocation;

static GLint g_heightMapTexturePassTwoLocation;

static GLint g_colorMapTexturePassTwoLocation;

static GLint g_normalMapTexturePassTwoLocation;

static GLint g_lightDirectionPassTwoLocation;

//

static GLfloat g_projectionMatrix[16];

static GLfloat g_viewMatrix[16];

static GLfloat g_textureToWorldMatrix[16];

static GLfloat g_worldToTextureMatrix[16];

static GLfloat g_worldToTextureNormalMatrix[16];

//

/**
 * Variable to query how much primitives were written.
 */
static GLuint g_transformFeedbackQuery;

//

static GLuint g_normalMapTexture;

static GLuint g_heightMapTexture;

static GLuint g_colorMapTexture;

//

typedef struct _ViewData
{
    GLfloat cameraPosition[4];
    GLfloat cameraDirection[3];
    GLfloat cameraUp[3];
    GLfloat fov;
} ViewData;

static ViewData g_topView;

static ViewData g_personView;

static ViewData* g_activeView;

//

static GLfloat g_width;

static GLfloat g_height;

GLUSboolean init(GLUSvoid)
{
    GLfloat textureToWorldNormalMatrix[16];

    // The maximum detail level which is 2^s = sMapExtend.
    GLuint sMaxDetailLevel;

    // The maximum detail level which is 2^t = tMapExtend.
    GLuint tMaxDetailLevel;

    // The overall maximum detail level from s and t.
    GLuint overallMaxDetailLevel;

    // Step for s and t direction.
    GLfloat detailStep;

    GLuint s, t;

    GLUStgaimage image;

    GLfloat* map = 0;

    GLuint* indices = 0;

    GLUStextfile vertexSource;
    GLUStextfile controlSource;
    GLUStextfile evaluationSource;
    GLUStextfile geometrySource;
    GLUStextfile fragmentSource;

    GLfloat lightDirection[3] = { 1.0f, 1.0f, 1.0f };

    glusVector3Normalizef(lightDirection);

    g_topView.cameraPosition[0] = 0.0f;
    g_topView.cameraPosition[1] = 30000.0f * METERS_TO_VIRTUAL_WORLD_SCALE;
    g_topView.cameraPosition[2] = 0.0f;
    g_topView.cameraPosition[3] = 1.0;
    g_topView.cameraDirection[0] = 0.0f;
    g_topView.cameraDirection[1] = -1.0f;
    g_topView.cameraDirection[2] = 0.0f;
    g_topView.cameraUp[0] = 0.0f;
    g_topView.cameraUp[1] = 0.0f;
    g_topView.cameraUp[2] = -1.0f;
    g_topView.fov = 40.0f;

    g_personView.cameraPosition[0] = 0.0f;
    g_personView.cameraPosition[1] = 4700.0f * METERS_TO_VIRTUAL_WORLD_SCALE;
    g_personView.cameraPosition[2] = 0.0f;
    g_personView.cameraPosition[3] = 1.0;
    g_personView.cameraDirection[0] = 0.0f;
    g_personView.cameraDirection[1] = 0.0f;
    g_personView.cameraDirection[2] = -1.0f;
    g_personView.cameraUp[0] = 0.0f;
    g_personView.cameraUp[1] = 1.0f;
    g_personView.cameraUp[2] = 0.0f;
    g_personView.fov = 60.0f;

    g_activeView = &g_personView;

    if (!glusImageLoadTga(NORMAL_MAP, &image))
    {
        printf("Could not load normal picture '%s'!\n", NORMAL_MAP);

        return GLUS_FALSE;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &g_normalMapTexture);

    glBindTexture(GL_TEXTURE_RECTANGLE, g_normalMapTexture);

    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);

    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glusImageDestroyTga(&image);

    if (!glusImageLoadTga(HEIGHT_MAP, &image))
    {
        printf("Could not load height picture '%s'!\n", HEIGHT_MAP);

        return GLUS_FALSE;
    }

    glGenTextures(1, &g_heightMapTexture);

    glBindTexture(GL_TEXTURE_RECTANGLE, g_heightMapTexture);

    glTexImage2D(GL_TEXTURE_RECTANGLE, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);

    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    g_sMapExtend = (GLfloat) image.width;

    g_tMapExtend = (GLfloat) image.height;

    glusImageDestroyTga(&image);

    // Calculate the detail level for the s and ...
    sMaxDetailLevel = (GLuint) floorf(logf(g_sMapExtend) / logf(2.0f));

    // ... t extend
    tMaxDetailLevel = (GLuint) floorf(logf(g_tMapExtend) / logf(2.0f));

    overallMaxDetailLevel = glusMathMinf(sMaxDetailLevel, tMaxDetailLevel);

    // Do checking of calculated parameters
    if (MINIMUM_DETAIL_LEVEL > overallMaxDetailLevel)
    {
        printf("Detail level to high %d > %d\n", MINIMUM_DETAIL_LEVEL, overallMaxDetailLevel);

        return GLUS_FALSE;
    }

    if (MINIMUM_DETAIL_LEVEL + DETAIL_LEVEL_FIRST_PASS > overallMaxDetailLevel)
    {
        printf("First pass detail level to high %d > %d\n", MINIMUM_DETAIL_LEVEL + DETAIL_LEVEL_FIRST_PASS, overallMaxDetailLevel);

        return GLUS_FALSE;
    }

    if (powf(2.0f, overallMaxDetailLevel - (MINIMUM_DETAIL_LEVEL + DETAIL_LEVEL_FIRST_PASS)) > 32.0f)
    {
        printf("Tessellation level to high %d > 32\n", (GLint) powf(2.0f, overallMaxDetailLevel - (MINIMUM_DETAIL_LEVEL + DETAIL_LEVEL_FIRST_PASS)));

        return GLUS_FALSE;
    }

    detailStep = powf(2.0f, overallMaxDetailLevel - MINIMUM_DETAIL_LEVEL);

    g_sNumPoints = (GLuint) ceilf(g_sMapExtend / detailStep) - 1;

    g_tNumPoints = (GLuint) ceilf(g_tMapExtend / detailStep) - 1;

    //
    // Generate the flat terrain mesh.
    //

    map = (GLUSfloat*) malloc(g_sNumPoints * g_tNumPoints * 2 * sizeof(GLfloat));

    indices = (GLuint*) malloc(g_sNumPoints * g_tNumPoints * sizeof(GLuint));

    for (t = 0; t < g_tNumPoints; t++)
    {
        for (s = 0; s < g_sNumPoints; s++)
        {
            map[t * g_sNumPoints * 2 + s * 2 + 0] = 0.5f + detailStep / 2.0f + (GLfloat) s * detailStep;
            map[t * g_sNumPoints * 2 + s * 2 + 1] = 0.5f + detailStep / 2.0f + (GLfloat) t * detailStep;

            indices[t * g_sNumPoints + s + 0] = (t + 0) * g_sNumPoints + s + 0;
        }
    }

    //
    // Transferring vertices and indices into GPU
    //

    // Pass one

    glGenBuffers(1, &g_verticesPassOneVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesPassOneVBO);
    glBufferData(GL_ARRAY_BUFFER, g_sNumPoints * g_tNumPoints * 2 * sizeof(GLfloat), map, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesPassOneVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesPassOneVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_sNumPoints * g_tNumPoints * sizeof(GLuint), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Pass two.

    glGenBuffers(1, &g_verticesPassTwoVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesPassTwoVBO);
    // Calculate enough space!
    glBufferData(GL_ARRAY_BUFFER, g_sNumPoints * g_tNumPoints * (GLuint) pow(4, DETAIL_LEVEL_FIRST_PASS + 1) * 2 * sizeof(GLfloat), 0, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //

    free(map);
    map = 0;

    free(indices);
    indices = 0;

    //

    if (!glusImageLoadTga(COLOR_MAP, &image))
    {
        printf("Could not load color picture '%s'!\n", COLOR_MAP);

        return GLUS_FALSE;
    }

    glGenTextures(1, &g_colorMapTexture);

    glBindTexture(GL_TEXTURE_2D, g_colorMapTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glusImageDestroyTga(&image);

    //

    glGenQueries(1, &g_transformFeedbackQuery);

    //
    // Creating the shader program.
    //

    // Pass one.

    glusFileLoadText("../Example14/shader/PassOne.vert.glsl", &vertexSource);
    glusFileLoadText("../Example14/shader/PassOne.geom.glsl", &geometrySource);
    glusFileLoadText("../Example14/shader/PassOne.frag.glsl", &fragmentSource);

    // Compile and ...
    glusProgramCreateFromSource(&g_programPassOne, (const GLUSchar**) &vertexSource.text, 0, 0, (const GLUSchar**) &geometrySource.text, (const GLUSchar**) &fragmentSource.text);

    // ... add the transform variable ...
    glTransformFeedbackVaryings(g_programPassOne.program, 1, (const GLchar**) &TRANSFORM_VARYING, GL_SEPARATE_ATTRIBS);

    // ... and link the program
    if (!glusProgramLink(&g_programPassOne))
    {
        printf("Could not build program one\n");

        return GLUS_FALSE;
    }

    // Destroy the text resource
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&geometrySource);
    glusFileDestroyText(&fragmentSource);

    g_halfDetailStepPassOneLocation = glGetUniformLocation(g_programPassOne.program, "u_halfDetailStep");

    g_detailLevelPassOneLocation = glGetUniformLocation(g_programPassOne.program, "u_detailLevel");

    g_fovRadiusPassOneLocation = glGetUniformLocation(g_programPassOne.program, "u_fovRadius");

    g_positionTextureSpacePassOneLocation = glGetUniformLocation(g_programPassOne.program, "u_positionTextureSpace");
    g_leftNormalTextureSpacePassOneLocation = glGetUniformLocation(g_programPassOne.program, "u_leftNormalTextureSpace");
    g_rightNormalTextureSpacePassOneLocation = glGetUniformLocation(g_programPassOne.program, "u_rightNormalTextureSpace");
    g_backNormalTextureSpacePassOneLocation = glGetUniformLocation(g_programPassOne.program, "u_backNormalTextureSpace");

    // Pass two.

    glusFileLoadText("../Example14/shader/PassTwo.vert.glsl", &vertexSource);
    glusFileLoadText("../Example14/shader/PassTwo.cont.glsl", &controlSource);
    glusFileLoadText("../Example14/shader/PassTwo.eval.glsl", &evaluationSource);
    glusFileLoadText("../Example14/shader/PassTwo.geom.glsl", &geometrySource);
    glusFileLoadText("../Example14/shader/PassTwo.frag.glsl", &fragmentSource);

    if (!glusProgramBuildFromSource(&g_shaderProgramPassTwo, (const GLUSchar**) &vertexSource.text, (const GLUSchar**) &controlSource.text, (const GLUSchar**) &evaluationSource.text, (const GLUSchar**) &geometrySource.text, (const GLUSchar**) &fragmentSource.text))
    {
        printf("Could not build program two\n");

        return GLUS_FALSE;
    }

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&controlSource);
    glusFileDestroyText(&evaluationSource);
    glusFileDestroyText(&geometrySource);
    glusFileDestroyText(&fragmentSource);

    g_maxTessellationLevelPassTwoLocation = glGetUniformLocation(g_shaderProgramPassTwo.program, "u_maxTessellationLevel");

    g_quadrantStepPassTwoLocation = glGetUniformLocation(g_shaderProgramPassTwo.program, "u_quadrantStep");

    g_positionTextureSpacePassTwoLocation = glGetUniformLocation(g_shaderProgramPassTwo.program, "u_positionTextureSpace");

    g_heightMapTexturePassTwoLocation = glGetUniformLocation(g_shaderProgramPassTwo.program, "u_heightMapTexture");
    g_normalMapTexturePassTwoLocation = glGetUniformLocation(g_shaderProgramPassTwo.program, "u_normalMapTexture");

    g_tmvpPassTwoLocation = glGetUniformLocation(g_shaderProgramPassTwo.program, "u_tmvpMatrix");

    g_lightDirectionPassTwoLocation = glGetUniformLocation(g_shaderProgramPassTwo.program, "u_lightDirection");

    g_colorMapTexturePassTwoLocation = glGetUniformLocation(g_shaderProgramPassTwo.program, "u_colorMapTexture");

    //

    // One time matrix calculations to convert between texture and world space

    glusMatrix4x4Identityf(g_textureToWorldMatrix);
    glusMatrix4x4Identityf(textureToWorldNormalMatrix);

    glusMatrix4x4Scalef(g_textureToWorldMatrix, HORIZONTAL_PIXEL_SPACING * METERS_TO_VIRTUAL_WORLD_SCALE, VERTICAL_PIXEL_RANGE * METERS_TO_VIRTUAL_WORLD_SCALE, HORIZONTAL_PIXEL_SPACING * METERS_TO_VIRTUAL_WORLD_SCALE);
    // Skip this scale for the normal matrix

    glusMatrix4x4Scalef(g_textureToWorldMatrix, 1.0f, 1.0f, -1.0f);
    glusMatrix4x4Scalef(textureToWorldNormalMatrix, 1.0f, 1.0f, -1.0f);

    glusMatrix4x4Translatef(g_textureToWorldMatrix, -g_sMapExtend / 2.0f, 0.0f, -g_tMapExtend / 2.0f);
    // No need for the translation matrix in the normal matrix

    glusMatrix4x4Copyf(g_worldToTextureMatrix, g_textureToWorldMatrix, GLUS_FALSE);
    glusMatrix4x4Inversef(g_worldToTextureMatrix);

    glusMatrix4x4Copyf(g_worldToTextureNormalMatrix, textureToWorldNormalMatrix, GLUS_FALSE);
    glusMatrix4x4Inversef(g_worldToTextureNormalMatrix);

    // Pass one

    glUseProgram(g_programPassOne.program);

    glUniform1f(g_halfDetailStepPassOneLocation, detailStep / 2.0f);
    glUniform1ui(g_detailLevelPassOneLocation, DETAIL_LEVEL_FIRST_PASS);
    glUniform1f(g_fovRadiusPassOneLocation, FOV_RADIUS / HORIZONTAL_PIXEL_SPACING * METERS_TO_VIRTUAL_WORLD_SCALE);

    glGenVertexArrays(1, &g_vaoPassOne);
    glBindVertexArray(g_vaoPassOne);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesPassOneVBO);
    // First 0 is the location = 0. See shader source
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    // Enable location = 0
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesPassOneVBO);

   // Pass two

    glUseProgram(g_shaderProgramPassTwo.program);

    glUniform3fv(g_lightDirectionPassTwoLocation, 1, lightDirection);
    glUniform1ui(g_maxTessellationLevelPassTwoLocation, overallMaxDetailLevel - (MINIMUM_DETAIL_LEVEL + DETAIL_LEVEL_FIRST_PASS));
    glUniform1i(g_quadrantStepPassTwoLocation, QUADRANT_STEP);

    glGenVertexArrays(1, &g_vaoPassTwo);
    glBindVertexArray(g_vaoPassTwo);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesPassTwoVBO);
    // First 0 is the location = 0. See shader source
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    // Enable location = 0
    glEnableVertexAttribArray(0);

    //

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(g_heightMapTexturePassTwoLocation, 0);
    glBindTexture(GL_TEXTURE_RECTANGLE, g_heightMapTexture);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(g_colorMapTexturePassTwoLocation, 1);
    glBindTexture(GL_TEXTURE_2D, g_colorMapTexture);

    glActiveTexture(GL_TEXTURE2);
    glUniform1i(g_normalMapTexturePassTwoLocation, 2);
    glBindTexture(GL_TEXTURE_RECTANGLE, g_normalMapTexture);

    //

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

    glPatchParameteri(GL_PATCH_VERTICES, 4);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    g_width = (GLfloat) width;
    g_height = (GLfloat) height;

    glViewport(0, 0, width, height);

    glusMatrix4x4Perspectivef(g_projectionMatrix, g_activeView->fov, (GLfloat) width / (GLfloat) height, 1.0f, 1000000.0f);
}

GLUSvoid key(GLUSboolean pressed, GLUSint key)
{
    static GLboolean filled = GL_TRUE;

    static GLboolean topViewActive = GL_TRUE;

    if (pressed)
    {
        // w for wireframe on / off
        if (key == 'w')
        {
            if (filled)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }

            filled = !filled;
        }

        // a for animation on / off
        if (key == 'a')
        {
            g_animationOn = !g_animationOn;
        }

        // space for changing the view position
        if (key == ' ')
        {
            topViewActive = !topViewActive;

            if (topViewActive)
            {
                g_activeView = &g_personView;
            }
            else
            {
                g_activeView = &g_topView;
            }

            glusMatrix4x4Perspectivef(g_projectionMatrix, g_activeView->fov, g_width / g_height, 1.0f, 1000000.0f);
        }
    }
}

GLUSboolean update(GLUSfloat time)
{
    static GLfloat angle = 0.0f;

    GLuint primitivesWritten;

    // Field of view

    GLfloat rotationMatrix[16];

    GLfloat positionTextureSpace[4];
    GLfloat directionTextureSpace[3];
    GLfloat leftNormalTextureSpace[3];
    GLfloat rightNormalTextureSpace[3];
    GLfloat backNormalTextureSpace[3];

    GLfloat xzPosition2D[4];

    //

    GLfloat tmvpMatrix[16];

    // Animation update

    g_personView.cameraPosition[0] = -cosf(2.0f * GLUS_PI * angle / TURN_DURATION) * TURN_RADIUS * METERS_TO_VIRTUAL_WORLD_SCALE;
    g_personView.cameraPosition[2] = -sinf(2.0f * GLUS_PI * angle / TURN_DURATION) * TURN_RADIUS * METERS_TO_VIRTUAL_WORLD_SCALE;

    g_personView.cameraDirection[0] = sinf(2.0f * GLUS_PI * angle / TURN_DURATION);
    g_personView.cameraDirection[2] = -cosf(2.0f * GLUS_PI * angle / TURN_DURATION);

    if (g_animationOn)
    {
        angle += time;
    }

    glusMatrix4x4LookAtf(g_viewMatrix, g_activeView->cameraPosition[0], g_activeView->cameraPosition[1], g_activeView->cameraPosition[2], g_activeView->cameraPosition[0] + g_activeView->cameraDirection[0], g_activeView->cameraPosition[1] + g_activeView->cameraDirection[1],
            g_activeView->cameraPosition[2] + g_activeView->cameraDirection[2], g_activeView->cameraUp[0], g_activeView->cameraUp[1], g_activeView->cameraUp[2]);

    glusMatrix4x4Identityf(tmvpMatrix);
    glusMatrix4x4Multiplyf(tmvpMatrix, tmvpMatrix, g_projectionMatrix);
    glusMatrix4x4Multiplyf(tmvpMatrix, tmvpMatrix, g_viewMatrix);
    glusMatrix4x4Multiplyf(tmvpMatrix, tmvpMatrix, g_textureToWorldMatrix);

    // Position

    xzPosition2D[0] = g_personView.cameraPosition[0];
    xzPosition2D[1] = 0.0f;
    xzPosition2D[2] = g_personView.cameraPosition[2];
    xzPosition2D[3] = g_personView.cameraPosition[3];

    glusMatrix4x4MultiplyPoint4f(positionTextureSpace, g_worldToTextureMatrix, xzPosition2D);

    // Direction

    glusMatrix4x4MultiplyVector3f(directionTextureSpace, g_worldToTextureMatrix, g_personView.cameraDirection);

    // Left normal of field of view

    glusMatrix4x4Identityf(rotationMatrix);
    glusMatrix4x4RotateRyf(rotationMatrix, g_personView.fov * (g_width / g_height) / 2.0f + 90.0f);
    glusMatrix4x4MultiplyVector3f(leftNormalTextureSpace, rotationMatrix, g_personView.cameraDirection);
    glusMatrix4x4MultiplyVector3f(leftNormalTextureSpace, g_worldToTextureNormalMatrix, leftNormalTextureSpace);

    // Right normal of field of view

    glusMatrix4x4Identityf(rotationMatrix);
    glusMatrix4x4RotateRyf(rotationMatrix, -g_personView.fov * (g_width / g_height) / 2.0f - 90.0f);
    glusMatrix4x4MultiplyVector3f(rightNormalTextureSpace, rotationMatrix, g_personView.cameraDirection);
    glusMatrix4x4MultiplyVector3f(rightNormalTextureSpace, g_worldToTextureNormalMatrix, rightNormalTextureSpace);

    // Back normal of field of view

    glusMatrix4x4Identityf(rotationMatrix);
    glusMatrix4x4RotateRyf(rotationMatrix, 180.0f);
    glusMatrix4x4MultiplyVector3f(backNormalTextureSpace, rotationMatrix, g_personView.cameraDirection);
    glusMatrix4x4MultiplyVector3f(backNormalTextureSpace, g_worldToTextureNormalMatrix, backNormalTextureSpace);

    // OpenGL stuff

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Pass one.

    // Disable any rasterization
    glEnable(GL_RASTERIZER_DISCARD);

    glUseProgram(g_programPassOne.program);

    glUniform4fv(g_positionTextureSpacePassOneLocation, 1, positionTextureSpace);
    glUniform3fv(g_leftNormalTextureSpacePassOneLocation, 1, leftNormalTextureSpace);
    glUniform3fv(g_rightNormalTextureSpacePassOneLocation, 1, rightNormalTextureSpace);
    glUniform3fv(g_backNormalTextureSpacePassOneLocation, 1, backNormalTextureSpace);

    glBindVertexArray(g_vaoPassOne);

    // Bind to vertices used in render pass two. To this buffer is written.
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, g_verticesPassTwoVBO);

    // We need to know, how many primitives are written. So start the query.
    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, g_transformFeedbackQuery);

    // Start the operation ...
    glBeginTransformFeedback(GL_POINTS);

    // ... render the elements ...
    glDrawElements(GL_POINTS, g_sNumPoints * g_tNumPoints, GL_UNSIGNED_INT, 0);

    // ... and stop the operation.
    glEndTransformFeedback();

    // Now, we can also stop the query.
    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

    glDisable(GL_RASTERIZER_DISCARD);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

    glBindVertexArray(0);

    // Pass two

    glUseProgram(g_shaderProgramPassTwo.program);

    glUniformMatrix4fv(g_tmvpPassTwoLocation, 1, GL_FALSE, tmvpMatrix);
    glUniform4fv(g_positionTextureSpacePassTwoLocation, 1, positionTextureSpace);

    glBindVertexArray(g_vaoPassTwo);

    // Now get the number of primitives written in the first render pass.
    glGetQueryObjectuiv(g_transformFeedbackQuery, GL_QUERY_RESULT, &primitivesWritten);

    // No draw the final terrain.
    glDrawArrays(GL_PATCHES, 0, primitivesWritten);

    return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
    // Pass one.

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (g_verticesPassOneVBO)
    {
        glDeleteBuffers(1, &g_verticesPassOneVBO);

        g_verticesPassOneVBO = 0;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    if (g_indicesPassOneVBO)
    {
        glDeleteBuffers(1, &g_indicesPassOneVBO);

        g_indicesPassOneVBO = 0;
    }

    glBindVertexArray(0);

    if (g_vaoPassOne)
    {
        glDeleteVertexArrays(1, &g_vaoPassOne);

        g_vaoPassOne = 0;
    }

    glUseProgram(0);

    glusProgramDestroy(&g_programPassOne);

    // Pass two.

    if (g_verticesPassTwoVBO)
    {
        glDeleteBuffers(1, &g_verticesPassTwoVBO);

        g_verticesPassTwoVBO = 0;
    }

    if (g_vaoPassTwo)
    {
        glDeleteVertexArrays(1, &g_vaoPassTwo);

        g_vaoPassTwo = 0;
    }

    glusProgramDestroy(&g_shaderProgramPassTwo);

    //

    if (g_transformFeedbackQuery)
    {
        glDeleteQueries(1, &g_transformFeedbackQuery);

        g_transformFeedbackQuery = 0;
    }

    //

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);

    if (g_heightMapTexture)
    {
        glDeleteTextures(1, &g_heightMapTexture);

        g_heightMapTexture = 0;
    }

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (g_colorMapTexture)
    {
        glDeleteTextures(1, &g_colorMapTexture);

        g_colorMapTexture = 0;
    }

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);

    if (g_normalMapTexture)
    {
        glDeleteTextures(1, &g_normalMapTexture);

        g_normalMapTexture = 0;
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

    glusWindowSetKeyFunc(key);

    glusWindowSetUpdateFunc(update);

    glusWindowSetTerminateFunc(terminate);

    if (!glusWindowCreate("GLUS Example Window", 640, 480, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    // Print out the keys
    printf("Keys:\n");
    printf("[Space] = Toggle view\n");
    printf("a       = Toggle animation on/off\n");
    printf("w       = Toggle wireframe on/off\n");

    glusWindowRun();

    return 0;
}
