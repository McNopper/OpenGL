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

GLUSboolean GLUSAPIENTRY glusCreateProgramFromSource(GLUSshaderprogram* shaderProgram, const GLUSchar** vertexSource, const GLUSchar** controlSource, const GLUSchar** evaluationSource, const GLUSchar** geometrySource, const GLUSchar** fragmentSource)
{
    GLUSint compiled;

    GLUSint logLength, charsWritten;

    char* log;

    if (!shaderProgram || !vertexSource || !fragmentSource)
    {
        return GLUS_FALSE;
    }

    shaderProgram->program = 0;
    shaderProgram->compute = 0;
    shaderProgram->vertex = 0;
    shaderProgram->control = 0;
    shaderProgram->evaluation = 0;
    shaderProgram->geometry = 0;
    shaderProgram->fragment = 0;

    shaderProgram->vertex = glCreateShader(GLUS_VERTEX_SHADER);

    glShaderSource(shaderProgram->vertex, 1, (const char**) vertexSource, 0);

    glCompileShader(shaderProgram->vertex);

    glGetShaderiv(shaderProgram->vertex, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        glGetShaderiv(shaderProgram->vertex, GL_INFO_LOG_LENGTH, &logLength);

        log = (char*) malloc((size_t)logLength);

        if (!log)
        {
            return GLUS_FALSE;
        }

        glGetShaderInfoLog(shaderProgram->vertex, logLength, &charsWritten, log);

        glusLogPrint(GLUS_LOG_ERROR, "Vertex shader compile error:");
        glusLogPrint(GLUS_LOG_ERROR, "%s", log);

        free(log);

        shaderProgram->vertex = 0;

        return GLUS_FALSE;
    }

    if (controlSource)
    {
        shaderProgram->control = glCreateShader(GLUS_TESS_CONTROL_SHADER);

        glShaderSource(shaderProgram->control, 1, (const char**) controlSource, 0);

        glCompileShader(shaderProgram->control);

        glGetShaderiv(shaderProgram->control, GL_COMPILE_STATUS, &compiled);

        if (!compiled)
        {
            glGetShaderiv(shaderProgram->control, GL_INFO_LOG_LENGTH, &logLength);

            log = (char*) malloc((size_t)logLength);

            if (!log)
            {
                glusDestroyProgram(shaderProgram);

                return GLUS_FALSE;
            }

            glGetShaderInfoLog(shaderProgram->control, logLength, &charsWritten, log);

            glusLogPrint(GLUS_LOG_ERROR, "Control shader compile error:");
            glusLogPrint(GLUS_LOG_ERROR, "%s", log);

            free(log);

            shaderProgram->control = 0;

            glusDestroyProgram(shaderProgram);

            return GLUS_FALSE;
        }
    }

    if (evaluationSource)
    {
        shaderProgram->evaluation = glCreateShader(GLUS_TESS_EVALUATION_SHADER);

        glShaderSource(shaderProgram->evaluation, 1, (const char**) evaluationSource, 0);

        glCompileShader(shaderProgram->evaluation);

        glGetShaderiv(shaderProgram->evaluation, GL_COMPILE_STATUS, &compiled);

        if (!compiled)
        {
            glGetShaderiv(shaderProgram->evaluation, GL_INFO_LOG_LENGTH, &logLength);

            log = (char*) malloc((size_t)logLength);

            if (!log)
            {
                glusDestroyProgram(shaderProgram);

                return GLUS_FALSE;
            }

            glGetShaderInfoLog(shaderProgram->evaluation, logLength, &charsWritten, log);

            glusLogPrint(GLUS_LOG_ERROR, "Evaluation shader compile error:");
            glusLogPrint(GLUS_LOG_ERROR, "%s", log);

            free(log);

            shaderProgram->evaluation = 0;

            glusDestroyProgram(shaderProgram);

            return GLUS_FALSE;
        }
    }

    if (geometrySource)
    {
        shaderProgram->geometry = glCreateShader(GLUS_GEOMETRY_SHADER);

        glShaderSource(shaderProgram->geometry, 1, (const char**) geometrySource, 0);

        glCompileShader(shaderProgram->geometry);

        glGetShaderiv(shaderProgram->geometry, GL_COMPILE_STATUS, &compiled);

        if (!compiled)
        {
            glGetShaderiv(shaderProgram->geometry, GL_INFO_LOG_LENGTH, &logLength);

            log = (char*) malloc((size_t)logLength);

            if (!log)
            {
                glusDestroyProgram(shaderProgram);

                return GLUS_FALSE;
            }

            glGetShaderInfoLog(shaderProgram->geometry, logLength, &charsWritten, log);

            glusLogPrint(GLUS_LOG_ERROR, "Geometry shader compile error:");
            glusLogPrint(GLUS_LOG_ERROR, "%s", log);

            free(log);

            shaderProgram->geometry = 0;

            glusDestroyProgram(shaderProgram);

            return GLUS_FALSE;
        }
    }

    shaderProgram->fragment = glCreateShader(GLUS_FRAGMENT_SHADER);

    glShaderSource(shaderProgram->fragment, 1, (const char**) fragmentSource, 0);

    glCompileShader(shaderProgram->fragment);

    glGetShaderiv(shaderProgram->fragment, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        glGetShaderiv(shaderProgram->fragment, GL_INFO_LOG_LENGTH, &logLength);

        log = (char*) malloc((size_t)logLength);

        if (!log)
        {
            glusDestroyProgram(shaderProgram);

            return GLUS_FALSE;
        }

        glGetShaderInfoLog(shaderProgram->fragment, logLength, &charsWritten, log);

        glusLogPrint(GLUS_LOG_ERROR, "Fragment shader compile error:");
        glusLogPrint(GLUS_LOG_ERROR, "%s", log);

        free(log);

        shaderProgram->fragment = 0;

        glusDestroyProgram(shaderProgram);

        return GLUS_FALSE;
    }

    shaderProgram->program = glCreateProgram();

    glAttachShader(shaderProgram->program, shaderProgram->vertex);

    if (shaderProgram->control)
    {
        glAttachShader(shaderProgram->program, shaderProgram->control);
    }

    if (shaderProgram->evaluation)
    {
        glAttachShader(shaderProgram->program, shaderProgram->evaluation);
    }

    if (shaderProgram->geometry)
    {
        glAttachShader(shaderProgram->program, shaderProgram->geometry);
    }

    glAttachShader(shaderProgram->program, shaderProgram->fragment);

    return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusCreateComputeProgramFromSource(GLUSshaderprogram* shaderProgram, const GLUSchar** computeSource)
{
    GLUSint compiled;

    GLUSint logLength, charsWritten;

    char* log;

    if (!shaderProgram || !computeSource)
    {
        return GLUS_FALSE;
    }

    shaderProgram->program = 0;
    shaderProgram->compute = 0;
    shaderProgram->vertex = 0;
    shaderProgram->control = 0;
    shaderProgram->evaluation = 0;
    shaderProgram->geometry = 0;
    shaderProgram->fragment = 0;

    shaderProgram->compute = glCreateShader(GLUS_COMPUTE_SHADER);

    glShaderSource(shaderProgram->compute, 1, (const char**) computeSource, 0);

    glCompileShader(shaderProgram->compute);

    glGetShaderiv(shaderProgram->compute, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        glGetShaderiv(shaderProgram->compute, GL_INFO_LOG_LENGTH, &logLength);

        log = (char*) malloc((size_t)logLength);

        if (!log)
        {
            return GLUS_FALSE;
        }

        glGetShaderInfoLog(shaderProgram->compute, logLength, &charsWritten, log);

        glusLogPrint(GLUS_LOG_ERROR, "Compute shader compile error:");
        glusLogPrint(GLUS_LOG_ERROR, "%s", log);

        free(log);

        shaderProgram->compute = 0;

        return GLUS_FALSE;
    }

    shaderProgram->program = glCreateProgram();

    glAttachShader(shaderProgram->program, shaderProgram->compute);

    return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusLinkProgram(GLUSshaderprogram* shaderProgram)
{
    GLUSint linked;

    GLUSint logLength, charsWritten;

    char* log;

    if (!shaderProgram)
    {
        return GLUS_FALSE;
    }

    glLinkProgram(shaderProgram->program);

    glGetProgramiv(shaderProgram->program, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        glGetProgramiv(shaderProgram->program, GL_INFO_LOG_LENGTH, &logLength);

        log = (char*) malloc((size_t)logLength);

        if (!log)
        {
            glusDestroyProgram(shaderProgram);

            return GLUS_FALSE;
        }

        glGetProgramInfoLog(shaderProgram->program, logLength, &charsWritten, log);

        glusLogPrint(GLUS_LOG_ERROR, "Shader program link error:");
        glusLogPrint(GLUS_LOG_ERROR, "%s", log);

        free(log);

        shaderProgram->program = 0;

        glusDestroyProgram(shaderProgram);

        return GLUS_FALSE;
    }

    return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusBuildProgramFromSource(GLUSshaderprogram* shaderProgram, const GLUSchar** vertexSource, const GLUSchar** controlSource, const GLUSchar** evaluationSource, const GLUSchar** geometrySource, const GLUSchar** fragmentSource)
{
    if (!glusCreateProgramFromSource(shaderProgram, vertexSource, controlSource, evaluationSource, geometrySource, fragmentSource))
    {
        return GLUS_FALSE;
    }

    return glusLinkProgram(shaderProgram);
}

GLUSboolean GLUSAPIENTRY glusBuildComputeProgramFromSource(GLUSshaderprogram* shaderProgram, const GLUSchar** computeSource)
{
    if (!glusCreateComputeProgramFromSource(shaderProgram, computeSource))
    {
        return GLUS_FALSE;
    }

    return glusLinkProgram(shaderProgram);
}

GLUSvoid GLUSAPIENTRY glusDestroyProgram(GLUSshaderprogram* shaderprogram)
{
    if (!shaderprogram)
    {
        return;
    }

    if (shaderprogram->program)
    {
        glDeleteProgram(shaderprogram->program);

        shaderprogram->program = 0;
    }

    if (shaderprogram->fragment)
    {
        glDeleteShader(shaderprogram->fragment);

        shaderprogram->fragment = 0;
    }

    if (shaderprogram->geometry)
    {
        glDeleteShader(shaderprogram->geometry);

        shaderprogram->geometry = 0;
    }

    if (shaderprogram->evaluation)
    {
        glDeleteShader(shaderprogram->evaluation);

        shaderprogram->evaluation = 0;
    }

    if (shaderprogram->control)
    {
        glDeleteShader(shaderprogram->control);

        shaderprogram->control = 0;
    }

    if (shaderprogram->vertex)
    {
        glDeleteShader(shaderprogram->vertex);

        shaderprogram->vertex = 0;
    }

    if (shaderprogram->compute)
    {
        glDeleteShader(shaderprogram->compute);

        shaderprogram->compute = 0;
    }
}
