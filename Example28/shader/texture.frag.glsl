#version 410 core

uniform sampler2D u_texture;

in float v_intensity;
in vec2 v_texCoord;
in vec3 v_normal;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 normalColor;

void main(void)
{
	fragColor = texture(u_texture, v_texCoord) * v_intensity;
		
	//
	
	vec3 normal = normalize(v_normal);
	
	normalColor = vec4(normal.x * 0.5 + 0.5, normal.y * 0.5 + 0.5, normal.z * 0.5 + 0.5, 1.0);
}
