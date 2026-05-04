/**
 * OpenGL 4 - Example 50
 *
 * @author  Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 *
 * glTF 2.0 PBR renderer with Image-Based Lighting and skeletal animation,
 * rendered in Rec.2020 working space and presented through HDR-capable EGL
 * surfaces using McNopper/EGL (https://github.com/McNopper/EGL).
 *
 * Compared to Example49:
 *   1. Working color space is linear Rec.2020 instead of linear Rec.709.
 *      The HDR panorama is converted Rec.709 -> Rec.2020 on the CPU before
 *      IBL pre-filtering, and the PBR fragment shader applies the same
 *      conversion to base-color and emissive texture samples.
 *   2. Window/context is created via McNopper/EGL.  At startup we probe the
 *      EGL extension string and pick the best output color space we find,
 *      preferring BT.2020 (PQ -> HLG -> linear) and falling back through
 *      scRGB, Display P3 and finally sRGB / linear.  The fullscreen
 *      resolve shader applies the matching primaries+EOTF conversion.
 *   3. Camera orbit direction is reversed.
 *
 * Usage:  Example50 [path/to/model.glb]  [path/to/panorama.hdr]
 * Defaults: phoenix/scene.gltf / sunny_rose_garden_4k.hdr next to the binary.
 *
 * IBL pipeline (uses GLUS IBL API):
 *   Pass 1 — Pre-filtered specular cubemap array (GGX importance sampling)
 *   Pass 2 — Diffuse irradiance cubemap
 *   Pass 3 — BRDF split-sum LUT
 *   Pass 4 — Background cubemap (plain panorama remap, no filtering)
 *
 * Animation pipeline (uses GLUS animation API):
 *   All TRS channels from all animations are sampled each frame and composed
 *   into per-node world matrices.  Skinned meshes use joint matrices derived
 *   from world matrices × inverse bind matrices.
 *
 * PBR rendering:
 *   Pass A — Opaque + Alpha-mask primitives (non-skinned: g_pbrProg,
 *             skinned: g_pbrSkinnedProg)
 *   Pass B — Alpha-blend primitives
 *   Resolve — MSAA tone-mapped fullscreen quad
 *
 * Camera: auto-orbits the scene; arrow keys adjust height/radius; +/- zoom.
 */

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <GL/glus.h>

/* GLUS's bundled glus.h defines a small handful of EGL_* enum macros (and an
 * EGLint typedef) for its own use.  Undefine them before pulling in the real
 * McNopper/EGL headers so the macros don't collide with the canonical
 * Khronos definitions (C4005). */
#undef EGL_TRUE
#undef EGL_FALSE
#undef EGL_NONE
#undef EGL_DONT_CARE

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
#  include <windows.h>
#endif

// ---------------------------------------------------------------------
// Diagnostic logging.Writes to stdout *and* to Example50.log next to
// the executable; either OutputDebugString or the log file will catch
// the message even when stdout is being discarded by the parent shell.
// ---------------------------------------------------------------------
static FILE* g_logFile = NULL;
static void log_printf(const char* fmt, ...)
{
	char buf[4096];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	fputs(buf, stdout);
	fflush(stdout);
	if (g_logFile)
	{
		fputs(buf, g_logFile);
		fflush(g_logFile);
	}
#ifdef _WIN32
	OutputDebugStringA(buf);
#endif
}
#define LOGF(...) log_printf(__VA_ARGS__)

// Output colorspace IDs, must match shader/fullscreen.frag.glsl.
enum
{
	OUT_LINEAR            = 0,
	OUT_SRGB              = 1,
	OUT_DISPLAY_P3        = 2,
	OUT_DISPLAY_P3_LINEAR = 3,
	OUT_SCRGB_LINEAR      = 4,
	OUT_SCRGB             = 5,
	OUT_BT2020_LINEAR     = 6,
	OUT_BT2020_HLG        = 7,
	OUT_BT2020_PQ         = 8
};

//
// Constants
//

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

#define SPECULAR_CUBEMAP_SIZE   256
#define DIFFUSE_CUBEMAP_SIZE    128
#define BRDF_LUT_SIZE           512
#define BACKGROUND_CUBEMAP_SIZE 512
#define NUMBER_ROUGHNESS        6
#define MSAA_SAMPLES            4

#define MAX_PRIMITIVES    4096
#define MAX_TEXTURES      1024
#define MAX_IMAGE_CACHE   1024
#define MAX_NODES         1024
#define MAX_NODE_CHILDREN   16
#define MAX_SKINS           16
#define MAX_JOINTS         128
#define MAX_ANIM_CHANNELS 1024

// Camera and rendering constants
#define CAMERA_ORBIT_RADIUS_FACTOR  1.8f
#define CAMERA_ORBIT_RADIUS_MIN     0.5f
#define CAMERA_HEIGHT_OFFSET        0.4f
#define CAMERA_HEIGHT_STEP          0.1f
#define CAMERA_ORBIT_SPEED_STEP     5.0f
#define CAMERA_ZOOM_IN_FACTOR       0.9f
#define CAMERA_ZOOM_OUT_FACTOR      1.1f
#define CAMERA_ZOOM_MIN_FACTOR      0.2f
#define CAMERA_FOV_DEG              45.0f
#define CAMERA_NEAR_FACTOR          0.005f
#define CAMERA_NEAR_MIN             0.001f
#define CAMERA_FAR_RADIUS_FACTOR    3.0f
#define CAMERA_FAR_EXTRA            600.0f
#define SCENE_RADIUS_MIN            0.01f
#define CAMERA_DEG_TO_RAD           (GLUS_PI / 180.0f)

#define ANIM_PATH_TRANSLATION 0
#define ANIM_PATH_ROTATION    1
#define ANIM_PATH_SCALE       2

//
// Data structures
//

typedef struct
{
GLuint  vao;
GLuint  vbo_pos;
GLuint  vbo_nor;
GLuint  vbo_tan;
GLuint  vbo_uv0;
GLuint  vbo_joints;   // JOINTS_0 (float*4); 0 if not skinned
GLuint  vbo_weights;  // WEIGHTS_0 (float*4); 0 if not skinned
GLuint  ibo;
GLsizei vertexCount;
GLsizei indexCount;
GLenum  indexType;

// Material
GLuint baseColorTexture;
GLuint metallicRoughnessTexture;
GLuint normalTexture;
GLuint occlusionTexture;
GLuint emissiveTexture;

GLfloat baseColorFactor[4];
GLfloat metallicFactor;
GLfloat roughnessFactor;
GLfloat emissiveFactor[3];
GLfloat occlusionStrength;
GLfloat alphaCutoff;
GLint   alphaMode;    // 0=OPAQUE, 1=MASK, 2=BLEND
GLint   doubleSided;
GLint   hasNormalMap;

// Transform
GLint   nodeIdx;      // index into g_nodes[]
GLint   skinIdx;      // -1 = not skinned, else index into g_skins[]
GLfloat modelMatrix[16];
GLfloat normalMatrix[9];
} GltfPrimitive;

typedef struct
{
GLfloat translation[3];
GLfloat rotation[4];       // xyzw quaternion
GLfloat scale[3];
GLint   hasMatrix;
GLfloat localMatrix[16];   // used only when hasMatrix == 1
GLfloat worldMatrix[16];   // recomputed every frame
GLint   parentIdx;
GLint   childIndices[MAX_NODE_CHILDREN];
GLint   childCount;
} GltfNode;

typedef struct
{
GLint   jointCount;
GLint   jointNodeIndices[MAX_JOINTS];
GLfloat inverseBindMatrices[MAX_JOINTS * 16];
GLfloat jointMatrices[MAX_JOINTS * 16];  // recomputed every frame
} GltfSkin;

typedef struct
{
GLint    nodeIdx;
GLint    path;          // ANIM_PATH_*
GLint    interpolation; // GLUS_ANIMATION_*
GLint    count;         // number of keyframes
GLfloat* times;         // [count]
GLfloat* values;        // [output->count * components]
} AnimChannel;

typedef struct
{
cgltf_image* image;
GLuint       texture;
} ImageCacheEntry;

//
// Globals
//

// Scene
static GltfPrimitive   g_primitives[MAX_PRIMITIVES];
static GLint           g_numPrimitives   = 0;
static GLuint          g_textures[MAX_TEXTURES];
static GLint           g_numTextures     = 0;
static ImageCacheEntry g_imageCache[MAX_IMAGE_CACHE];
static GLint           g_numCacheEntries = 0;
static GLuint          g_defaultWhiteTexture  = 0;
static GLuint          g_defaultNormalTexture = 0;

// Node hierarchy
static GltfNode g_nodes[MAX_NODES];
static GLint    g_numNodes     = 0;
static GLint    g_rootNodes[MAX_NODES];
static GLint    g_numRootNodes = 0;

// Skins
static GltfSkin g_skins[MAX_SKINS];
static GLint    g_numSkins = 0;

// Animation
static AnimChannel g_animChannels[MAX_ANIM_CHANNELS];
static GLint       g_numAnimChannels = 0;
static GLfloat     g_animTime        = 0.0f;
static GLfloat     g_animDuration    = 0.0f;

// Scene bounds
static GLfloat g_sceneMin[3]    = {  1e30f,  1e30f,  1e30f };
static GLfloat g_sceneMax[3]    = { -1e30f, -1e30f, -1e30f };
static GLfloat g_sceneCenterX   = 0.0f;
static GLfloat g_sceneCenterY   = 0.0f;
static GLfloat g_sceneCenterZ   = 0.0f;
static GLfloat g_sceneRadius    = 1.0f;

// Camera
static GLfloat g_orbitAngle  = 0.0f;
static GLfloat g_orbitRadius = 5.0f;
static GLfloat g_cameraY     = 0.0f;
static GLfloat g_orbitSpeed  = 18.0f;
static GLfloat g_viewProjectionMatrix[16];
static GLfloat g_eye[4];

// Render settings
static GLfloat g_gamma             = 2.2f;
static GLint   g_outputColorspace  = OUT_SRGB;       // chosen at startup
static GLfloat g_referenceWhiteNits = 200.0f;        // scene 1.0 -> 200 nits
static GLfloat g_peakNits           = 1000.0f;       // assumed display peak

// IBL textures
static GLuint g_specularTexture  = 0;
static GLuint g_diffuseTexture   = 0;
static GLuint g_brdfLutTexture   = 0;
static GLuint g_bgCubemapTexture = 0;

// Background sphere
static GLuint  g_bgVAO        = 0;
static GLuint  g_bgVBO        = 0;
static GLuint  g_bgIBO        = 0;
static GLsizei g_bgIndexCount = 0;

// Empty VAO for attribute-less fullscreen draw
static GLuint g_fullscreenVAO = 0;

// MSAA framebuffer
static GLuint g_msaaFBO      = 0;
static GLuint g_msaaColor    = 0;
static GLuint g_msaaDepth    = 0;
static GLint  g_windowWidth  = 0;
static GLint  g_windowHeight = 0;

// Programs
static GLUSprogram g_bgProg;
static GLUSprogram g_fullscreenProg;
static GLUSprogram g_pbrProg;
static GLUSprogram g_pbrSkinnedProg;

// Uniform locations — non-skinned PBR
static GLint g_u_modelMatrix;
static GLint g_u_vpMatrix;
static GLint g_u_normalMatrix;
static GLint g_u_eye_pbr;
static GLint g_u_baseColorFactor;
static GLint g_u_metallicFactor;
static GLint g_u_roughnessFactor;
static GLint g_u_emissiveFactor;
static GLint g_u_occlusionStrength;
static GLint g_u_alphaCutoff;
static GLint g_u_alphaMode;
static GLint g_u_hasNormalMap;

// Uniform locations — skinned PBR
static GLint g_u_vpMatrix_sk;
static GLint g_u_eye_sk;
static GLint g_u_jointMatrices_sk;
static GLint g_u_baseColorFactor_sk;
static GLint g_u_metallicFactor_sk;
static GLint g_u_roughnessFactor_sk;
static GLint g_u_emissiveFactor_sk;
static GLint g_u_occlusionStrength_sk;
static GLint g_u_alphaCutoff_sk;
static GLint g_u_alphaMode_sk;
static GLint g_u_hasNormalMap_sk;

// Uniform locations — background
static GLint g_u_vpMatrix_bg;

// Uniform locations — fullscreen
static GLint g_u_gamma;
static GLint g_u_msaaSamples;
static GLint g_u_outputColorspace;
static GLint g_u_referenceWhiteNits;
static GLint g_u_peakNits;

// Command-line config
static const GLUSchar* g_gltfPath     = NULL;
static const GLUSchar* g_panoramaPath = NULL;

//
// Texture helpers
//

static GLuint createTexture1x1(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
GLuint  tex;
GLubyte data[4];

data[0] = r; data[1] = g; data[2] = b; data[3] = a;

glGenTextures(1, &tex);
glBindTexture(GL_TEXTURE_2D, tex);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
return tex;
}

static void trackTexture(GLuint tex)
{
if (g_numTextures < MAX_TEXTURES)
g_textures[g_numTextures++] = tex;
}

static GLuint loadImageTexture(cgltf_image* image, const GLUSchar* basePath, GLint sRGB)
{
GLint          w, h, comp;
GLubyte*       pixels;
GLuint         tex;
GLenum         internalFmt;
GLint          i;
GLUSchar       path[1024];
const GLubyte* embeddedBuf;

if (!image)
return g_defaultWhiteTexture;

for (i = 0; i < g_numCacheEntries; i++)
if (g_imageCache[i].image == image)
return g_imageCache[i].texture;

pixels      = NULL;
embeddedBuf = NULL;

if (image->buffer_view)
{
embeddedBuf = (const GLubyte*)image->buffer_view->buffer->data
            + image->buffer_view->offset;
pixels = stbi_load_from_memory(embeddedBuf, (int)image->buffer_view->size,
                               &w, &h, &comp, 4);
}
else if (image->uri && strncmp(image->uri, "data:", 5) != 0)
{
if (basePath[0])
snprintf(path, sizeof(path), "%s%s", basePath, image->uri);
else
{
strncpy(path, image->uri, sizeof(path) - 1);
path[sizeof(path) - 1] = '\0';
}
pixels = stbi_load(path, &w, &h, &comp, 4);
}

if (!pixels)
{
printf("Warning: failed to load image '%s'\n",
       image->uri ? image->uri : "(embedded)");
return g_defaultWhiteTexture;
}

glGenTextures(1, &tex);
glBindTexture(GL_TEXTURE_2D, tex);

internalFmt = sRGB ? GL_SRGB8_ALPHA8 : GL_RGBA8;
glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, w, h, 0,
             GL_RGBA, GL_UNSIGNED_BYTE, pixels);
glGenerateMipmap(GL_TEXTURE_2D);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

stbi_image_free(pixels);

trackTexture(tex);

if (g_numCacheEntries < MAX_IMAGE_CACHE)
{
g_imageCache[g_numCacheEntries].image   = image;
g_imageCache[g_numCacheEntries].texture = tex;
g_numCacheEntries++;
}

return tex;
}

static void applyGltfSampler(const cgltf_sampler* sampler)
{
if (!sampler) return;
if (sampler->min_filter)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler->min_filter);
if (sampler->mag_filter)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler->mag_filter);
if (sampler->wrap_s)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler->wrap_s);
if (sampler->wrap_t)
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler->wrap_t);
}

//
// Scene bounds helpers
//

static void expandBoundsWithPoint(const GLfloat p[3])
{
GLint i;
for (i = 0; i < 3; i++)
{
if (p[i] < g_sceneMin[i]) g_sceneMin[i] = p[i];
if (p[i] > g_sceneMax[i]) g_sceneMax[i] = p[i];
}
}

static void expandBoundsAABB(const GLfloat minP[3], const GLfloat maxP[3],
                              const GLfloat modelMatrix[16])
{
GLint cx, cy, cz;
for (cx = 0; cx < 2; cx++)
for (cy = 0; cy < 2; cy++)
for (cz = 0; cz < 2; cz++)
{
GLfloat corner[4];
GLfloat world[4];
GLfloat wp3[3];

corner[0] = cx ? maxP[0] : minP[0];
corner[1] = cy ? maxP[1] : minP[1];
corner[2] = cz ? maxP[2] : minP[2];
corner[3] = 1.0f;

glusMatrix4x4MultiplyPoint4f(world, modelMatrix, corner);

wp3[0] = world[0]; wp3[1] = world[1]; wp3[2] = world[2];
expandBoundsWithPoint(wp3);
}
}

//
// Node hierarchy helpers
//

// Build the local 4x4 matrix from a node's TRS or raw matrix.
static void buildLocalMatrix(const GltfNode* gn, GLfloat result[16])
{
GLfloat T[16], R[16], S[16], RS[16];

if (gn->hasMatrix)
{
memcpy(result, gn->localMatrix, 64);
return;
}

glusMatrix4x4Identityf(T);
T[12] = gn->translation[0];
T[13] = gn->translation[1];
T[14] = gn->translation[2];

glusQuaternionGetMatrix4x4f(R, (GLUSfloat*)gn->rotation);

glusMatrix4x4Identityf(S);
S[0]  = gn->scale[0];
S[5]  = gn->scale[1];
S[10] = gn->scale[2];

glusMatrix4x4Multiplyf(RS, R, S);
glusMatrix4x4Multiplyf(result, T, RS);
}

// Recursively compute world matrices, top-down.
static void computeWorldMatrix(GLint nodeIdx, const GLfloat parentWorld[16])
{
GLfloat local[16];
GLint   ci;

buildLocalMatrix(&g_nodes[nodeIdx], local);
glusMatrix4x4Multiplyf(g_nodes[nodeIdx].worldMatrix, parentWorld, local);

for (ci = 0; ci < g_nodes[nodeIdx].childCount; ci++)
computeWorldMatrix(g_nodes[nodeIdx].childIndices[ci],
                   g_nodes[nodeIdx].worldMatrix);
}

// Copy TRS and parent/child relationships from cgltf into g_nodes[].
static void buildNodeHierarchy(const cgltf_data* data)
{
GLint ni, ci, i;

g_numNodes = (GLint)data->nodes_count;
if (g_numNodes > MAX_NODES)
{
printf("Warning: node count %d exceeds MAX_NODES %d, clamping\n",
       g_numNodes, MAX_NODES);
g_numNodes = MAX_NODES;
}

for (ni = 0; ni < g_numNodes; ni++)
{
const cgltf_node* node = &data->nodes[ni];
GltfNode*         gn   = &g_nodes[ni];

gn->translation[0] = gn->translation[1] = gn->translation[2] = 0.0f;
gn->rotation[0] = gn->rotation[1] = gn->rotation[2] = 0.0f;
gn->rotation[3] = 1.0f;
gn->scale[0] = gn->scale[1] = gn->scale[2] = 1.0f;
gn->hasMatrix  = 0;
gn->parentIdx  = -1;
gn->childCount = 0;
glusMatrix4x4Identityf(gn->worldMatrix);

if (node->has_matrix)
{
gn->hasMatrix = 1;
for (i = 0; i < 16; i++)
gn->localMatrix[i] = (GLfloat)node->matrix[i];
}
else
{
if (node->has_translation)
{
gn->translation[0] = (GLfloat)node->translation[0];
gn->translation[1] = (GLfloat)node->translation[1];
gn->translation[2] = (GLfloat)node->translation[2];
}
if (node->has_rotation)
{
gn->rotation[0] = (GLfloat)node->rotation[0];
gn->rotation[1] = (GLfloat)node->rotation[1];
gn->rotation[2] = (GLfloat)node->rotation[2];
gn->rotation[3] = (GLfloat)node->rotation[3];
}
if (node->has_scale)
{
gn->scale[0] = (GLfloat)node->scale[0];
gn->scale[1] = (GLfloat)node->scale[1];
gn->scale[2] = (GLfloat)node->scale[2];
}
}

gn->childCount = (GLint)node->children_count;
if (gn->childCount > MAX_NODE_CHILDREN)
{
printf("Warning: node %d has %d children, clamping to %d\n",
       ni, gn->childCount, MAX_NODE_CHILDREN);
gn->childCount = MAX_NODE_CHILDREN;
}
for (ci = 0; ci < gn->childCount; ci++)
gn->childIndices[ci] = (GLint)(node->children[ci] - data->nodes);
}

// Set parent indices from child lists.
for (ni = 0; ni < g_numNodes; ni++)
for (ci = 0; ci < g_nodes[ni].childCount; ci++)
{
GLint childIdx = g_nodes[ni].childIndices[ci];
if (childIdx >= 0 && childIdx < g_numNodes)
g_nodes[childIdx].parentIdx = ni;
}
}

// Find scene root nodes.
static void buildRootNodes(const cgltf_data* data)
{
GLint ri, ni;

g_numRootNodes = 0;

if (data->scene)
{
g_numRootNodes = (GLint)data->scene->nodes_count;
for (ri = 0; ri < g_numRootNodes; ri++)
g_rootNodes[ri] = (GLint)(data->scene->nodes[ri] - data->nodes);
}
else
{
for (ni = 0; ni < g_numNodes; ni++)
if (g_nodes[ni].parentIdx == -1 && g_numRootNodes < MAX_NODES)
g_rootNodes[g_numRootNodes++] = ni;
}
}

// Recompute all world matrices from the scene roots.
static void computeAllWorldMatrices(void)
{
GLfloat identity[16];
GLint   ri;

glusMatrix4x4Identityf(identity);
for (ri = 0; ri < g_numRootNodes; ri++)
computeWorldMatrix(g_rootNodes[ri], identity);
}

//
// Skin helpers
//

// Copy joint indices and inverse bind matrices from cgltf.
static void loadSkins(const cgltf_data* data)
{
GLint si, ji;

g_numSkins = (GLint)data->skins_count;
if (g_numSkins > MAX_SKINS)
{
printf("Warning: skin count %d exceeds MAX_SKINS %d, clamping\n",
       g_numSkins, MAX_SKINS);
g_numSkins = MAX_SKINS;
}

for (si = 0; si < g_numSkins; si++)
{
const cgltf_skin* skin = &data->skins[si];
GltfSkin*         gs   = &g_skins[si];

gs->jointCount = (GLint)skin->joints_count;
if (gs->jointCount > MAX_JOINTS)
{
printf("Warning: skin %d has %d joints, clamping to %d\n",
       si, gs->jointCount, MAX_JOINTS);
gs->jointCount = MAX_JOINTS;
}

for (ji = 0; ji < gs->jointCount; ji++)
gs->jointNodeIndices[ji] = (GLint)(skin->joints[ji] - data->nodes);

// Default IBM = identity; overwrite when accessor is present.
for (ji = 0; ji < gs->jointCount; ji++)
glusMatrix4x4Identityf(&gs->inverseBindMatrices[ji * 16]);

if (skin->inverse_bind_matrices)
{
GLint accessorCount = (GLint)skin->inverse_bind_matrices->count;
GLint readCount = (accessorCount < gs->jointCount) ? accessorCount : gs->jointCount;
for (ji = 0; ji < readCount; ji++)
cgltf_accessor_read_float(skin->inverse_bind_matrices, ji,
                          &gs->inverseBindMatrices[ji * 16], 16);
}
}
}

// Recompute joint matrices: jointMatrix[j] = worldMatrix(joint[j]) * IBM[j].
static void computeJointMatrices(void)
{
GLint si, ji;

for (si = 0; si < g_numSkins; si++)
{
GltfSkin* gs = &g_skins[si];
for (ji = 0; ji < gs->jointCount; ji++)
{
GLint jni = gs->jointNodeIndices[ji];
glusMatrix4x4Multiplyf(&gs->jointMatrices[ji * 16],
                       g_nodes[jni].worldMatrix,
                       &gs->inverseBindMatrices[ji * 16]);
}
}
}

//
// Animation helpers
//

// Load all animation channels from all clips into a flat list.
static void loadAnimations(const cgltf_data* data)
{
GLint    ai, chi, kfi;
GLfloat  maxTime;

maxTime           = 0.0f;
g_numAnimChannels = 0;

for (ai = 0; ai < (GLint)data->animations_count; ai++)
{
const cgltf_animation* anim = &data->animations[ai];

for (chi = 0; chi < (GLint)anim->channels_count; chi++)
{
const cgltf_animation_channel* ch      = &anim->channels[chi];
const cgltf_animation_sampler* sampler = ch->sampler;
AnimChannel* ac;
GLint path, interp, components, outputCount;

if (!ch->target_node) continue;
if (g_numAnimChannels >= MAX_ANIM_CHANNELS) break;

switch (ch->target_path)
{
case cgltf_animation_path_type_translation: path = ANIM_PATH_TRANSLATION; break;
case cgltf_animation_path_type_rotation:    path = ANIM_PATH_ROTATION;    break;
case cgltf_animation_path_type_scale:       path = ANIM_PATH_SCALE;       break;
default: continue;
}

switch (sampler->interpolation)
{
case cgltf_interpolation_type_step:         interp = GLUS_ANIMATION_STEP;        break;
case cgltf_interpolation_type_cubic_spline: interp = GLUS_ANIMATION_CUBICSPLINE; break;
default:                                    interp = GLUS_ANIMATION_LINEAR;      break;
}

ac              = &g_animChannels[g_numAnimChannels++];
ac->nodeIdx     = (GLint)(ch->target_node - data->nodes);
ac->path        = path;
ac->interpolation = interp;
ac->count       = (GLint)sampler->input->count;
components      = (path == ANIM_PATH_ROTATION) ? 4 : 3;
outputCount     = (GLint)sampler->output->count;

ac->times  = (GLfloat*)malloc((size_t)ac->count * sizeof(GLfloat));
ac->values = (GLfloat*)malloc((size_t)outputCount * components * sizeof(GLfloat));

for (kfi = 0; kfi < ac->count; kfi++)
cgltf_accessor_read_float(sampler->input, kfi, &ac->times[kfi], 1);

for (kfi = 0; kfi < outputCount; kfi++)
cgltf_accessor_read_float(sampler->output, kfi,
                          &ac->values[kfi * components], components);

if (ac->count > 0 && ac->times[ac->count - 1] > maxTime)
maxTime = ac->times[ac->count - 1];
}
}

g_animDuration = maxTime;
}

//
// glTF scene loading
//

static void processMesh(const cgltf_data* data, const cgltf_mesh* mesh,
                        GLint nodeIdx, GLint skinIdx, const GLUSchar* basePath)
{
GLint pi;
(void)data;

for (pi = 0; pi < (GLint)mesh->primitives_count; pi++)
{
const cgltf_primitive* prim = &mesh->primitives[pi];
GltfPrimitive*         gp;
GLfloat                tmpM[16];
const cgltf_accessor*  accPos;
const cgltf_accessor*  accNor;
const cgltf_accessor*  accTan;
const cgltf_accessor*  accUV0;
const cgltf_accessor*  accJoints;
const cgltf_accessor*  accWeights;
GLint                  ai;
GLsizei                vertCount;
GLfloat*               buf;

if (prim->type != cgltf_primitive_type_triangles) continue;
if (g_numPrimitives >= MAX_PRIMITIVES) break;

gp = &g_primitives[g_numPrimitives++];
memset(gp, 0, sizeof(*gp));

gp->nodeIdx = nodeIdx;
gp->skinIdx = skinIdx;

// World transform from pre-computed node world matrix.
memcpy(gp->modelMatrix, g_nodes[nodeIdx].worldMatrix, 64);
memcpy(tmpM, gp->modelMatrix, 64);
glusMatrix4x4Inversef(tmpM);
glusMatrix4x4Transposef(tmpM);
glusMatrix4x4ExtractMatrix3x3f(gp->normalMatrix, tmpM);

// Attribute accessors.
accPos     = NULL;
accNor     = NULL;
accTan     = NULL;
accUV0     = NULL;
accJoints  = NULL;
accWeights = NULL;

for (ai = 0; ai < (GLint)prim->attributes_count; ai++)
{
const cgltf_attribute* attr = &prim->attributes[ai];
if (attr->index != 0) continue;
switch (attr->type)
{
case cgltf_attribute_type_position: accPos     = attr->data; break;
case cgltf_attribute_type_normal:   accNor     = attr->data; break;
case cgltf_attribute_type_tangent:  accTan     = attr->data; break;
case cgltf_attribute_type_texcoord: accUV0     = attr->data; break;
case cgltf_attribute_type_joints:   accJoints  = attr->data; break;
case cgltf_attribute_type_weights:  accWeights = attr->data; break;
default: break;
}
}

if (!accPos) { g_numPrimitives--; continue; }

// Expand scene bounds using the rest-pose world matrix.
if (accPos->has_min && accPos->has_max)
{
GLfloat lMin[3];
GLfloat lMax[3];
lMin[0] = (GLfloat)accPos->min[0];
lMin[1] = (GLfloat)accPos->min[1];
lMin[2] = (GLfloat)accPos->min[2];
lMax[0] = (GLfloat)accPos->max[0];
lMax[1] = (GLfloat)accPos->max[1];
lMax[2] = (GLfloat)accPos->max[2];
expandBoundsAABB(lMin, lMax, g_nodes[nodeIdx].worldMatrix);
}

vertCount = (GLsizei)accPos->count;

// Positions.
{
GLsizei vi;
buf = (GLfloat*)malloc((size_t)vertCount * 3 * sizeof(GLfloat));
for (vi = 0; vi < vertCount; vi++)
cgltf_accessor_read_float(accPos, vi, buf + vi * 3, 3);
glGenBuffers(1, &gp->vbo_pos);
glBindBuffer(GL_ARRAY_BUFFER, gp->vbo_pos);
glBufferData(GL_ARRAY_BUFFER, vertCount * 3 * sizeof(GLfloat), buf, GL_STATIC_DRAW);
free(buf);
}

// Normals.
{
GLsizei vi;
buf = (GLfloat*)calloc((size_t)vertCount * 3, sizeof(GLfloat));
if (accNor)
for (vi = 0; vi < vertCount; vi++)
cgltf_accessor_read_float(accNor, vi, buf + vi * 3, 3);
glGenBuffers(1, &gp->vbo_nor);
glBindBuffer(GL_ARRAY_BUFFER, gp->vbo_nor);
glBufferData(GL_ARRAY_BUFFER, vertCount * 3 * sizeof(GLfloat), buf, GL_STATIC_DRAW);
free(buf);
}

// Tangents.
{
GLsizei vi;
buf = (GLfloat*)calloc((size_t)vertCount * 4, sizeof(GLfloat));
if (accTan)
for (vi = 0; vi < vertCount; vi++)
cgltf_accessor_read_float(accTan, vi, buf + vi * 4, 4);
glGenBuffers(1, &gp->vbo_tan);
glBindBuffer(GL_ARRAY_BUFFER, gp->vbo_tan);
glBufferData(GL_ARRAY_BUFFER, vertCount * 4 * sizeof(GLfloat), buf, GL_STATIC_DRAW);
free(buf);
gp->hasNormalMap = (accTan != NULL) ? 1 : 0;
}

// UVs.
{
GLsizei vi;
buf = (GLfloat*)calloc((size_t)vertCount * 2, sizeof(GLfloat));
if (accUV0)
for (vi = 0; vi < vertCount; vi++)
cgltf_accessor_read_float(accUV0, vi, buf + vi * 2, 2);
glGenBuffers(1, &gp->vbo_uv0);
glBindBuffer(GL_ARRAY_BUFFER, gp->vbo_uv0);
glBufferData(GL_ARRAY_BUFFER, vertCount * 2 * sizeof(GLfloat), buf, GL_STATIC_DRAW);
free(buf);
}

// Indices.
if (prim->indices)
{
const cgltf_accessor* accIdx  = prim->indices;
GLsizei               idxCount = (GLsizei)accIdx->count;
unsigned int*         ibuf;
GLsizei               ii;

ibuf = (unsigned int*)malloc((size_t)idxCount * sizeof(unsigned int));
for (ii = 0; ii < idxCount; ii++)
ibuf[ii] = (unsigned int)cgltf_accessor_read_index(accIdx, ii);
glGenBuffers(1, &gp->ibo);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gp->ibo);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, idxCount * sizeof(unsigned int),
             ibuf, GL_STATIC_DRAW);
free(ibuf);
gp->indexCount = idxCount;
gp->indexType  = GL_UNSIGNED_INT;
}
else
{
gp->vertexCount = vertCount;
}

// Build VAO.
glGenVertexArrays(1, &gp->vao);
glBindVertexArray(gp->vao);

glBindBuffer(GL_ARRAY_BUFFER, gp->vbo_pos);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
glEnableVertexAttribArray(0);

glBindBuffer(GL_ARRAY_BUFFER, gp->vbo_nor);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
glEnableVertexAttribArray(1);

glBindBuffer(GL_ARRAY_BUFFER, gp->vbo_tan);
glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);
glEnableVertexAttribArray(2);

glBindBuffer(GL_ARRAY_BUFFER, gp->vbo_uv0);
glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
glEnableVertexAttribArray(3);

// Skinning attributes.
if (skinIdx >= 0 && accJoints && accWeights)
{
GLfloat* jbuf;
GLfloat* wbuf;
GLuint   jval[4];
GLsizei  vi;

jbuf = (GLfloat*)malloc((size_t)vertCount * 4 * sizeof(GLfloat));
for (vi = 0; vi < vertCount; vi++)
{
cgltf_accessor_read_uint(accJoints, vi, jval, 4);
jbuf[vi * 4 + 0] = (GLfloat)jval[0];
jbuf[vi * 4 + 1] = (GLfloat)jval[1];
jbuf[vi * 4 + 2] = (GLfloat)jval[2];
jbuf[vi * 4 + 3] = (GLfloat)jval[3];
}
glGenBuffers(1, &gp->vbo_joints);
glBindBuffer(GL_ARRAY_BUFFER, gp->vbo_joints);
glBufferData(GL_ARRAY_BUFFER, vertCount * 4 * sizeof(GLfloat), jbuf, GL_STATIC_DRAW);
free(jbuf);

wbuf = (GLfloat*)malloc((size_t)vertCount * 4 * sizeof(GLfloat));
for (vi = 0; vi < vertCount; vi++)
cgltf_accessor_read_float(accWeights, vi, wbuf + vi * 4, 4);
glGenBuffers(1, &gp->vbo_weights);
glBindBuffer(GL_ARRAY_BUFFER, gp->vbo_weights);
glBufferData(GL_ARRAY_BUFFER, vertCount * 4 * sizeof(GLfloat), wbuf, GL_STATIC_DRAW);
free(wbuf);

glBindBuffer(GL_ARRAY_BUFFER, gp->vbo_joints);
glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, 0);
glEnableVertexAttribArray(4);

glBindBuffer(GL_ARRAY_BUFFER, gp->vbo_weights);
glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 0, 0);
glEnableVertexAttribArray(5);
}
else if (skinIdx >= 0)
{
// Skinned mesh without joint data — fall back to non-skinned.
gp->skinIdx = -1;
}

if (gp->ibo)
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gp->ibo);

glBindVertexArray(0);

// Material defaults.
gp->baseColorFactor[0] = 1.0f;
gp->baseColorFactor[1] = 1.0f;
gp->baseColorFactor[2] = 1.0f;
gp->baseColorFactor[3] = 1.0f;
gp->metallicFactor     = 1.0f;
gp->roughnessFactor    = 1.0f;
gp->emissiveFactor[0]  = 0.0f;
gp->emissiveFactor[1]  = 0.0f;
gp->emissiveFactor[2]  = 0.0f;
gp->occlusionStrength  = 1.0f;
gp->alphaCutoff        = 0.5f;

gp->baseColorTexture         = g_defaultWhiteTexture;
gp->metallicRoughnessTexture = g_defaultWhiteTexture;
gp->normalTexture            = g_defaultNormalTexture;
gp->occlusionTexture         = g_defaultWhiteTexture;
gp->emissiveTexture          = g_defaultWhiteTexture;

{
const cgltf_material* mat = prim->material;
if (mat)
{
gp->doubleSided = mat->double_sided ? 1 : 0;
gp->alphaMode   = (GLint)mat->alpha_mode;
gp->alphaCutoff = mat->alpha_cutoff > 0.0f ? mat->alpha_cutoff : 0.5f;

if (mat->has_pbr_metallic_roughness)
{
const cgltf_pbr_metallic_roughness* pbr = &mat->pbr_metallic_roughness;
memcpy(gp->baseColorFactor, pbr->base_color_factor, 16);
gp->metallicFactor  = pbr->metallic_factor;
gp->roughnessFactor = pbr->roughness_factor;

if (pbr->base_color_texture.texture)
{
gp->baseColorTexture = loadImageTexture(
    pbr->base_color_texture.texture->image, basePath, 1);
glBindTexture(GL_TEXTURE_2D, gp->baseColorTexture);
applyGltfSampler(pbr->base_color_texture.texture->sampler);
}
if (pbr->metallic_roughness_texture.texture)
{
gp->metallicRoughnessTexture = loadImageTexture(
    pbr->metallic_roughness_texture.texture->image, basePath, 0);
glBindTexture(GL_TEXTURE_2D, gp->metallicRoughnessTexture);
applyGltfSampler(pbr->metallic_roughness_texture.texture->sampler);
}
}

if (mat->normal_texture.texture)
{
gp->normalTexture = loadImageTexture(
    mat->normal_texture.texture->image, basePath, 0);
gp->hasNormalMap = (accTan != NULL) ? 1 : 0;
glBindTexture(GL_TEXTURE_2D, gp->normalTexture);
applyGltfSampler(mat->normal_texture.texture->sampler);
}

if (mat->occlusion_texture.texture)
{
gp->occlusionTexture = loadImageTexture(
    mat->occlusion_texture.texture->image, basePath, 0);
gp->occlusionStrength = mat->occlusion_texture.scale;
glBindTexture(GL_TEXTURE_2D, gp->occlusionTexture);
applyGltfSampler(mat->occlusion_texture.texture->sampler);
}

if (mat->emissive_texture.texture)
{
gp->emissiveTexture = loadImageTexture(
    mat->emissive_texture.texture->image, basePath, 1);
glBindTexture(GL_TEXTURE_2D, gp->emissiveTexture);
applyGltfSampler(mat->emissive_texture.texture->sampler);
}

memcpy(gp->emissiveFactor, mat->emissive_factor, 12);
}
}
}
}

// Recursive mesh-upload traversal — world matrices must be computed first.
static void processNodeMeshes(const cgltf_data* data, const cgltf_node* node,
                              const GLUSchar* basePath)
{
GLint nodeIdx = (GLint)(node - data->nodes);
GLint skinIdx = -1;
GLint ci;

if (node->skin)
skinIdx = (GLint)(node->skin - data->skins);

if (node->mesh)
processMesh(data, node->mesh, nodeIdx, skinIdx, basePath);

for (ci = 0; ci < (GLint)node->children_count; ci++)
processNodeMeshes(data, node->children[ci], basePath);
}

//
// GLUS callbacks
//

GLUSboolean init(GLUSvoid)
{
GLUShdrimage  panoramaImage;
GLuint        panoramaTex;
GLUSshape     sphere;
cgltf_options gltfOptions;
cgltf_data*   gltfData;
cgltf_result  res;
GLUSchar      basePath[1024];
GLUStextfile  vertexSource;
GLUStextfile  fragmentSource;
GLUSchar      tmp[1024];
GLUSchar*     sl;
GLUSchar*     bsl;
GLUSchar*     sep;
GLint         ni;
GLfloat       dx, dy, dz;

// ----------------------------------------------------------------
// Build shader programs.
// ----------------------------------------------------------------

glusFileLoadText("../Example50/shader/background.vert.glsl", &vertexSource);
glusFileLoadText("../Example50/shader/background.frag.glsl", &fragmentSource);
if (!glusProgramBuildFromSource(&g_bgProg, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text))
{
glusFileDestroyText(&vertexSource); glusFileDestroyText(&fragmentSource);
printf("Failed to build background program\n");
return GLUS_FALSE;
}
glusFileDestroyText(&vertexSource); glusFileDestroyText(&fragmentSource);

glusFileLoadText("../Example50/shader/fullscreen.vert.glsl", &vertexSource);
glusFileLoadText("../Example50/shader/fullscreen.frag.glsl", &fragmentSource);
if (!glusProgramBuildFromSource(&g_fullscreenProg, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text))
{
glusFileDestroyText(&vertexSource); glusFileDestroyText(&fragmentSource);
printf("Failed to build fullscreen program\n");
return GLUS_FALSE;
}
glusFileDestroyText(&vertexSource); glusFileDestroyText(&fragmentSource);

glusFileLoadText("../Example50/shader/glus_gltf_pbr.vert.glsl", &vertexSource);
glusFileLoadText("../Example50/shader/glus_gltf_pbr.frag.glsl", &fragmentSource);
if (!glusProgramBuildFromSource(&g_pbrProg, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text))
{
glusFileDestroyText(&vertexSource); glusFileDestroyText(&fragmentSource);
printf("Failed to build PBR program\n");
return GLUS_FALSE;
}
glusFileDestroyText(&vertexSource); glusFileDestroyText(&fragmentSource);

glusFileLoadText("../Example50/shader/glus_gltf_pbr_skinned.vert.glsl", &vertexSource);
glusFileLoadText("../Example50/shader/glus_gltf_pbr.frag.glsl", &fragmentSource);
if (!glusProgramBuildFromSource(&g_pbrSkinnedProg, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text))
{
glusFileDestroyText(&vertexSource); glusFileDestroyText(&fragmentSource);
printf("Failed to build skinned PBR program\n");
return GLUS_FALSE;
}
glusFileDestroyText(&vertexSource); glusFileDestroyText(&fragmentSource);

// ----------------------------------------------------------------
// Fullscreen VAO (attribute-less draw using gl_VertexID).
// ----------------------------------------------------------------

glGenVertexArrays(1, &g_fullscreenVAO);

// ----------------------------------------------------------------
// Default textures.
// ----------------------------------------------------------------

g_defaultWhiteTexture  = createTexture1x1(255, 255, 255, 255);
g_defaultNormalTexture = createTexture1x1(128, 128, 255, 255);

// ----------------------------------------------------------------
// IBL — load panorama and run all four GPU prefilter passes.
// ----------------------------------------------------------------

printf("Loading panorama '%s' ...\n", g_panoramaPath);
if (!glusImageLoadHdr(g_panoramaPath, &panoramaImage))
{
printf("Error: failed to load panorama '%s'\n", g_panoramaPath);
return GLUS_FALSE;
}

// Convert the panorama from linear Rec.709 (sRGB primaries) into linear
// Rec.2020 (BT.2020 primaries) so all derived IBL textures - the
// pre-filtered specular cubemap, the diffuse irradiance cubemap, and the
// background cubemap - end up in the working color space already.  The
// .hdr file is float32 RGB so this conversion is lossless.
{
	const float m00 = 0.62740389896f, m01 = 0.32928303716f, m02 = 0.04331306388f;
	const float m10 = 0.06909728935f, m11 = 0.91954039507f, m12 = 0.01136231558f;
	const float m20 = 0.01639143887f, m21 = 0.08801330909f, m22 = 0.89559525204f;

	GLfloat* p   = (GLfloat*)panoramaImage.data;
	size_t   pix = (size_t)panoramaImage.width * (size_t)panoramaImage.height;
	for (size_t i = 0; i < pix; ++i)
	{
		float r = p[0], g = p[1], b = p[2];
		p[0] = m00 * r + m01 * g + m02 * b;
		p[1] = m10 * r + m11 * g + m12 * b;
		p[2] = m20 * r + m21 * g + m22 * b;
		p += 3;
	}
}

glGenTextures(1, &panoramaTex);
glBindTexture(GL_TEXTURE_2D, panoramaTex);
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F,
             panoramaImage.width, panoramaImage.height,
             0, GL_RGB, GL_FLOAT, panoramaImage.data);
glusImageDestroyHdr(&panoramaImage);
glGenerateMipmap(GL_TEXTURE_2D);

glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glBindTexture(GL_TEXTURE_2D, 0);

glusIblSetShaderPath("../GLUS/shader/");

printf("Pre-filtering specular cubemap (%dx%d, %d levels) ...\n",
       SPECULAR_CUBEMAP_SIZE, SPECULAR_CUBEMAP_SIZE, NUMBER_ROUGHNESS);
if (!glusIblBuildSpecularEnvironmentMap(&g_specularTexture, panoramaTex,
                                        SPECULAR_CUBEMAP_SIZE, NUMBER_ROUGHNESS))
{
printf("Error: glusIblBuildSpecularEnvironmentMap failed\n");
return GLUS_FALSE;
}

printf("Pre-filtering diffuse irradiance cubemap (%dx%d) ...\n",
       DIFFUSE_CUBEMAP_SIZE, DIFFUSE_CUBEMAP_SIZE);
if (!glusIblBuildDiffuseEnvironmentMap(&g_diffuseTexture, panoramaTex,
                                       DIFFUSE_CUBEMAP_SIZE))
{
printf("Error: glusIblBuildDiffuseEnvironmentMap failed\n");
return GLUS_FALSE;
}

printf("Integrating BRDF LUT (%dx%d) ...\n", BRDF_LUT_SIZE, BRDF_LUT_SIZE);
if (!glusIblBuildBrdfLookupTable(&g_brdfLutTexture, BRDF_LUT_SIZE))
{
printf("Error: glusIblBuildBrdfLookupTable failed\n");
return GLUS_FALSE;
}

printf("Building background cubemap (%dx%d) ...\n",
       BACKGROUND_CUBEMAP_SIZE, BACKGROUND_CUBEMAP_SIZE);
if (!glusIblBuildBackgroundCubemap(&g_bgCubemapTexture, panoramaTex,
                                   BACKGROUND_CUBEMAP_SIZE))
{
printf("Error: glusIblBuildBackgroundCubemap failed\n");
return GLUS_FALSE;
}

glDeleteTextures(1, &panoramaTex);

// ----------------------------------------------------------------
// Background sphere.
// ----------------------------------------------------------------

glusShapeCreateSpheref(&sphere, 500.0f, 32);
g_bgIndexCount = sphere.numberIndices;

glGenVertexArrays(1, &g_bgVAO);
glBindVertexArray(g_bgVAO);

glGenBuffers(1, &g_bgVBO);
glBindBuffer(GL_ARRAY_BUFFER, g_bgVBO);
glBufferData(GL_ARRAY_BUFFER, sphere.numberVertices * 4 * sizeof(GLfloat),
             sphere.vertices, GL_STATIC_DRAW);
glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
glEnableVertexAttribArray(0);

glGenBuffers(1, &g_bgIBO);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_bgIBO);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.numberIndices * sizeof(GLuint),
             sphere.indices, GL_STATIC_DRAW);

glBindVertexArray(0);
glusShapeDestroyf(&sphere);

// ----------------------------------------------------------------
// Set persistent sampler / scalar uniforms.
// ----------------------------------------------------------------

glUseProgram(g_bgProg.program);
glUniform1i(glGetUniformLocation(g_bgProg.program, "u_texture"), 0);

glUseProgram(g_fullscreenProg.program);
glUniform1i(glGetUniformLocation(g_fullscreenProg.program, "u_framebufferTexture"), 0);
glUniform1i(glGetUniformLocation(g_fullscreenProg.program, "u_msaaSamples"), MSAA_SAMPLES);

glUseProgram(g_pbrProg.program);
glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_baseColorTexture"),         0);
glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_metallicRoughnessTexture"), 1);
glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_normalTexture"),            2);
glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_occlusionTexture"),         3);
glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_emissiveTexture"),          4);
glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_specularEnvMap"),           5);
glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_diffuseEnvMap"),            6);
glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_brdfLUT"),                  7);
glUniform1f(glGetUniformLocation(g_pbrProg.program, "u_roughnessScale"),
            (GLfloat)(NUMBER_ROUGHNESS - 1));

g_u_modelMatrix       = glGetUniformLocation(g_pbrProg.program, "u_modelMatrix");
g_u_vpMatrix          = glGetUniformLocation(g_pbrProg.program, "u_viewProjectionMatrix");
g_u_normalMatrix      = glGetUniformLocation(g_pbrProg.program, "u_normalMatrix");
g_u_eye_pbr           = glGetUniformLocation(g_pbrProg.program, "u_eye");
g_u_baseColorFactor   = glGetUniformLocation(g_pbrProg.program, "u_baseColorFactor");
g_u_metallicFactor    = glGetUniformLocation(g_pbrProg.program, "u_metallicFactor");
g_u_roughnessFactor   = glGetUniformLocation(g_pbrProg.program, "u_roughnessFactor");
g_u_emissiveFactor    = glGetUniformLocation(g_pbrProg.program, "u_emissiveFactor");
g_u_occlusionStrength = glGetUniformLocation(g_pbrProg.program, "u_occlusionStrength");
g_u_alphaCutoff       = glGetUniformLocation(g_pbrProg.program, "u_alphaCutoff");
g_u_alphaMode         = glGetUniformLocation(g_pbrProg.program, "u_alphaMode");
g_u_hasNormalMap      = glGetUniformLocation(g_pbrProg.program, "u_hasNormalMap");

glUseProgram(g_pbrSkinnedProg.program);
glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_baseColorTexture"),         0);
glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_metallicRoughnessTexture"), 1);
glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_normalTexture"),            2);
glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_occlusionTexture"),         3);
glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_emissiveTexture"),          4);
glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_specularEnvMap"),           5);
glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_diffuseEnvMap"),            6);
glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_brdfLUT"),                  7);
glUniform1f(glGetUniformLocation(g_pbrSkinnedProg.program, "u_roughnessScale"),
            (GLfloat)(NUMBER_ROUGHNESS - 1));

g_u_vpMatrix_sk          = glGetUniformLocation(g_pbrSkinnedProg.program, "u_viewProjectionMatrix");
g_u_eye_sk               = glGetUniformLocation(g_pbrSkinnedProg.program, "u_eye");
g_u_jointMatrices_sk     = glGetUniformLocation(g_pbrSkinnedProg.program, "u_jointMatrices");
g_u_baseColorFactor_sk   = glGetUniformLocation(g_pbrSkinnedProg.program, "u_baseColorFactor");
g_u_metallicFactor_sk    = glGetUniformLocation(g_pbrSkinnedProg.program, "u_metallicFactor");
g_u_roughnessFactor_sk   = glGetUniformLocation(g_pbrSkinnedProg.program, "u_roughnessFactor");
g_u_emissiveFactor_sk    = glGetUniformLocation(g_pbrSkinnedProg.program, "u_emissiveFactor");
g_u_occlusionStrength_sk = glGetUniformLocation(g_pbrSkinnedProg.program, "u_occlusionStrength");
g_u_alphaCutoff_sk       = glGetUniformLocation(g_pbrSkinnedProg.program, "u_alphaCutoff");
g_u_alphaMode_sk         = glGetUniformLocation(g_pbrSkinnedProg.program, "u_alphaMode");
g_u_hasNormalMap_sk      = glGetUniformLocation(g_pbrSkinnedProg.program, "u_hasNormalMap");

g_u_vpMatrix_bg = glGetUniformLocation(g_bgProg.program, "u_viewProjectionMatrix");

g_u_gamma              = glGetUniformLocation(g_fullscreenProg.program, "u_gamma");
g_u_msaaSamples        = glGetUniformLocation(g_fullscreenProg.program, "u_msaaSamples");
g_u_outputColorspace   = glGetUniformLocation(g_fullscreenProg.program, "u_outputColorspace");
g_u_referenceWhiteNits = glGetUniformLocation(g_fullscreenProg.program, "u_referenceWhiteNits");
g_u_peakNits           = glGetUniformLocation(g_fullscreenProg.program, "u_peakNits");

glUseProgram(0);

// ----------------------------------------------------------------
// Load glTF scene.
// ----------------------------------------------------------------

memset(&gltfOptions, 0, sizeof(gltfOptions));
gltfData = NULL;

printf("Loading glTF: %s\n", g_gltfPath);
res = cgltf_parse_file(&gltfOptions, g_gltfPath, &gltfData);
if (res != cgltf_result_success)
{
printf("Error: cgltf_parse_file failed (%d)\n", res);
return GLUS_FALSE;
}
res = cgltf_load_buffers(&gltfOptions, gltfData, g_gltfPath);
if (res != cgltf_result_success)
{
printf("Error: cgltf_load_buffers failed (%d)\n", res);
cgltf_free(gltfData);
return GLUS_FALSE;
}

// Derive base path for external image loading.
memset(basePath, 0, sizeof(basePath));
strncpy(tmp, g_gltfPath, sizeof(tmp) - 1);
tmp[sizeof(tmp) - 1] = '\0';
sl  = strrchr(tmp, '/');
bsl = strrchr(tmp, '\\');
sep = (sl > bsl) ? sl : bsl;
if (sep) { *(sep + 1) = '\0'; strncpy(basePath, tmp, sizeof(basePath) - 1); }

// Build node hierarchy and initial world matrices.
buildNodeHierarchy(gltfData);
buildRootNodes(gltfData);
computeAllWorldMatrices();

// Load skins and initial joint matrices.
loadSkins(gltfData);
computeJointMatrices();

// Load animation channels.
loadAnimations(gltfData);

// Upload mesh data (world matrices must be ready).
if (gltfData->scene)
{
for (ni = 0; ni < (GLint)gltfData->scene->nodes_count; ni++)
processNodeMeshes(gltfData, gltfData->scene->nodes[ni], basePath);
}
else
{
for (ni = 0; ni < (GLint)gltfData->nodes_count; ni++)
processNodeMeshes(gltfData, &gltfData->nodes[ni], basePath);
}

cgltf_free(gltfData);
printf("Loaded %d primitives, %d textures, %d skins, %d anim channels (duration %.2fs)\n",
       g_numPrimitives, g_numTextures, g_numSkins, g_numAnimChannels, g_animDuration);

// ----------------------------------------------------------------
// Compute scene centre / orbit radius from AABB.
// ----------------------------------------------------------------

if (g_numPrimitives > 0)
{
g_sceneCenterX = (g_sceneMin[0] + g_sceneMax[0]) * 0.5f;
g_sceneCenterY = (g_sceneMin[1] + g_sceneMax[1]) * 0.5f;
g_sceneCenterZ = (g_sceneMin[2] + g_sceneMax[2]) * 0.5f;
dx = g_sceneMax[0] - g_sceneMin[0];
dy = g_sceneMax[1] - g_sceneMin[1];
dz = g_sceneMax[2] - g_sceneMin[2];
g_sceneRadius = sqrtf(dx*dx + dy*dy + dz*dz) * 0.5f;
if (g_sceneRadius < SCENE_RADIUS_MIN) g_sceneRadius = 1.0f;
}
g_orbitRadius = g_sceneRadius * CAMERA_ORBIT_RADIUS_FACTOR;
if (g_orbitRadius < CAMERA_ORBIT_RADIUS_MIN) g_orbitRadius = CAMERA_ORBIT_RADIUS_MIN;
g_cameraY = g_sceneCenterY + g_sceneRadius * CAMERA_HEIGHT_OFFSET;

// ----------------------------------------------------------------
// General GL state.
// ----------------------------------------------------------------

glEnable(GL_DEPTH_TEST);
glDepthFunc(GL_LEQUAL);
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);
glFrontFace(GL_CCW);
glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

printf("Scene centre (%.2f, %.2f, %.2f)  radius %.2f  orbit %.2f\n",
       g_sceneCenterX, g_sceneCenterY, g_sceneCenterZ,
       g_sceneRadius, g_orbitRadius);

return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
if (height == 0) height = 1;
g_windowWidth  = width;
g_windowHeight = height;

if (g_msaaFBO)
{
glDeleteFramebuffers(1, &g_msaaFBO);
glDeleteTextures(1, &g_msaaColor);
glDeleteRenderbuffers(1, &g_msaaDepth);
}

glGenFramebuffers(1, &g_msaaFBO);
glBindFramebuffer(GL_FRAMEBUFFER, g_msaaFBO);

glGenTextures(1, &g_msaaColor);
glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, g_msaaColor);
glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_SAMPLES,
                        GL_RGBA16F, width, height, GL_TRUE);
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                       GL_TEXTURE_2D_MULTISAMPLE, g_msaaColor, 0);

glGenRenderbuffers(1, &g_msaaDepth);
glBindRenderbuffer(GL_RENDERBUFFER, g_msaaDepth);
glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_SAMPLES,
                                 GL_DEPTH_COMPONENT24, width, height);
glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                          GL_RENDERBUFFER, g_msaaDepth);

glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Bind material textures and select the correct PBR program.
static void bindPrimitiveMaterial(const GltfPrimitive* gp)
{
GLint isSkinned = (gp->skinIdx >= 0);

glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, gp->baseColorTexture);
glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, gp->metallicRoughnessTexture);
glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, gp->normalTexture);
glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, gp->occlusionTexture);
glActiveTexture(GL_TEXTURE4); glBindTexture(GL_TEXTURE_2D, gp->emissiveTexture);

if (isSkinned)
{
const GltfSkin* gs = &g_skins[gp->skinIdx];
glUseProgram(g_pbrSkinnedProg.program);
glUniformMatrix4fv(g_u_jointMatrices_sk, gs->jointCount, GL_FALSE, gs->jointMatrices);
glUniform4fv(g_u_baseColorFactor_sk,   1, gp->baseColorFactor);
glUniform1f(g_u_metallicFactor_sk,        gp->metallicFactor);
glUniform1f(g_u_roughnessFactor_sk,       gp->roughnessFactor);
glUniform3fv(g_u_emissiveFactor_sk,    1, gp->emissiveFactor);
glUniform1f(g_u_occlusionStrength_sk,     gp->occlusionStrength);
glUniform1f(g_u_alphaCutoff_sk,           gp->alphaCutoff);
glUniform1i(g_u_alphaMode_sk,             gp->alphaMode);
glUniform1i(g_u_hasNormalMap_sk,          gp->hasNormalMap);
}
else
{
glUseProgram(g_pbrProg.program);
glUniformMatrix4fv(g_u_modelMatrix,  1, GL_FALSE, gp->modelMatrix);
glUniformMatrix3fv(g_u_normalMatrix, 1, GL_FALSE, gp->normalMatrix);
glUniform4fv(g_u_baseColorFactor,    1, gp->baseColorFactor);
glUniform1f(g_u_metallicFactor,         gp->metallicFactor);
glUniform1f(g_u_roughnessFactor,        gp->roughnessFactor);
glUniform3fv(g_u_emissiveFactor,     1, gp->emissiveFactor);
glUniform1f(g_u_occlusionStrength,      gp->occlusionStrength);
glUniform1f(g_u_alphaCutoff,            gp->alphaCutoff);
glUniform1i(g_u_alphaMode,              gp->alphaMode);
glUniform1i(g_u_hasNormalMap,           gp->hasNormalMap);
}
}

static void drawPrimitive(const GltfPrimitive* gp)
{
glBindVertexArray(gp->vao);
if (gp->ibo)
glDrawElements(GL_TRIANGLES, gp->indexCount, gp->indexType, 0);
else
glDrawArrays(GL_TRIANGLES, 0, gp->vertexCount);
glBindVertexArray(0);
}

GLUSboolean update(GLUSfloat time)
{
GLfloat rad;
GLfloat eyeX, eyeY, eyeZ;
GLfloat nearPlane, farPlane;
GLfloat viewMatrix[16];
GLfloat projMatrix[16];
GLfloat bgViewMatrix[16];
GLfloat bgVPMatrix[16];
GLint   i;

if (!g_msaaFBO) return GLUS_TRUE;

// --- Orbit camera ---
// Reverse-orbit camera (opposite direction to Example49).
g_orbitAngle += g_orbitSpeed * time;
if (g_orbitAngle >= 360.0f) g_orbitAngle = fmodf(g_orbitAngle, 360.0f);

rad  = g_orbitAngle * CAMERA_DEG_TO_RAD;
eyeX = g_sceneCenterX + g_orbitRadius * sinf(rad);
eyeY = g_cameraY;
eyeZ = g_sceneCenterZ + g_orbitRadius * cosf(rad);

g_eye[0] = eyeX; g_eye[1] = eyeY; g_eye[2] = eyeZ; g_eye[3] = 1.0f;

nearPlane = g_sceneRadius * CAMERA_NEAR_FACTOR;
farPlane  = g_orbitRadius + g_sceneRadius * CAMERA_FAR_RADIUS_FACTOR + CAMERA_FAR_EXTRA;
if (nearPlane < CAMERA_NEAR_MIN) nearPlane = CAMERA_NEAR_MIN;

glusMatrix4x4LookAtf(viewMatrix, eyeX, eyeY, eyeZ,
                     g_sceneCenterX, g_sceneCenterY, g_sceneCenterZ,
                     0.0f, 1.0f, 0.0f);
glusMatrix4x4Perspectivef(projMatrix, CAMERA_FOV_DEG,
                          (GLfloat)g_windowWidth / (GLfloat)g_windowHeight,
                          nearPlane, farPlane);
glusMatrix4x4Multiplyf(g_viewProjectionMatrix, projMatrix, viewMatrix);

memcpy(bgViewMatrix, viewMatrix, 16 * sizeof(GLfloat));
bgViewMatrix[12] = 0.0f;
bgViewMatrix[13] = 0.0f;
bgViewMatrix[14] = 0.0f;
glusMatrix4x4Multiplyf(bgVPMatrix, projMatrix, bgViewMatrix);

// --- Animation update ---
if (g_animDuration > 0.0f)
{
GLint ci;

g_animTime += time;
if (g_animTime > g_animDuration)
g_animTime = fmodf(g_animTime, g_animDuration);

for (ci = 0; ci < g_numAnimChannels; ci++)
{
AnimChannel* ac = &g_animChannels[ci];
GltfNode*    gn = &g_nodes[ac->nodeIdx];

switch (ac->path)
{
case ANIM_PATH_TRANSLATION:
glusAnimationSampleVec3f(gn->translation, ac->times, ac->values,
                         ac->count, ac->interpolation, g_animTime);
break;
case ANIM_PATH_ROTATION:
glusAnimationSampleQuaternionf(gn->rotation, ac->times, ac->values,
                               ac->count, ac->interpolation, g_animTime);
break;
case ANIM_PATH_SCALE:
glusAnimationSampleVec3f(gn->scale, ac->times, ac->values,
                         ac->count, ac->interpolation, g_animTime);
break;
default:
break;
}
}

computeAllWorldMatrices();
computeJointMatrices();

// Refresh model/normal matrices for non-skinned animated nodes.
for (i = 0; i < g_numPrimitives; i++)
{
GltfPrimitive* gp = &g_primitives[i];
GLfloat        tmpM[16];

if (gp->skinIdx >= 0) continue;

memcpy(gp->modelMatrix, g_nodes[gp->nodeIdx].worldMatrix, 64);
memcpy(tmpM, gp->modelMatrix, 64);
glusMatrix4x4Inversef(tmpM);
glusMatrix4x4Transposef(tmpM);
glusMatrix4x4ExtractMatrix3x3f(gp->normalMatrix, tmpM);
}
}

// --- Render into MSAA FBO ---
glBindFramebuffer(GL_FRAMEBUFFER, g_msaaFBO);
glViewport(0, 0, g_windowWidth, g_windowHeight);
glEnable(GL_MULTISAMPLE);
glEnable(GL_DEPTH_TEST);
glDepthMask(GL_TRUE);
glDisable(GL_BLEND);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// --- Background sphere ---
glFrontFace(GL_CW);
glDisable(GL_CULL_FACE);
glDepthMask(GL_FALSE);
glDepthFunc(GL_LEQUAL);
glUseProgram(g_bgProg.program);
glUniformMatrix4fv(g_u_vpMatrix_bg, 1, GL_FALSE, bgVPMatrix);
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_CUBE_MAP, g_bgCubemapTexture);
glBindVertexArray(g_bgVAO);
glDrawElements(GL_TRIANGLES, g_bgIndexCount, GL_UNSIGNED_INT, 0);
glBindVertexArray(0);
glDepthFunc(GL_LESS);
glFrontFace(GL_CCW);
glDepthMask(GL_TRUE);

// --- IBL textures on units 5-7 (persistent per frame) ---
glActiveTexture(GL_TEXTURE5);
glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, g_specularTexture);
glActiveTexture(GL_TEXTURE6);
glBindTexture(GL_TEXTURE_CUBE_MAP, g_diffuseTexture);
glActiveTexture(GL_TEXTURE7);
glBindTexture(GL_TEXTURE_2D, g_brdfLutTexture);

// Set VP + eye on both PBR programs before the draw loop.
glUseProgram(g_pbrProg.program);
glUniformMatrix4fv(g_u_vpMatrix, 1, GL_FALSE, g_viewProjectionMatrix);
glUniform4fv(g_u_eye_pbr, 1, g_eye);

glUseProgram(g_pbrSkinnedProg.program);
glUniformMatrix4fv(g_u_vpMatrix_sk, 1, GL_FALSE, g_viewProjectionMatrix);
glUniform4fv(g_u_eye_sk, 1, g_eye);

// --- Pass A: Opaque + Mask ---
glEnable(GL_DEPTH_TEST);
for (i = 0; i < g_numPrimitives; i++)
{
const GltfPrimitive* gp = &g_primitives[i];
if (gp->alphaMode == 2) continue;

if (gp->doubleSided) glDisable(GL_CULL_FACE);
else                 glEnable(GL_CULL_FACE);

bindPrimitiveMaterial(gp);
drawPrimitive(gp);
}

// --- Pass B: Blend ---
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glDepthMask(GL_FALSE);

for (i = 0; i < g_numPrimitives; i++)
{
const GltfPrimitive* gp = &g_primitives[i];
if (gp->alphaMode != 2) continue;

if (gp->doubleSided) glDisable(GL_CULL_FACE);
else                 glEnable(GL_CULL_FACE);

bindPrimitiveMaterial(gp);
drawPrimitive(gp);
}

glDisable(GL_BLEND);
glDepthMask(GL_TRUE);
glEnable(GL_CULL_FACE);

// --- Fullscreen resolve: MSAA -> default FBO with tone-map ---
glBindFramebuffer(GL_FRAMEBUFFER, 0);
glViewport(0, 0, g_windowWidth, g_windowHeight);
glDisable(GL_DEPTH_TEST);
glDisable(GL_MULTISAMPLE);
glClear(GL_COLOR_BUFFER_BIT);

glUseProgram(g_fullscreenProg.program);
glUniform1f(g_u_gamma,              g_gamma);
glUniform1i(g_u_msaaSamples,        MSAA_SAMPLES);
glUniform1i(g_u_outputColorspace,   g_outputColorspace);
glUniform1f(g_u_referenceWhiteNits, g_referenceWhiteNits);
glUniform1f(g_u_peakNits,           g_peakNits);
glActiveTexture(GL_TEXTURE0);
glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, g_msaaColor);
glBindVertexArray(g_fullscreenVAO);
glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
glBindVertexArray(0);

glEnable(GL_DEPTH_TEST);

return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
GLint i;

// Primitives
for (i = 0; i < g_numPrimitives; i++)
{
GltfPrimitive* gp = &g_primitives[i];
glDeleteVertexArrays(1, &gp->vao);
glDeleteBuffers(1, &gp->vbo_pos);
glDeleteBuffers(1, &gp->vbo_nor);
glDeleteBuffers(1, &gp->vbo_tan);
glDeleteBuffers(1, &gp->vbo_uv0);
if (gp->vbo_joints)  glDeleteBuffers(1, &gp->vbo_joints);
if (gp->vbo_weights) glDeleteBuffers(1, &gp->vbo_weights);
if (gp->ibo)         glDeleteBuffers(1, &gp->ibo);
}

// Textures
for (i = 0; i < g_numTextures; i++)
glDeleteTextures(1, &g_textures[i]);
glDeleteTextures(1, &g_defaultWhiteTexture);
glDeleteTextures(1, &g_defaultNormalTexture);

// IBL
glDeleteTextures(1, &g_specularTexture);
glDeleteTextures(1, &g_diffuseTexture);
glDeleteTextures(1, &g_brdfLutTexture);
glDeleteTextures(1, &g_bgCubemapTexture);

// Background sphere
glDeleteVertexArrays(1, &g_bgVAO);
glDeleteBuffers(1, &g_bgVBO);
glDeleteBuffers(1, &g_bgIBO);

// VAOs
glDeleteVertexArrays(1, &g_fullscreenVAO);

// MSAA FBO
if (g_msaaFBO)
{
glDeleteFramebuffers(1, &g_msaaFBO);
glDeleteTextures(1, &g_msaaColor);
glDeleteRenderbuffers(1, &g_msaaDepth);
}

// Animation channel data
for (i = 0; i < g_numAnimChannels; i++)
{
free(g_animChannels[i].times);
free(g_animChannels[i].values);
}

// Programs
glusProgramDestroy(&g_bgProg);
glusProgramDestroy(&g_fullscreenProg);
glusProgramDestroy(&g_pbrProg);
glusProgramDestroy(&g_pbrSkinnedProg);
}

GLUSvoid key(const GLUSboolean pressed, const GLUSint key)
{
if (!pressed) return;

switch (key)
{
case 265:  // Arrow up   — camera higher
g_cameraY += g_sceneRadius * CAMERA_HEIGHT_STEP;
break;
case 264:  // Arrow down — camera lower
g_cameraY -= g_sceneRadius * CAMERA_HEIGHT_STEP;
break;
case 262:  // Arrow right — orbit faster
g_orbitSpeed += CAMERA_ORBIT_SPEED_STEP;
break;
case 263:  // Arrow left  — orbit slower
g_orbitSpeed -= CAMERA_ORBIT_SPEED_STEP;
if (g_orbitSpeed < 0.0f) g_orbitSpeed = 0.0f;
break;
case '+':
case '=':
g_orbitRadius *= CAMERA_ZOOM_IN_FACTOR;
if (g_orbitRadius < g_sceneRadius * CAMERA_ZOOM_MIN_FACTOR)
g_orbitRadius = g_sceneRadius * CAMERA_ZOOM_MIN_FACTOR;
break;
case '-':
g_orbitRadius *= CAMERA_ZOOM_OUT_FACTOR;
break;
default:
break;
}
}

// ---------------------------------------------------------------------
// Output color space selection and Win32 + McNopper/EGL bootstrap.
// ---------------------------------------------------------------------

typedef struct
{
	int          id;            // OUT_*
	const char*  name;          // human-readable
	const char*  extension;     // EGL extension that signals support
	EGLint       colorspace;    // EGL_GL_COLORSPACE_* attribute value
	EGLint       redSize;
	EGLint       greenSize;
	EGLint       blueSize;
	EGLint       alphaSize;
} OutputColorspaceCandidate;

// Candidates ordered by preference.  The user explicitly asked for Rec.2020
// preferred, so all BT.2020 variants come first; scRGB / Display P3 / sRGB
// are progressive fallbacks.
static const OutputColorspaceCandidate g_candidates[] = {
	{ OUT_BT2020_PQ,         "BT.2020 PQ (HDR10)",        "EGL_EXT_gl_colorspace_bt2020_pq",     EGL_GL_COLORSPACE_BT2020_PQ_EXT,     10, 10, 10,  2 },
	{ OUT_BT2020_HLG,        "BT.2020 HLG",               "EGL_EXT_gl_colorspace_bt2020_hlg",    EGL_GL_COLORSPACE_BT2020_HLG_EXT,    10, 10, 10,  2 },
	{ OUT_BT2020_LINEAR,     "BT.2020 linear",            "EGL_EXT_gl_colorspace_bt2020_linear", EGL_GL_COLORSPACE_BT2020_LINEAR_EXT, 16, 16, 16, 16 },
	{ OUT_SCRGB_LINEAR,      "scRGB linear (fp16)",       "EGL_EXT_gl_colorspace_scrgb_linear",  EGL_GL_COLORSPACE_SCRGB_LINEAR_EXT,  16, 16, 16, 16 },
	{ OUT_SCRGB,             "scRGB",                     "EGL_EXT_gl_colorspace_scrgb",         EGL_GL_COLORSPACE_SCRGB_EXT,         16, 16, 16, 16 },
	{ OUT_DISPLAY_P3_LINEAR, "Display P3 linear",         "EGL_EXT_gl_colorspace_display_p3_linear", EGL_GL_COLORSPACE_DISPLAY_P3_LINEAR_EXT, 16, 16, 16, 16 },
	{ OUT_DISPLAY_P3,        "Display P3",                "EGL_EXT_gl_colorspace_display_p3",    EGL_GL_COLORSPACE_DISPLAY_P3_EXT,     8,  8,  8,  8 },
	{ OUT_SRGB,              "sRGB",                      NULL,                                  EGL_GL_COLORSPACE_SRGB,               8,  8,  8,  8 },
	{ OUT_LINEAR,            "linear (Rec.709)",          NULL,                                  EGL_GL_COLORSPACE_LINEAR,             8,  8,  8,  8 },
};
static const int g_numCandidates = (int)(sizeof(g_candidates) / sizeof(g_candidates[0]));

#ifdef _WIN32

static const wchar_t* WIN32_CLASS_NAME = L"Example50WindowClass";

static HWND   g_hwnd            = NULL;
static HDC    g_hdc             = NULL;
static GLint  g_runWidth        = SCREEN_WIDTH;
static GLint  g_runHeight       = SCREEN_HEIGHT;
static int    g_shouldClose     = 0;
static int    g_glReady        = 0;
static double g_lastTimeSeconds = 0.0;

static double winNowSeconds(void)
{
	LARGE_INTEGER c, f;
	QueryPerformanceCounter(&c);
	QueryPerformanceFrequency(&f);
	return (double)c.QuadPart / (double)f.QuadPart;
}

static int translateGlfwKey(WPARAM wp)
{
	// Reuse the GLFW key codes that Example49's key() handler already speaks.
	switch (wp)
	{
	case VK_UP:    return 265;
	case VK_DOWN:  return 264;
	case VK_LEFT:  return 263;
	case VK_RIGHT: return 262;
	case VK_OEM_PLUS:  case VK_ADD:      return '+';
	case VK_OEM_MINUS: case VK_SUBTRACT: return '-';
	default:
		if (wp >= 0x20 && wp < 0x80) return (int)wp;
		return -1;
	}
}

static LRESULT CALLBACK winProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_CLOSE:
	case WM_DESTROY:
		g_shouldClose = 1;
		PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		g_runWidth  = LOWORD(lp);
		g_runHeight = HIWORD(lp);
		if (g_glReady && g_runWidth > 0 && g_runHeight > 0)
			reshape(g_runWidth, g_runHeight);
		return 0;
	case WM_KEYDOWN:
		if (wp == VK_ESCAPE) { g_shouldClose = 1; PostQuitMessage(0); return 0; }
		{
			int k = translateGlfwKey(wp);
			if (k >= 0) key(GLUS_TRUE, k);
		}
		return 0;
	case WM_KEYUP:
		{
			int k = translateGlfwKey(wp);
			if (k >= 0) key(GLUS_FALSE, k);
		}
		return 0;
	}
	return DefWindowProcW(hwnd, msg, wp, lp);
}

static HWND createNativeWindow(int width, int height, const char* title)
{
	HINSTANCE hi = GetModuleHandleW(NULL);
	WNDCLASSW wc;
	memset(&wc, 0, sizeof(wc));
	wc.style         = CS_OWNDC;
	wc.lpfnWndProc   = winProc;
	wc.hInstance     = hi;
	wc.hCursor       = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
	wc.lpszClassName = WIN32_CLASS_NAME;
	RegisterClassW(&wc);

	wchar_t wtitle[256];
	MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, 256);

	RECT r = { 0, 0, width, height };
	AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hwnd = CreateWindowExW(0, WIN32_CLASS_NAME, wtitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		r.right - r.left, r.bottom - r.top,
		NULL, NULL, hi, NULL);
	return hwnd;
}

#endif // _WIN32

static int extensionPresent(const char* exts, const char* ext)
{
	if (!exts || !ext) return 0;
	const char* p = strstr(exts, ext);
	if (!p) return 0;
	char tail = p[strlen(ext)];
	return (tail == ' ' || tail == '\0');
}

int main(int argc, char** argv)
{
#ifndef _WIN32
	(void)argc; (void)argv;
	fprintf(stderr, "Example50 currently only ships a Windows native-window backend.\n");
	return 1;
#else
	g_gltfPath     = (argc > 1) ? argv[1] : "phoenix/scene.gltf";
	g_panoramaPath = (argc > 2) ? argv[2] : "sunny_rose_garden_4k.hdr";

	// Force every printf to flush immediately and also mirror to a log
	// file next to the executable so we can diagnose runs that finish
	// (or hang) before any pipe-buffered output is visible.
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	g_logFile = fopen("Example50.log", "w");
	if (g_logFile) setvbuf(g_logFile, NULL, _IONBF, 0);
	LOGF("Example50 starting...\n");
	LOGF("  glTF      : %s\n", g_gltfPath);
	LOGF("  panorama  : %s\n", g_panoramaPath);

	if (!glewExperimental) glewExperimental = GL_TRUE;

	// Opt into per-monitor DPI v2 awareness so the window we ask Windows for
	// (SCREEN_WIDTH x SCREEN_HEIGHT logical pixels) is created at that exact
	// physical-pixel size on high-DPI displays, instead of being DPI-virtualised
	// (which would silently inflate the EGL/Vulkan swapchain to the monitor's
	// physical resolution - e.g. 3840x2160 on a 4K display at 300% scaling).
	// Resolved dynamically so the binary still loads on older Windows versions.
	{
		HMODULE user32 = GetModuleHandleW(L"user32.dll");
		if (user32)
		{
			typedef BOOL (WINAPI *PFN_SetProcessDpiAwarenessContext)(HANDLE);
			PFN_SetProcessDpiAwarenessContext pSetCtx =
				(PFN_SetProcessDpiAwarenessContext)GetProcAddress(user32, "SetProcessDpiAwarenessContext");
			if (pSetCtx)
			{
				/* DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = ((HANDLE)-4) */
				if (!pSetCtx((HANDLE)-4))
				{
					/* Fall back to v1 per-monitor awareness if v2 isn't available. */
					pSetCtx((HANDLE)-3);
				}
			}
			else
			{
				/* Pre-1703 Windows 10: best-effort, system-DPI awareness only. */
				typedef BOOL (WINAPI *PFN_SetProcessDPIAware)(void);
				PFN_SetProcessDPIAware pAware =
					(PFN_SetProcessDPIAware)GetProcAddress(user32, "SetProcessDPIAware");
				if (pAware) pAware();
			}
		}
	}

	g_hwnd = createNativeWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
	                            "Example 50 - Rec.2020 PBR + IBL + HDR EGL");
	if (!g_hwnd) { LOGF("createNativeWindow failed (GetLastError=%lu)\n", GetLastError()); return 1; }
	g_hdc = GetDC(g_hwnd);
	LOGF("Native window created: HWND=%p HDC=%p\n", (void*)g_hwnd, (void*)g_hdc);

	// Initialize EGL using McNopper/EGL.  Their own examples use
	// EGL_DEFAULT_DISPLAY, so try that first; HDC is a documented but
	// less-tested path in the implementation.
	EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (dpy == EGL_NO_DISPLAY)
		dpy = eglGetDisplay((EGLNativeDisplayType)g_hdc);
	if (dpy == EGL_NO_DISPLAY) { LOGF("eglGetDisplay failed: 0x%x\n", eglGetError()); return 1; }
	LOGF("eglGetDisplay -> %p\n", (void*)dpy);

	EGLint eglMajor = 0, eglMinor = 0;
	if (!eglInitialize(dpy, &eglMajor, &eglMinor))
	{
		LOGF("eglInitialize failed: 0x%x\n", eglGetError());
		return 1;
	}
	LOGF("EGL %d.%d initialized via McNopper/EGL\n", eglMajor, eglMinor);
	LOGF("EGL_VENDOR     : %s\n", eglQueryString(dpy, EGL_VENDOR));
	LOGF("EGL_VERSION    : %s\n", eglQueryString(dpy, EGL_VERSION));
	LOGF("EGL_CLIENT_APIS: %s\n", eglQueryString(dpy, EGL_CLIENT_APIS));

	const char* eglExts = eglQueryString(dpy, EGL_EXTENSIONS);
	LOGF("EGL_EXTENSIONS : %s\n", eglExts ? eglExts : "(null)");

	// Per-extension diagnostic: list which colorspace EXT we recognise.
	for (int i = 0; i < g_numCandidates; ++i)
	{
		const OutputColorspaceCandidate* c = &g_candidates[i];
		if (c->extension == NULL) continue;
		LOGF("  %-44s : %s\n", c->extension,
			extensionPresent(eglExts, c->extension) ? "supported" : "absent");
	}

	if (!eglBindAPI(EGL_OPENGL_API))
	{
		LOGF("eglBindAPI(EGL_OPENGL_API) failed: 0x%x\n", eglGetError());
		return 1;
	}

	// Pick the best supported output color space (informational only;
	// the surface-creation loop below confirms what actually works).
	const OutputColorspaceCandidate* chosen = NULL;
	for (int i = 0; i < g_numCandidates; ++i)
	{
		const OutputColorspaceCandidate* c = &g_candidates[i];
		if (c->extension == NULL || extensionPresent(eglExts, c->extension))
		{
			chosen = c;
			break;
		}
	}
	if (!chosen) chosen = &g_candidates[g_numCandidates - 1];
	g_outputColorspace = chosen->id;
	LOGF("Preferred output colorspace: %s\n", chosen->name);

	// Build EGL config + surface for the chosen color space.  If anything
	// fails (e.g. driver advertises the extension but the surface format
	// still won't validate) walk down the candidate list.
	EGLConfig  cfg     = NULL;
	EGLSurface surface = EGL_NO_SURFACE;
	for (int i = 0; i < g_numCandidates && surface == EGL_NO_SURFACE; ++i)
	{
		const OutputColorspaceCandidate* c = &g_candidates[i];
		if (c->extension && !extensionPresent(eglExts, c->extension))
		{
			LOGF("  [%-22s] skipped (extension absent)\n", c->name);
			continue;
		}

		EGLint cfgAttribs[] = {
			EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
			EGL_RED_SIZE,        c->redSize,
			EGL_GREEN_SIZE,      c->greenSize,
			EGL_BLUE_SIZE,       c->blueSize,
			EGL_ALPHA_SIZE,      c->alphaSize,
			EGL_DEPTH_SIZE,      24,
			EGL_NONE
		};
		EGLint numCfg = 0;
		EGLConfig tryCfg = NULL;
		if (!eglChooseConfig(dpy, cfgAttribs, &tryCfg, 1, &numCfg) || numCfg == 0)
		{
			LOGF("  [%-22s] eglChooseConfig found 0 (err=0x%x)\n", c->name, eglGetError());
			continue;
		}

		EGLint surfAttribs[] = {
			EGL_GL_COLORSPACE, c->colorspace,
			EGL_NONE
		};
		EGLSurface trySurf = eglCreateWindowSurface(dpy, tryCfg,
			(EGLNativeWindowType)g_hwnd, surfAttribs);
		if (trySurf == EGL_NO_SURFACE)
		{
			LOGF("  [%-22s] eglCreateWindowSurface failed (err=0x%x)\n", c->name, eglGetError());
			continue;
		}

		cfg     = tryCfg;
		surface = trySurf;
		chosen  = c;
		g_outputColorspace = c->id;
		LOGF("  [%-22s] surface created OK <-- using this\n", c->name);
	}
	if (surface == EGL_NO_SURFACE)
	{
		LOGF("Could not create any EGL window surface (last error 0x%x)\n", eglGetError());
		return 1;
	}

	// HDR10 mastering metadata (only meaningful for BT.2020 PQ; harmless
	// otherwise - eglSurfaceAttrib will silently ignore unsupported attribs).
	if (g_outputColorspace == OUT_BT2020_PQ)
	{
		const struct { EGLint a; EGLint v; } md[] = {
			{ EGL_SMPTE2086_DISPLAY_PRIMARY_RX_EXT, (EGLint)(0.708f  * 50000) },
			{ EGL_SMPTE2086_DISPLAY_PRIMARY_RY_EXT, (EGLint)(0.292f  * 50000) },
			{ EGL_SMPTE2086_DISPLAY_PRIMARY_GX_EXT, (EGLint)(0.170f  * 50000) },
			{ EGL_SMPTE2086_DISPLAY_PRIMARY_GY_EXT, (EGLint)(0.797f  * 50000) },
			{ EGL_SMPTE2086_DISPLAY_PRIMARY_BX_EXT, (EGLint)(0.131f  * 50000) },
			{ EGL_SMPTE2086_DISPLAY_PRIMARY_BY_EXT, (EGLint)(0.046f  * 50000) },
			{ EGL_SMPTE2086_WHITE_POINT_X_EXT,      (EGLint)(0.3127f * 50000) },
			{ EGL_SMPTE2086_WHITE_POINT_Y_EXT,      (EGLint)(0.3290f * 50000) },
			{ EGL_SMPTE2086_MAX_LUMINANCE_EXT,      (EGLint)(g_peakNits * 10000.0f) },
			{ EGL_SMPTE2086_MIN_LUMINANCE_EXT,      (EGLint)(0.001f  * 10000) },
		};
		for (size_t i = 0; i < sizeof(md) / sizeof(md[0]); ++i)
			eglSurfaceAttrib(dpy, surface, md[i].a, md[i].v);
	}

	const EGLint ctxAttribs[] = {
		EGL_CONTEXT_MAJOR_VERSION,             4,
		EGL_CONTEXT_MINOR_VERSION,             6,
		EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
		EGL_CONTEXT_OPENGL_PROFILE_MASK,       EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
		EGL_NONE
	};
	EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ctxAttribs);
	if (ctx == EGL_NO_CONTEXT)
	{
		LOGF("eglCreateContext failed: 0x%x\n", eglGetError());
		return 1;
	}
	if (!eglMakeCurrent(dpy, surface, surface, ctx))
	{
		LOGF("eglMakeCurrent failed: 0x%x\n", eglGetError());
		return 1;
	}
	LOGF("EGL context current.\n");

	GLenum glewErr = glewInit();
	if (glewErr != GLEW_OK)
	{
		LOGF("glewInit failed: %s\n", glewGetErrorString(glewErr));
		return 1;
	}
	LOGF("GL_VENDOR  : %s\n", (const char*)glGetString(GL_VENDOR));
	LOGF("GL_RENDERER: %s\n", (const char*)glGetString(GL_RENDERER));
	LOGF("GL_VERSION : %s\n", (const char*)glGetString(GL_VERSION));

	// Make the window visible only after we have a working context, so the
	// user doesn't see a black flash if surface creation fails.
	ShowWindow(g_hwnd, SW_SHOW);
	UpdateWindow(g_hwnd);

	g_runWidth  = SCREEN_WIDTH;
	g_runHeight = SCREEN_HEIGHT;
	LOGF("Calling init() ...\n");
	if (!init())
	{
		LOGF("init() failed\n");
		return 1;
	}
	LOGF("init() OK; entering main loop.\n");
	g_glReady = 1;
	reshape(g_runWidth, g_runHeight);

	g_lastTimeSeconds = winNowSeconds();

	while (!g_shouldClose)
	{
		MSG msg;
		while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		if (g_shouldClose) break;

		double now = winNowSeconds();
		float  dt  = (float)(now - g_lastTimeSeconds);
		g_lastTimeSeconds = now;

		if (!update(dt)) break;
		eglSwapBuffers(dpy, surface);
	}

	terminate();
	eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglDestroyContext(dpy, ctx);
	eglDestroySurface(dpy, surface);
	eglTerminate(dpy);

	if (g_hdc) ReleaseDC(g_hwnd, g_hdc);
	DestroyWindow(g_hwnd);
	UnregisterClassW(WIN32_CLASS_NAME, GetModuleHandleW(NULL));

	return 0;
#endif
}
