/*
 * GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities. Copyright (C) since 2010 Norbert Nopper
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

#include "GL/glus.h"

#define GLUS_MAX_OBJECTS 1
#define GLUS_MAX_ATTRIBUTES (GLUS_MAX_VERTICES/GLUS_VERTICES_DIVISOR)
#define GLUS_MAX_TRIANGLE_ATTRIBUTES GLUS_MAX_VERTICES
#define GLUS_BUFFERSIZE 1024

static GLUSboolean glusWavefrontMallocTempMemory(GLUSfloat** vertices, GLUSfloat** normals, GLUSfloat** texCoords, GLUSfloat** triangleVertices, GLUSfloat** triangleNormals, GLUSfloat** triangleTexCoords)
{
	if (!vertices || !normals || !texCoords || !triangleVertices || !triangleNormals || !triangleTexCoords)
	{
		return GLUS_FALSE;
	}

	*vertices = (GLUSfloat*)glusMemoryMalloc(4 * GLUS_MAX_ATTRIBUTES * sizeof(GLUSfloat));
	if (!*vertices)
	{
		return GLUS_FALSE;
	}

	*normals = (GLUSfloat*)glusMemoryMalloc(3 * GLUS_MAX_ATTRIBUTES * sizeof(GLUSfloat));
	if (!*normals)
	{
		return GLUS_FALSE;
	}

	*texCoords = (GLUSfloat*)glusMemoryMalloc(2 * GLUS_MAX_ATTRIBUTES * sizeof(GLUSfloat));
	if (!*texCoords)
	{
		return GLUS_FALSE;
	}

	*triangleVertices = (GLUSfloat*)glusMemoryMalloc(4 * GLUS_MAX_TRIANGLE_ATTRIBUTES * sizeof(GLUSfloat));
	if (!*triangleVertices)
	{
		return GLUS_FALSE;
	}

	*triangleNormals = (GLUSfloat*)glusMemoryMalloc(3 * GLUS_MAX_TRIANGLE_ATTRIBUTES * sizeof(GLUSfloat));
	if (!*triangleNormals)
	{
		return GLUS_FALSE;
	}

	*triangleTexCoords = (GLUSfloat*)glusMemoryMalloc(2 * GLUS_MAX_TRIANGLE_ATTRIBUTES * sizeof(GLUSfloat));
	if (!*triangleTexCoords)
	{
		return GLUS_FALSE;
	}

	return GLUS_TRUE;
}

static GLUSvoid glusWavefrontFreeTempMemory(GLUSfloat** vertices, GLUSfloat** normals, GLUSfloat** texCoords, GLUSfloat** triangleVertices, GLUSfloat** triangleNormals, GLUSfloat** triangleTexCoords)
{
	if (vertices && *vertices)
	{
		glusMemoryFree(*vertices);

		*vertices = 0;
	}

	if (normals && *normals)
	{
		glusMemoryFree(*normals);

		*normals = 0;
	}

	if (texCoords && *texCoords)
	{
		glusMemoryFree(*texCoords);

		*texCoords = 0;
	}

	if (triangleVertices && *triangleVertices)
	{
		glusMemoryFree(*triangleVertices);

		*triangleVertices = 0;
	}

	if (triangleNormals && *triangleNormals)
	{
		glusMemoryFree(*triangleNormals);

		*triangleNormals = 0;
	}

	if (triangleTexCoords && *triangleTexCoords)
	{
		glusMemoryFree(*triangleTexCoords);

		*triangleTexCoords = 0;
	}
}

static GLUSvoid glusWavefrontInitMaterial(GLUSmaterial* material)
{
	if (!material)
	{
		return;
	}

	material->name[0] = 0;

	material->emissive[0] = 0.0f;
	material->emissive[1] = 0.0f;
	material->emissive[2] = 0.0f;
	material->emissive[3] = 1.0f;

	material->ambient[0] = 0.0f;
	material->ambient[1] = 0.0f;
	material->ambient[2] = 0.0f;
	material->ambient[3] = 1.0f;

	material->diffuse[0] = 0.0f;
	material->diffuse[1] = 0.0f;
	material->diffuse[2] = 0.0f;
	material->diffuse[3] = 1.0f;

	material->specular[0] = 0.0f;
	material->specular[1] = 0.0f;
	material->specular[2] = 0.0f;
	material->specular[3] = 1.0f;

	material->shininess = 0.0f;

	material->transparency = 1.0f;

	material->reflection = GLUS_FALSE;

	material->refraction = GLUS_FALSE;

	material->indexOfRefraction = 1.0f;

	material->ambientTextureFilename[0] = 0;

	material->diffuseTextureFilename[0] = 0;

	material->specularTextureFilename[0] = 0;

	material->transparencyTextureFilename[0] = 0;

	material->bumpTextureFilename[0] = 0;

	material->ambientTextureName = 0;

	material->diffuseTextureName = 0;

	material->specularTextureName = 0;

	material->transparencyTextureName = 0;

	material->bumpTextureName = 0;
}

static GLUSvoid glusWavefrontDestroyMaterial(GLUSmaterialList** materialList)
{
	GLUSmaterialList* currentMaterialList = 0;
	GLUSmaterialList* nextMaterialList = 0;

	if (!materialList)
	{
		return;
	}

	currentMaterialList = *materialList;
	while (currentMaterialList != 0)
	{
		nextMaterialList = currentMaterialList->next;

		memset(&currentMaterialList->material, 0, sizeof(GLUSmaterial));

		glusMemoryFree(currentMaterialList);

		currentMaterialList = nextMaterialList;
	}

	*materialList = 0;
}

static GLUSboolean glusWavefrontLoadMaterial(const GLUSchar* filename, GLUSmaterialList** materialList)
{
	FILE* f;

	GLUSint i, k;

	GLUSchar buffer[GLUS_BUFFERSIZE];
	GLUSchar* checkBuffer;
	GLUSchar name[GLUS_MAX_STRING];
	GLUSchar identifier[7];

	GLUSmaterialList* currentMaterialList = 0;

	if (!filename || !materialList)
	{
		return GLUS_FALSE;
	}

	f = fopen(filename, "r");

	if (!f)
	{
		return GLUS_FALSE;
	}

	while (!feof(f))
	{
		if (fgets(buffer, GLUS_BUFFERSIZE, f) == 0)
		{
			if (ferror(f))
			{
				fclose(f);

				return GLUS_FALSE;
			}
		}

		checkBuffer = buffer;

		k = 0;

		// Skip first spaces etc.
		while (*checkBuffer)
		{
			if (*checkBuffer != ' ' && *checkBuffer != '\t')
			{
				break;
			}

			checkBuffer++;
			k++;

			if (k >= GLUS_BUFFERSIZE)
			{
				fclose(f);

				return GLUS_FALSE;
			}
		}

		i = 0;

		while (checkBuffer[i])
		{
			if (checkBuffer[i] == ' ' || checkBuffer[i] == '\t')
			{
				break;
			}

			checkBuffer[i] = tolower(checkBuffer[i]);

			i++;

			if (i >= GLUS_BUFFERSIZE - k)
			{
				fclose(f);

				return GLUS_FALSE;
			}
		}

		if (strncmp(checkBuffer, "newmtl", 6) == 0)
		{
			GLUSmaterialList* newMaterialList = 0;

			sscanf(checkBuffer, "%s %s", identifier, name);

			newMaterialList = (GLUSmaterialList*)glusMemoryMalloc(sizeof(GLUSmaterialList));

			if (!newMaterialList)
			{
				glusWavefrontDestroyMaterial(materialList);

				fclose(f);

				return GLUS_FALSE;
			}

			memset(newMaterialList, 0, sizeof(GLUSmaterialList));

			glusWavefrontInitMaterial(&newMaterialList->material);

			strcpy(newMaterialList->material.name, name);

			if (*materialList == 0)
			{
				*materialList = newMaterialList;
			}
			else
			{
				currentMaterialList->next = newMaterialList;
			}

			currentMaterialList = newMaterialList;
		}
		else if (strncmp(checkBuffer, "ke", 2) == 0)
		{
			sscanf(checkBuffer, "%s %f %f %f", identifier, &currentMaterialList->material.emissive[0], &currentMaterialList->material.emissive[1], &currentMaterialList->material.emissive[2]);

			currentMaterialList->material.emissive[3] = 1.0f;
		}
		else if (strncmp(checkBuffer, "ka", 2) == 0)
		{
			sscanf(checkBuffer, "%s %f %f %f", identifier, &currentMaterialList->material.ambient[0], &currentMaterialList->material.ambient[1], &currentMaterialList->material.ambient[2]);

			currentMaterialList->material.ambient[3] = 1.0f;
		}
		else if (strncmp(checkBuffer, "kd", 2) == 0)
		{
			sscanf(checkBuffer, "%s %f %f %f", identifier, &currentMaterialList->material.diffuse[0], &currentMaterialList->material.diffuse[1], &currentMaterialList->material.diffuse[2]);

			currentMaterialList->material.diffuse[3] = 1.0f;
		}
		else if (strncmp(checkBuffer, "ks", 2) == 0)
		{
			sscanf(checkBuffer, "%s %f %f %f", identifier, &currentMaterialList->material.specular[0], &currentMaterialList->material.specular[1], &currentMaterialList->material.specular[2]);

			currentMaterialList->material.specular[3] = 1.0f;
		}
		else if (strncmp(checkBuffer, "ns", 2) == 0)
		{
			sscanf(checkBuffer, "%s %f", identifier, &currentMaterialList->material.shininess);
		}
		else if (strncmp(checkBuffer, "d", 1) == 0 || strncmp(checkBuffer, "Tr", 2) == 0)
		{
			sscanf(checkBuffer, "%s %f", identifier, &currentMaterialList->material.transparency);
		}
		else if (strncmp(checkBuffer, "ni", 2) == 0)
		{
			sscanf(checkBuffer, "%s %f", identifier, &currentMaterialList->material.indexOfRefraction);
		}
		else if (strncmp(checkBuffer, "map_ka", 6) == 0)
		{
			sscanf(checkBuffer, "%s %s", identifier, name);

			strcpy(currentMaterialList->material.ambientTextureFilename, name);
		}
		else if (strncmp(checkBuffer, "map_kd", 6) == 0)
		{
			sscanf(checkBuffer, "%s %s", identifier, name);

			strcpy(currentMaterialList->material.diffuseTextureFilename, name);
		}
		else if (strncmp(checkBuffer, "map_ks", 6) == 0)
		{
			sscanf(checkBuffer, "%s %s", identifier, name);

			strcpy(currentMaterialList->material.specularTextureFilename, name);
		}
		else if (strncmp(checkBuffer, "map_d", 5) == 0 || strncmp(checkBuffer, "map_Tr", 6) == 0)
		{
			sscanf(checkBuffer, "%s %s", identifier, name);

			strcpy(currentMaterialList->material.transparencyTextureFilename, name);
		}
		else if (strncmp(checkBuffer, "map_bump", 8) == 0 || strncmp(checkBuffer, "bump", 4) == 0)
		{
			sscanf(checkBuffer, "%s %s", identifier, name);

			strcpy(currentMaterialList->material.bumpTextureFilename, name);
		}
		else if (strncmp(checkBuffer, "illum", 5) == 0)
		{
			GLUSint illum;

			sscanf(checkBuffer, "%s %d", identifier, &illum);

			// Only setting reflection and refraction depending on illumination model.
			switch (illum)
			{
				case 3:
				case 4:
				case 5:
				case 8:
				case 9:
					currentMaterialList->material.reflection = GLUS_TRUE;
					break;
				case 6:
				case 7:
					currentMaterialList->material.reflection = GLUS_TRUE;
					currentMaterialList->material.refraction = GLUS_TRUE;
					break;
			}
		}
	}

	fclose(f);

	return GLUS_TRUE;
}

static GLUSvoid glusWavefrontDestroyGroup(GLUSgroupList** groupList)
{
	GLUSgroupList* currentGroupList = 0;
	GLUSgroupList* nextGroupList = 0;

	if (!groupList)
	{
		return;
	}

	currentGroupList = *groupList;
	while (currentGroupList != 0)
	{
		nextGroupList = currentGroupList->next;

		if (currentGroupList->group.indices)
		{
			glusMemoryFree(currentGroupList->group.indices);
		}
		memset(&currentGroupList->group, 0, sizeof(GLUSgroup));

		glusMemoryFree(currentGroupList);

		currentGroupList = nextGroupList;
	}

	*groupList = 0;
}

static GLUSboolean glusWavefrontCopyData(GLUSshape* shape, GLUSuint totalNumberVertices, GLUSfloat* triangleVertices, GLUSuint totalNumberNormals, GLUSfloat* triangleNormals, GLUSuint totalNumberTexCoords, GLUSfloat* triangleTexCoords)
{
	GLUSuint indicesCounter = 0;

	if (!shape || !triangleVertices || !triangleNormals || !triangleTexCoords)
	{
		return GLUS_FALSE;
	}

	shape->numberVertices = totalNumberVertices;

	if (totalNumberVertices > 0)
	{
		shape->vertices = (GLUSfloat*)glusMemoryMalloc(totalNumberVertices * 4 * sizeof(GLUSfloat));

		if (shape->vertices == 0)
		{
			glusShapeDestroyf(shape);

			return GLUS_FALSE;
		}

		memcpy(shape->vertices, triangleVertices, totalNumberVertices * 4 * sizeof(GLUSfloat));
	}
	if (totalNumberNormals > 0)
	{
		shape->normals = (GLUSfloat*)glusMemoryMalloc(totalNumberNormals * 3 * sizeof(GLUSfloat));

		if (shape->normals == 0)
		{
			glusShapeDestroyf(shape);

			return GLUS_FALSE;
		}

		memcpy(shape->normals, triangleNormals, totalNumberNormals * 3 * sizeof(GLUSfloat));
	}
	if (totalNumberTexCoords > 0)
	{
		shape->texCoords = (GLUSfloat*)glusMemoryMalloc(totalNumberTexCoords * 2 * sizeof(GLUSfloat));

		if (shape->texCoords == 0)
		{
			glusShapeDestroyf(shape);

			return GLUS_FALSE;
		}

		memcpy(shape->texCoords, triangleTexCoords, totalNumberTexCoords * 2 * sizeof(GLUSfloat));
	}

	// Just create the indices from the list of vertices.

	shape->numberIndices = totalNumberVertices;

	if (totalNumberVertices > 0)
	{
		shape->indices = (GLUSindex*)glusMemoryMalloc(totalNumberVertices * sizeof(GLUSindex));

		if (shape->indices == 0)
		{
			glusShapeDestroyf(shape);

			return GLUS_FALSE;
		}

		for (indicesCounter = 0; indicesCounter < totalNumberVertices; indicesCounter++)
		{
			shape->indices[indicesCounter] = indicesCounter;
		}
	}

	shape->mode = GLUS_TRIANGLES;

	return GLUS_TRUE;
}

GLUSboolean _glusWavefrontParse(const GLUSchar* filename, GLUSshape* shape, GLUSwavefront* wavefront)
{
	GLUSboolean result;

	FILE* f;

	GLUSchar buffer[GLUS_BUFFERSIZE];
	GLUSchar identifier[7];

	GLUSfloat x, y, z;
	GLUSfloat s, t;

	GLUSfloat* vertices = 0;
	GLUSfloat* normals = 0;
	GLUSfloat* texCoords = 0;

	GLUSuint numberVertices = 0;
	GLUSuint numberNormals = 0;
	GLUSuint numberTexCoords = 0;

	GLUSfloat* triangleVertices = 0;
	GLUSfloat* triangleNormals = 0;
	GLUSfloat* triangleTexCoords = 0;

	GLUSuint totalNumberVertices = 0;
	GLUSuint totalNumberNormals = 0;
	GLUSuint totalNumberTexCoords = 0;

	GLUSuint facesEncoding = 0;

	// Material and groups

	GLUSchar name[GLUS_MAX_STRING];

	GLUSuint numberIndicesGroup = 0;
	GLUSuint numberMaterials = 0;
	GLUSuint numberGroups = 0;

	GLUSgroupList* currentGroupList = 0;

	// Objects

	GLUSuint numberObjects = 0;

	if (!filename || !shape)
	{
		return GLUS_FALSE;
	}

	memset(shape, 0, sizeof(GLUSshape));

	f = fopen(filename, "r");

	if (!f)
	{
		return GLUS_FALSE;
	}

	if (!glusWavefrontMallocTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords))
	{
		glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

		fclose(f);

		return GLUS_FALSE;
	}

	while (!feof(f))
	{
		if (fgets(buffer, GLUS_BUFFERSIZE, f) == 0)
		{
			if (ferror(f))
			{
				glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

				fclose(f);

				return GLUS_FALSE;
			}
		}

		if (wavefront)
		{
			if (strncmp(buffer, "mtllib", 6) == 0)
			{
				sscanf(buffer, "%s %s", identifier, name);

				if (numberMaterials == 0)
				{
					wavefront->materials = 0;
				}

				if (!glusWavefrontLoadMaterial(name, &wavefront->materials))
				{
					glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

					fclose(f);

					return GLUS_FALSE;
				}

				numberMaterials++;
			}
			else if (strncmp(buffer, "usemtl", 6) == 0)
			{
				if (!currentGroupList || currentGroupList->group.materialName[0] != '\0')
				{
					GLUSgroupList* newGroupList;

					newGroupList = (GLUSgroupList*)glusMemoryMalloc(sizeof(GLUSgroupList));

					if (!newGroupList)
					{
						glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

						fclose(f);

						return GLUS_FALSE;
					}

					memset(newGroupList, 0, sizeof(GLUSgroupList));

					strcpy(newGroupList->group.name, name);

					if (numberGroups == 0)
					{
						wavefront->groups = newGroupList;
					}
					else
					{
						if (!currentGroupList)
						{
							glusMemoryFree(newGroupList);

							glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

							fclose(f);

							return GLUS_FALSE;
						}

						currentGroupList->next = newGroupList;

						currentGroupList->group.numberIndices = numberIndicesGroup;
						numberIndicesGroup = 0;
					}

					currentGroupList = newGroupList;

					numberGroups++;
				}

				//

				sscanf(buffer, "%s %s", identifier, name);

				strcpy(currentGroupList->group.materialName, name);
			}
			else if (strncmp(buffer, "g", 1) == 0)
			{
				GLUSgroupList* newGroupList;

				sscanf(buffer, "%s %s", identifier, name);

				newGroupList = (GLUSgroupList*)glusMemoryMalloc(sizeof(GLUSgroupList));

				if (!newGroupList)
				{
					glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

					fclose(f);

					return GLUS_FALSE;
				}

				memset(newGroupList, 0, sizeof(GLUSgroupList));

				strcpy(newGroupList->group.name, name);

				if (numberGroups == 0)
				{
					wavefront->groups = newGroupList;
				}
				else
				{
					if (!currentGroupList)
					{
						glusMemoryFree(newGroupList);

						glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

						fclose(f);

						return GLUS_FALSE;
					}

					currentGroupList->next = newGroupList;

					currentGroupList->group.numberIndices = numberIndicesGroup;
					numberIndicesGroup = 0;
				}

				currentGroupList = newGroupList;

				numberGroups++;
			}
		}

		if (strncmp(buffer, "o", 1) == 0)
		{
			if (numberObjects == GLUS_MAX_OBJECTS)
			{
				glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

				fclose(f);

				return GLUS_FALSE;
			}

			numberObjects++;
		}
		else if (strncmp(buffer, "vt", 2) == 0)
		{
			if (numberTexCoords == GLUS_MAX_ATTRIBUTES)
			{
				glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

				fclose(f);

				return GLUS_FALSE;
			}

			sscanf(buffer, "%s %f %f", identifier, &s, &t);

			texCoords[2 * numberTexCoords + 0] = s;
			texCoords[2 * numberTexCoords + 1] = t;

			numberTexCoords++;
		}
		else if (strncmp(buffer, "vn", 2) == 0)
		{
			if (numberNormals == GLUS_MAX_ATTRIBUTES)
			{
				glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

				fclose(f);

				return GLUS_FALSE;
			}

			sscanf(buffer, "%s %f %f %f", identifier, &x, &y, &z);

			normals[3 * numberNormals + 0] = x;
			normals[3 * numberNormals + 1] = y;
			normals[3 * numberNormals + 2] = z;

			numberNormals++;
		}
		else if (strncmp(buffer, "v", 1) == 0)
		{
			if (numberVertices == GLUS_MAX_ATTRIBUTES)
			{
				glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

				fclose(f);

				return GLUS_FALSE;
			}

			sscanf(buffer, "%s %f %f %f", identifier, &x, &y, &z);

			vertices[4 * numberVertices + 0] = x;
			vertices[4 * numberVertices + 1] = y;
			vertices[4 * numberVertices + 2] = z;
			vertices[4 * numberVertices + 3] = 1.0f;

			numberVertices++;
		}
		else if (strncmp(buffer, "f", 1) == 0)
		{
			GLUSchar* token;

			GLUSint vIndex, vtIndex, vnIndex;

			GLUSuint edgeCount = 0;

			token = strtok(buffer, " \t");
			token = strtok(0, " \n");

			if (!token)
			{
				continue;
			}

			// Check faces
			if (strstr(token, "//") != 0)
			{
				facesEncoding = 2;
			}
			else if (strstr(token, "/") == 0)
			{
				facesEncoding = 0;
			}
			else if (strstr(token, "/") != 0)
			{
				GLUSchar* c = strstr(token, "/");

				c++;

				if (!c)
				{
					continue;
				}

				if (strstr(c, "/") == 0)
				{
					facesEncoding = 1;
				}
				else
				{
					facesEncoding = 3;
				}
			}

			while (token != 0)
			{
				vIndex = -1;
				vtIndex = -1;
				vnIndex = -1;

				switch (facesEncoding)
				{
					case 0:
						sscanf(token, "%d", &vIndex);
					break;
					case 1:
						sscanf(token, "%d/%d", &vIndex, &vtIndex);
					break;
					case 2:
						sscanf(token, "%d//%d", &vIndex, &vnIndex);
					break;
					case 3:
						sscanf(token, "%d/%d/%d", &vIndex, &vtIndex, &vnIndex);
					break;
				}

				vIndex--;
				vtIndex--;
				vnIndex--;

				if (vIndex >= 0)
				{
					if (edgeCount < 3)
					{
						if (totalNumberVertices >= GLUS_MAX_TRIANGLE_ATTRIBUTES)
						{
							glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

							fclose(f);

							return GLUS_FALSE;
						}

						memcpy(&triangleVertices[4 * totalNumberVertices], &vertices[4 * vIndex], 4 * sizeof(GLUSfloat));

						totalNumberVertices++;
						numberIndicesGroup++;
					}
					else
					{
						if (totalNumberVertices >= GLUS_MAX_TRIANGLE_ATTRIBUTES - 2)
						{
							glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

							fclose(f);

							return GLUS_FALSE;
						}

						memcpy(&triangleVertices[4 * (totalNumberVertices)], &triangleVertices[4 * (totalNumberVertices - edgeCount)], 4 * sizeof(GLUSfloat));
						memcpy(&triangleVertices[4 * (totalNumberVertices + 1)], &triangleVertices[4 * (totalNumberVertices - 1)], 4 * sizeof(GLUSfloat));
						memcpy(&triangleVertices[4 * (totalNumberVertices + 2)], &vertices[4 * vIndex], 4 * sizeof(GLUSfloat));

						totalNumberVertices += 3;
						numberIndicesGroup +=3;
					}
				}
				if (vnIndex >= 0)
				{
					if (edgeCount < 3)
					{
						if (totalNumberNormals >= GLUS_MAX_TRIANGLE_ATTRIBUTES)
						{
							glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

							fclose(f);

							return GLUS_FALSE;
						}

						memcpy(&triangleNormals[3 * totalNumberNormals], &normals[3 * vnIndex], 3 * sizeof(GLUSfloat));

						totalNumberNormals++;
					}
					else
					{
						if (totalNumberNormals >= GLUS_MAX_TRIANGLE_ATTRIBUTES - 2)
						{
							glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

							fclose(f);

							return GLUS_FALSE;
						}

						memcpy(&triangleNormals[3 * (totalNumberNormals)], &triangleNormals[3 * (totalNumberNormals - edgeCount)], 3 * sizeof(GLUSfloat));
						memcpy(&triangleNormals[3 * (totalNumberNormals + 1)], &triangleNormals[3 * (totalNumberNormals - 1)], 3 * sizeof(GLUSfloat));
						memcpy(&triangleNormals[3 * (totalNumberNormals + 2)], &normals[3 * vnIndex], 3 * sizeof(GLUSfloat));

						totalNumberNormals += 3;
					}
				}
				if (vtIndex >= 0)
				{
					if (edgeCount < 3)
					{
						if (totalNumberTexCoords >= GLUS_MAX_TRIANGLE_ATTRIBUTES)
						{
							glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

							fclose(f);

							return GLUS_FALSE;
						}

						memcpy(&triangleTexCoords[2 * totalNumberTexCoords], &texCoords[2 * vtIndex], 2 * sizeof(GLUSfloat));

						totalNumberTexCoords++;
					}
					else
					{
						if (totalNumberTexCoords >= GLUS_MAX_TRIANGLE_ATTRIBUTES - 2)
						{
							glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

							fclose(f);

							return GLUS_FALSE;
						}

						memcpy(&triangleTexCoords[2 * (totalNumberTexCoords)], &triangleTexCoords[2 * (totalNumberTexCoords - edgeCount)], 2 * sizeof(GLUSfloat));
						memcpy(&triangleTexCoords[2 * (totalNumberTexCoords + 1)], &triangleTexCoords[2 * (totalNumberTexCoords - 1)], 2 * sizeof(GLUSfloat));
						memcpy(&triangleTexCoords[2 * (totalNumberTexCoords + 2)], &texCoords[2 * vtIndex], 2 * sizeof(GLUSfloat));

						totalNumberTexCoords += 3;
					}
				}

				edgeCount++;

				token = strtok(0, " \n");
			}
		}
	}

	fclose(f);

	if (wavefront && currentGroupList)
	{
		currentGroupList->group.numberIndices = numberIndicesGroup;
		numberIndicesGroup = 0;
	}

	result = glusWavefrontCopyData(shape, totalNumberVertices, triangleVertices, totalNumberNormals, triangleNormals, totalNumberTexCoords, triangleTexCoords);

	glusWavefrontFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

	if (result)
	{
		glusShapeCalculateTangentBitangentf(shape);
	}

	return result;
}

//

GLUSboolean GLUSAPIENTRY glusWavefrontLoad(const GLUSchar* filename, GLUSwavefront* wavefront)
{
	GLUSshape dummyShape;

	GLUSmaterialList* materialWalker;
	GLUSgroupList* groupWalker;

	GLUSuint i;
	GLUSuint counter = 0;

	if (!_glusWavefrontParse(filename, &dummyShape, wavefront))
	{
		glusWavefrontDestroy(wavefront);

		return GLUS_FALSE;
	}

	wavefront->vertices = dummyShape.vertices;
	wavefront->normals = dummyShape.normals;
	wavefront->texCoords = dummyShape.texCoords;
	wavefront->tangents = dummyShape.tangents;
	wavefront->bitangents = dummyShape.bitangents;
	wavefront->numberVertices = dummyShape.numberVertices;

	glusMemoryFree(dummyShape.indices);

	groupWalker = wavefront->groups;
	while (groupWalker)
	{
		groupWalker->group.indices = (GLUSindex*)glusMemoryMalloc(groupWalker->group.numberIndices * sizeof(GLUSindex));

		if (!groupWalker->group.indices)
		{
			glusWavefrontDestroy(wavefront);

			return GLUS_FALSE;
		}

		for (i = 0; i < groupWalker->group.numberIndices; i++)
		{
			groupWalker->group.indices[i] = counter++;
		}

		materialWalker = wavefront->materials;

		while (materialWalker)
		{
			if (strcmp(materialWalker->material.name, groupWalker->group.materialName) == 0)
			{
				groupWalker->group.material = &materialWalker->material;

				break;
			}

			materialWalker = materialWalker->next;
		}

		groupWalker = groupWalker->next;
	}

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusWavefrontDestroy(GLUSwavefront* wavefront)
{
	if (!wavefront)
	{
		return;
	}

	glusWavefrontDestroyMaterial(&wavefront->materials);
	glusWavefrontDestroyGroup(&wavefront->groups);

	if (wavefront->vertices)
	{
		glusMemoryFree(wavefront->vertices);

		wavefront->vertices = 0;
	}

	if (wavefront->normals)
	{
		glusMemoryFree(wavefront->normals);

		wavefront->normals = 0;
	}

	if (wavefront->texCoords)
	{
		glusMemoryFree(wavefront->texCoords);

		wavefront->texCoords = 0;
	}

	if (wavefront->tangents)
	{
		glusMemoryFree(wavefront->tangents);

		wavefront->tangents = 0;
	}

	if (wavefront->bitangents)
	{
		glusMemoryFree(wavefront->bitangents);

		wavefront->bitangents = 0;
	}

	memset(wavefront, 0, sizeof(GLUSwavefront));
}
