#version 330

in vec2 texCoord;

layout(location = 0) out vec3 color;
layout(location = 1) out float height;


// Simplex 2D noise
// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v)
{
	const vec4 C = vec4(
		0.211324865405187, 0.366025403784439,
		-0.577350269189626, 0.024390243902439);
	vec2 i  = floor(v + dot(v, C.yy) );
	vec2 x0 = v -   i + dot(i, C.xx);
	vec2 i1;
	i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
	vec4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;
	i = mod(i, 289.0);
	vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
	+ i.x + vec3(0.0, i1.x, 1.0 ));
	vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
	dot(x12.zw,x12.zw)), 0.0);
	m = m*m ;
	m = m*m ;
	vec3 x = 2.0 * fract(p * C.www) - 1.0;
	vec3 h = abs(x) - 0.5;
	vec3 ox = floor(x + 0.5);
	vec3 a0 = x - ox;
	m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
	vec3 g;
	g.x  = a0.x  * x0.x  + h.x  * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	return 130.0 * dot(m, g);
}

float fnoise(vec2 v)
{
	float sum = 0.0;
	sum += snoise(v * 1.0) * 0.5;
	sum += snoise(v * 2.0) * 0.25;
	sum += snoise(v * 4.0) * 0.125;
	sum += snoise(v * 8.0) * 0.0625;
	return sum;
}

float rnoise(vec2 v)
{
	float sum = 0.0;
	float freq = 1.0;
	float amp = 1.0;

	for (int i = 0; i < 16; ++ i)
	{
		float val = 1.0 - 2.0 * abs(snoise(v * freq));

		sum += amp * val;

		amp /= 2.5;
		freq *= 2.0;
	}

	return sum;
}

float dnoise(vec2 v)
{
	vec2 distortion = vec2(
		snoise(v + vec2(3.0)),
		snoise(v - vec2(1.5)));

	return snoise(v + distortion * 3.0);
}

void main()
{
	float elevation = rnoise(texCoord);
	elevation = min(elevation, 0.75 + 0.25 * fnoise(texCoord * 4.0 + vec2(42.0)));


	float colElevation = elevation + fnoise(texCoord * 8.0 + vec2(2.5, 3.11)) * 0.5;

	color = vec3(1.0);

	float river = fnoise(texCoord * 4.0 + vec2(7.0));


	if (colElevation < 0.55)
		color = vec3(0.3, 0.2, 0.0);
	if (colElevation < 0.0)
		color = vec3(0.1, 0.5, 0.1);

	if (abs(river) < 0.05)
	{
		color = vec3(0.2, 0.2, 0.6);
	}
	height = elevation * 20.0;

	if (abs(river) < 0.1)
	{
		height -= mix(0.0, 6.0, clamp(abs(abs(river) - 0.1) / 0.025, 0.0, 1.0));
	}
}
