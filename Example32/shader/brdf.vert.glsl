#version 410 core

uniform mat4 u_viewProjectionMatrix;
uniform mat4 u_modelMatrix;
uniform mat3 u_normalMatrix;

uniform vec4 u_eye;

in vec4 a_vertex;
in vec3 a_normal;

out vec3 v_eye;

out vec3 v_tangent;
out vec3 v_bitangent;
out vec3 v_normal;

void calculateBasis(out vec3 tangent, out vec3 bitangent, in vec3 normal)
{
    bitangent = vec3(0.0, 1.0, 0.0);

    // Compared on the NORMALIZED normal: v_normal carries the model scale (0.001
    // in this example), so dot() could never reach exactly +/-1 and these
    // degenerate-case guards were dead code. At a pole cross(bitangent, normal)
    // was then vec3(0) and the fragment shader's normalize(v_tangent) produced
    // NaN basis vectors.
    float normalDotUp = dot(normalize(normal), bitangent);

    if (normalDotUp > 0.999)
    {
        bitangent = vec3(0.0, 0.0, -1.0);
    }
    else if (normalDotUp < -0.999)
    {
        bitangent = vec3(0.0, 0.0, 1.0);
    }

    tangent   = cross(bitangent, normal);
    bitangent = cross(normal, tangent);
}

void main(void)
{
    v_normal = u_normalMatrix * a_normal;

    calculateBasis(v_tangent, v_bitangent, v_normal);

    vec4 vertex = u_modelMatrix * a_vertex;

    v_eye = (u_eye - vertex).xyz;

    gl_Position = u_viewProjectionMatrix * vertex;
}
