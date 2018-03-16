#version 330

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertTex;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

uniform sampler2D heightTex;

out vec3 gWorldPos;
out vec2 gTexCoords;

void main()
{
	// compute world space position
	vec4 Position = M * vec4(vertPos + vec3(0, texture(heightTex, vertTex).r, 0), 1.0);
	gWorldPos = vec3(Position);

	// write out clip space
	gl_Position = P * V * Position;

	gTexCoords = vertTex;
}
