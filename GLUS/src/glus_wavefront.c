/*
 * GLUS - OpenGL 3 and 4 Utilities. Copyright (C) 2010 - 2013 Norbert Nopper
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

#define GLUS_MAX_ATTRIBUTES (GLUS_MAX_VERTICES/GLUS_VERTICES_DIVISOR)
#define GLUS_MAX_TRIANGLE_ATTRIBUTES GLUS_MAX_VERTICES
#define GLUS_BUFFERSIZE 1024

static GLboolean glusCopyObjData(GLUSshape* shape, GLUSuint totalNumberVertices, GLUSfloat* triangleVertices, GLUSuint totalNumberNormals, GLUSfloat* triangleNormals, GLUSuint totalNumberTexCoords, GLUSfloat* triangleTexCoords)
{
	GLUSuint indicesCounter = 0;

	if (!shape || !triangleVertices || !triangleNormals || !triangleTexCoords)
	{
		return GLUS_FALSE;
	}

    shape->numberVertices = totalNumberVertices;

    if (totalNumberVertices > 0)
    {
        shape->vertices = (GLUSfloat*)malloc(totalNumberVertices*4*sizeof(GLUSfloat));

        if (shape->vertices == 0)
        {
            glusDestroyShapef(shape);

            return GLUS_FALSE;
        }

        memcpy(shape->vertices, triangleVertices, totalNumberVertices*4*sizeof(GLUSfloat));
    }
    if (totalNumberNormals > 0)
    {
        shape->normals = (GLUSfloat*)malloc(totalNumberNormals*3*sizeof(GLUSfloat));

        if (shape->normals == 0)
        {
            glusDestroyShapef(shape);

            return GLUS_FALSE;
        }

        memcpy(shape->normals, triangleNormals, totalNumberNormals*3*sizeof(GLUSfloat));
    }
    if (totalNumberTexCoords > 0)
    {
        shape->texCoords = (GLUSfloat*)malloc(totalNumberTexCoords*2*sizeof(GLUSfloat));

        if (shape->texCoords == 0)
        {
            glusDestroyShapef(shape);

            return GLUS_FALSE;
        }

        memcpy(shape->texCoords, triangleTexCoords, totalNumberTexCoords*2*sizeof(GLUSfloat));
    }

    // Just create the indices from the list of vertices.

    shape->numberIndices = totalNumberVertices;

    if (totalNumberVertices > 0)
    {
        shape->indices = (GLUSuint*)malloc(totalNumberVertices*sizeof(GLUSuint));

        if (shape->indices == 0)
        {
            glusDestroyShapef(shape);

            return GLUS_FALSE;
        }

        for (indicesCounter = 0; indicesCounter < totalNumberVertices; indicesCounter++)
        {
        	shape->indices[indicesCounter] = indicesCounter;
        }
    }

    shape->mode = GL_TRIANGLES;

    return GLUS_TRUE;
}

static GLUSboolean glusMallocTempMemory(GLUSfloat** vertices, GLUSfloat** normals, GLUSfloat** texCoords, GLUSfloat** triangleVertices, GLUSfloat** triangleNormals, GLUSfloat** triangleTexCoords)
{
	if (!vertices || !normals || !texCoords || !triangleVertices || !triangleNormals || !triangleTexCoords)
	{
		return GLUS_FALSE;
	}

    *vertices = (GLUSfloat*)malloc(4 * GLUS_MAX_ATTRIBUTES * sizeof(GLUSfloat));
    if (!*vertices)
    {
    	return GLUS_FALSE;
    }

    *normals = (GLUSfloat*)malloc(3 * GLUS_MAX_ATTRIBUTES * sizeof(GLUSfloat));
    if (!*normals)
    {
    	return GLUS_FALSE;
    }

    *texCoords = (GLUSfloat*)malloc(2 * GLUS_MAX_ATTRIBUTES * sizeof(GLUSfloat));
    if (!*texCoords)
    {
    	return GLUS_FALSE;
    }

    *triangleVertices = (GLUSfloat*)malloc(4 * GLUS_MAX_TRIANGLE_ATTRIBUTES * sizeof(GLUSfloat));
    if (!*triangleVertices)
    {
    	return GLUS_FALSE;
    }

    *triangleNormals = (GLUSfloat*)malloc(3 * GLUS_MAX_TRIANGLE_ATTRIBUTES * sizeof(GLUSfloat));
    if (!*triangleNormals)
    {
    	return GLUS_FALSE;
    }

    *triangleTexCoords = (GLUSfloat*)malloc(2 * GLUS_MAX_TRIANGLE_ATTRIBUTES * sizeof(GLUSfloat));
    if (!*triangleTexCoords)
    {
    	return GLUS_FALSE;
    }

	return GLUS_TRUE;
}

static GLUSvoid glusFreeTempMemory(GLUSfloat** vertices, GLUSfloat** normals, GLUSfloat** texCoords, GLUSfloat** triangleVertices, GLUSfloat** triangleNormals, GLUSfloat** triangleTexCoords)
{
	if (vertices && *vertices)
	{
		free(*vertices);

		*vertices = 0;
	}

	if (normals && *normals)
	{
		free(*normals);

		*normals = 0;
	}

	if (texCoords && *texCoords)
	{
		free(*texCoords);

		*texCoords = 0;
	}

	if (triangleVertices && *triangleVertices)
	{
		free(*triangleVertices);

		*triangleVertices = 0;
	}

	if (triangleNormals && *triangleNormals)
	{
		free(*triangleNormals);

		*triangleNormals = 0;
	}

	if (triangleTexCoords && *triangleTexCoords)
	{
		free(*triangleTexCoords);

		*triangleTexCoords = 0;
	}
}

GLUSboolean GLUSAPIENTRY glusLoadObjFile(const GLUSchar* filename, GLUSshape* shape)
{
	GLUSboolean result;

    FILE* f;

    GLUSchar buffer[GLUS_BUFFERSIZE];
    GLUSchar identifier[3];

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

    GLUSuint totalNumberVertices= 0;
    GLUSuint totalNumberNormals= 0;
    GLUSuint totalNumberTexCoords= 0;

    GLUSuint facesEncoding = 0;

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

    if (!glusMallocTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords))
    {
    	glusFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

    	fclose(f);

    	return GLUS_FALSE;
    }

    while (!feof(f))
    {
        if (fgets(buffer, GLUS_BUFFERSIZE, f) == 0)
        {
        	if (ferror(f))
        	{
        		glusFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

                fclose(f);

                return GLUS_FALSE;
        	}
        }

        if (strncmp(buffer, "vt", 2) == 0)
        {
            if (numberTexCoords == GLUS_MAX_ATTRIBUTES)
            {
            	glusFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

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
            	glusFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

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
            	glusFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

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
                        	glusFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

                            fclose(f);

                            return GL_FALSE;
                        }

                        memcpy(&triangleVertices[4*totalNumberVertices], &vertices[4*vIndex], 4*sizeof(GLUSfloat));

                        totalNumberVertices++;
                    }
                    else
                    {
                        if (totalNumberVertices >= GLUS_MAX_TRIANGLE_ATTRIBUTES - 2)
                        {
                        	glusFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

                            fclose(f);

                            return GL_FALSE;
                        }

                        memcpy(&triangleVertices[4*(totalNumberVertices)], &triangleVertices[4*(totalNumberVertices-edgeCount)], 4*sizeof(GLUSfloat));
                        memcpy(&triangleVertices[4*(totalNumberVertices+1)], &triangleVertices[4*(totalNumberVertices-1)], 4*sizeof(GLUSfloat));
                        memcpy(&triangleVertices[4*(totalNumberVertices+2)], &vertices[4*vIndex], 4*sizeof(GLUSfloat));

                        totalNumberVertices += 3;
                    }
                }
                if (vnIndex >= 0)
                {
                    if (edgeCount < 3)
                    {
                        if (totalNumberNormals >= GLUS_MAX_TRIANGLE_ATTRIBUTES)
                        {
                        	glusFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

                            fclose(f);

                            return GL_FALSE;
                        }

                        memcpy(&triangleNormals[3*totalNumberNormals], &normals[3*vnIndex], 3*sizeof(GLUSfloat));

                        totalNumberNormals++;
                    }
                    else
                    {
                        if (totalNumberNormals >= GLUS_MAX_TRIANGLE_ATTRIBUTES - 2)
                        {
                        	glusFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

                            fclose(f);

                            return GL_FALSE;
                        }

                        memcpy(&triangleNormals[3*(totalNumberNormals)], &triangleNormals[3*(totalNumberNormals-edgeCount)], 3*sizeof(GLUSfloat));
                        memcpy(&triangleNormals[3*(totalNumberNormals+1)], &triangleNormals[3*(totalNumberNormals-1)], 3*sizeof(GLUSfloat));
                        memcpy(&triangleNormals[3*(totalNumberNormals+2)], &normals[3*vnIndex], 3*sizeof(GLUSfloat));

                        totalNumberNormals += 3;
                    }
                }
                if (vtIndex >= 0)
                {
                    if (edgeCount < 3)
                    {
                        if (totalNumberTexCoords >= GLUS_MAX_TRIANGLE_ATTRIBUTES)
                        {
                        	glusFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

                            fclose(f);

                            return GL_FALSE;
                        }

                        memcpy(&triangleTexCoords[2*totalNumberTexCoords], &texCoords[2*vtIndex], 2*sizeof(GLUSfloat));

                        totalNumberTexCoords++;
                    }
                    else
                    {
                        if (totalNumberTexCoords >= GLUS_MAX_TRIANGLE_ATTRIBUTES - 2)
                        {
                        	glusFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

                            fclose(f);

                            return GL_FALSE;
                        }

                        memcpy(&triangleTexCoords[2*(totalNumberTexCoords)], &triangleTexCoords[2*(totalNumberTexCoords-edgeCount)], 2*sizeof(GLUSfloat));
                        memcpy(&triangleTexCoords[2*(totalNumberTexCoords+1)], &triangleTexCoords[2*(totalNumberTexCoords-1)], 2*sizeof(GLUSfloat));
                        memcpy(&triangleTexCoords[2*(totalNumberTexCoords+2)], &texCoords[2*vtIndex], 2*sizeof(GLUSfloat));

                        totalNumberTexCoords += 3;
                    }
                }

                edgeCount++;

                token = strtok(0, " \n");
            }
        }
    }

    fclose(f);

    result = glusCopyObjData(shape, totalNumberVertices, triangleVertices, totalNumberNormals, triangleNormals, totalNumberTexCoords, triangleTexCoords);

    glusFreeTempMemory(&vertices, &normals, &texCoords, &triangleVertices, &triangleNormals, &triangleTexCoords);

    if (result)
    {
    	glusCalculateTangentSpacef(shape);
    }

    return result;
}
