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

#ifndef GLUS_PROGRAMPIPELINE_H_
#define GLUS_PROGRAMPIPELINE_H_

/**
 * Structure for program pipeline handling.
 */
typedef struct _GLUSprogrampipeline
{
	/**
	 * The created pipeline.
	 */
    GLUSuint pipeline;

    /**
     * Compute shader program.
     */
    GLUSuint computeProgram;

    /**
     * Vertex shader program.
     */
    GLUSuint vertexProgram;

    /**
     * Tessellation control shader program.
     */
    GLUSuint controlProgram;

    /**
     * Tessellation evaluation shader program.
     */
    GLUSuint evaluationProgram;

    /**
     * Geometry shader program.
     */
    GLUSuint geometryProgram;

    /**
     * Fragment shader program.
     */
    GLUSuint fragmentProgram;

} GLUSprogrampipeline;

/**
 * Builds a program pipeline.
 *
 * @param programPipeline 	This structure holds the necessary information of the program pipeline and the different shader programs.
 * @param vertexProgram   	Vertex shader program.
 * @param controlProgram  	Tessellation control shader program.
 * @param evaluationProgram Tessellation evaluation shader program.
 * @param geometryProgram	Geometry shader program.
 * @param fragmentProgram 	Fragment shader program.
 *
 * @return GLUS_TRUE, if building of program pipeline succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusBuildProgramPipeline(GLUSprogrampipeline* programPipeline, GLUSuint vertexProgram, GLUSuint controlProgram, GLUSuint evaluationProgram, GLUSuint geometryProgram, GLUSuint fragmentProgram);

/**
 * Builds a compute shader program pipeline.
 *
 * @param programPipeline This structure holds the necessary information of the program pipeline and the different shader programs.
 * @param computeProgram  Compute shader program.
 *
 * @return GLUS_TRUE, if building of program pipeline succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusBuildComputeProgramPipeline(GLUSprogrampipeline* programPipeline, GLUSuint computeProgram);

/**
 * Destroys a program pipeline by freeing the pipeline. The programs are not freed.
 *
 * @param programPipeline This structure holds the necessary information of the program pipeline and the different shader programs.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyProgramPipeline(GLUSprogrampipeline* programPipeline);

#endif /* GLUS_PROGRAMPIPELINE_H_ */
