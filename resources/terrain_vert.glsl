#version 330

layout (location = 0) in vec4 vertPos;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 gWorldPos;


void main()
{
	// compute world space position
	vec4 Position = M * vertPos;
	gWorldPos = vec3(Position);

	// write out clip space
	gl_Position = P * V * Position;
}
