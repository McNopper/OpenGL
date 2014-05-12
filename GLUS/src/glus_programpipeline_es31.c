/*
 * GLUS - OpenGL ES 2.0 and 3.0 Utilities. Copyright (C) 2010 - 2013 Norbert Nopper
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

GLUSAPI GLUSboolean GLUSAPIENTRY glusBuildProgramPipeline(GLUSprogrampipeline* programPipeline, GLUSuint vertexProgram, GLUSuint fragmentProgram)
{
    if (!programPipeline)
    {
        return GLUS_FALSE;
    }

	programPipeline->computeProgram = 0;
    programPipeline->vertexProgram = vertexProgram;
    programPipeline->fragmentProgram = fragmentProgram;

	glGenProgramPipelines(1, &programPipeline->pipeline);
	if (vertexProgram)
	{
		glUseProgramStages(programPipeline->pipeline, GLUS_VERTEX_SHADER_BIT, vertexProgram);
	}
	if (fragmentProgram)
	{
		glUseProgramStages(programPipeline->pipeline, GLUS_FRAGMENT_SHADER_BIT, fragmentProgram);
	}

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusBuildComputeProgramPipeline(GLUSprogrampipeline* programPipeline, GLUSuint computeProgram)
{
    if (!programPipeline)
    {
        return GLUS_FALSE;
    }

	programPipeline->computeProgram = computeProgram;
    programPipeline->vertexProgram = 0;
    programPipeline->fragmentProgram = 0;

	glGenProgramPipelines(1, &programPipeline->pipeline);
	if (computeProgram)
	{
		glUseProgramStages(programPipeline->pipeline, GLUS_COMPUTE_SHADER_BIT, computeProgram);
	}

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusDestroyProgramPipeline(GLUSprogrampipeline* programPipeline)
{
    if (!programPipeline)
    {
        return;
    }

    if (programPipeline->pipeline)
    {
        glUseProgramStages(programPipeline->pipeline, GLUS_COMPUTE_SHADER_BIT, 0);
        glUseProgramStages(programPipeline->pipeline, GLUS_VERTEX_SHADER_BIT, 0);
        glUseProgramStages(programPipeline->pipeline, GLUS_FRAGMENT_SHADER_BIT, 0);

    	glDeleteProgramPipelines(1, &programPipeline->pipeline);

        programPipeline->pipeline = 0;
    }

    programPipeline->computeProgram = 0;
    programPipeline->fragmentProgram = 0;
    programPipeline->vertexProgram = 0;

}
