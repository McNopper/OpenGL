/*
 * GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities. Copyright (C) 2010 - 2014 Norbert Nopper
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

#ifndef GLUS_WAVEFRONT_MATERIAL_H_
#define GLUS_WAVEFRONT_MATERIAL_H_

/**
 * Structure for holding material data.
 */
typedef struct _GLUSmaterial
{
	/**
	 * Name of the material.
	 */
	GLUSchar name[GLUS_MAX_STRING];

	/**
	 * Emissive color.
	 */
	GLUSfloat emissive[4];

	/**
	 * Ambient color.
	 */
	GLUSfloat ambient[4];

	/**
	 * Diffuse color.
	 */
	GLUSfloat diffuse[4];

	/**
	 * Specular color.
	 */
	GLUSfloat specular[4];

	/**
	 * Shininess.
	 */
	GLUSfloat shininess;

	/**
	 * Transparency, which is the alpha value.
	 */
	GLUSfloat transparency;

	/**
	 * Reflection.
	 */
	GLUSboolean reflection;

	/**
	 * Refraction.
	 */
	GLUSboolean refraction;

	/**
	 * Index of refraction.
	 */
	GLUSfloat indexOfRefraction;

	/**
	 * Ambient color texture filename.
	 */
	GLUSchar ambientTextureFilename[GLUS_MAX_STRING];

	/**
	 * Diffuse color texture filename.
	 */
	GLUSchar diffuseTextureFilename[GLUS_MAX_STRING];

	/**
	 * Specular color texture filename.
	 */
	GLUSchar specularTextureFilename[GLUS_MAX_STRING];

	/**
	 * Transparency texture filename.
	 */
	GLUSchar transparencyTextureFilename[GLUS_MAX_STRING];

	/**
	 * Bump texture filename.
	 */
	GLUSchar bumpTextureFilename[GLUS_MAX_STRING];

	/**
	 * Can be used to store the ambient texture name.
	 */
    GLUSuint ambientTextureName;

	/**
	 * Can be used to store the diffuse texture name.
	 */
    GLUSuint diffuseTextureName;

	/**
	 * Can be used to store the specular texture name.
	 */
    GLUSuint specularTextureName;

	/**
	 * Can be used to store the transparency texture name.
	 */
    GLUSuint transparencyTextureName;

	/**
	 * Can be used to store the bump texture name.
	 */
    GLUSuint bumpTextureName;

} GLUSmaterial;

/**
 * Structure for holding material data list.
 */
typedef struct _GLUSmaterialList
{
	/**
	 * The material data.
	 */
	GLUSmaterial material;

	/**
	 * The pointer to the next element.
	 */
	struct _GLUSmaterialList* next;

} GLUSmaterialList;

#endif /* GLUS_WAVEFRONT_MATERIAL_H_ */
