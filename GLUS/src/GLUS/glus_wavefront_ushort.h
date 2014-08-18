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

#ifndef GLUS_WAVEFRONT_USHORT_H_
#define GLUS_WAVEFRONT_USHORT_H_

#include "glus_wavefront_material.h"

/**
 * Group of geometry.
 */
typedef struct _GLUSgroup
{
		/**
		 * Name of the group.
		 */
		GLUSchar name[GLUS_MAX_STRING];

		/**
		 * Name of the material.
		 */
		GLUSchar materialName[GLUS_MAX_STRING];

		/**
		 * Pointer to the material.
		 */
		GLUSmaterial* material;

	    /**
	     * Indices.
	     */
	    GLUSushort* indices;

	    /**
	     * Indices VBO.
	     */
	    GLUSuint indicesVBO;

	    /**
	     * Number of indices.
	     */
	    GLUSuint numberIndices;

	    /**
	     * Triangle render mode - could be either:
	     *
	     * GL_TRIANGLES
	     * GL_TRIANGLE_STRIP
	     */
	    GLUSenum mode;

} GLUSgroup;

/**
 * Structure for holding the group data list.
 */
typedef struct _GLUSgroupList
{
		/**
		 * The group data.
		 */
		GLUSgroup group;

	    /**
	     * The pointer to the next group element.
	     */
	    struct _GLUSgroupList* next;

} GLUSgroupList;

/**
 * Structure for a complete wavefront object file.
 */
typedef struct _GLUSwavefront
{
		/**
		 * Vertices in homogeneous coordinates.
		 */
	    GLUSfloat* vertices;

	    /**
	     * Vertices VBO.
	     */
	    GLUSuint verticesVBO;

	    /**
	     * Normals.
	     */
	    GLUSfloat* normals;

	    /**
	     * Normals VBO.
	     */
	    GLUSuint normalsVBO;

	    /**
	     * Tangents.
	     */
	    GLUSfloat* tangents;

	    /**
	     * Tangents VBO.
	     */
	    GLUSuint tangentsVBO;

	    /**
	     * Bitangents.
	     */
	    GLUSfloat* bitangents;

	    /**
	     * Bitangents VBO.
	     */
	    GLUSuint bitangentsVBO;

	    /**
	     * Texture coordinates.
	     */
	    GLUSfloat* texCoords;

	    /**
	     * Texture corrdinates VBO.
	     */
	    GLUSuint texCoordsVBO;

	    /**
	     * Number of vertices.
	     */
	    GLUSuint numberVertices;

	    /**
	     * Pointer to the first element of the groups.
	     */
	    GLUSgroupList* groups;

	    /**
	     * Pointer to the first element of the materials.
	     */
	    GLUSmaterialList* materials;

} GLUSwavefront;

#endif /* GLUS_WAVEFRONT_USHORT_H_ */
