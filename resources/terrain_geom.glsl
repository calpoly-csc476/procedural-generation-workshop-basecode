#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 gWorldPos[];

out vec3 fWorldPos;
out vec3 fNormal;


void main()
{
	vec3 Normal = normalize(cross(
		gWorldPos[1].xyz - gWorldPos[0].xyz,
		gWorldPos[2].xyz - gWorldPos[0].xyz));

	for (int i = 0; i < 3; ++ i)
	{
		gl_Position = gl_in[i].gl_Position;
		fWorldPos = gWorldPos[i];

		fNormal = Normal;
		EmitVertex();
	}

	EndPrimitive();
}
