#version 330

in vec2 texCoord;

layout(location = 0) out vec3 color;
layout(location = 1) out float height;


void main()
{
	color = vec3(texCoord, 0.0);
	height = texCoord.r * 100.0;
}
