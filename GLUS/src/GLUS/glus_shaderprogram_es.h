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

#ifndef GLUS_SHADERPROGRAM_ES_H_
#define GLUS_SHADERPROGRAM_ES_H_

/**
 * Creates a program by compiling the giving sources. Linking has to be done in a separate step.
 *
 * @param shaderProgram This structure holds the necessary information of the program and the different shaders.
 * @param vertexSource Vertex shader source code.
 * @param fragmentSource Fragment shader source code.
 *
 * @return GLUS_TRUE, if compiling and creation of program succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateProgramFromSource(GLUSshaderprogram* shaderProgram, const GLUSchar** vertexSource, const GLUSchar** fragmentSource);

/**
 * Links a formerly created program.
 *
 * @param shaderProgram This structure holds the necessary information of the program and the different shaders.
 *
 * @return GLUS_TRUE, if linking of program succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusLinkProgram(GLUSshaderprogram* shaderProgram);

/**
 * Builds a program by compiling and linking the giving sources.
 *
 * @param shaderProgram This structure holds the necessary information of the program and the different shaders.
 * @param vertexSource Vertex shader source code.
 * @param fragmentSource Fragment shader source code.
 *
 * @return GLUS_TRUE, if compiling and linking of program succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusBuildProgramFromSource(GLUSshaderprogram* shaderProgram, const GLUSchar** vertexSource, const GLUSchar** fragmentSource);

/**
 * Destroys a program by freeing all resources.
 *
 * @param shaderprogram This structure holds the necessary information of the program and the different shaders.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyProgram(GLUSshaderprogram* shaderprogram);

#endif /* GLUS_SHADERPROGRAM_ES_H_ */
