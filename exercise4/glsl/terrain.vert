#version 330 core
// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

in vec4 position;
in vec4 offset;
out vec3 normal; // Pass the computed normal to the fragment shader
out vec3 tangent;      // Pass the tangent vector to the fragment shader
out vec3 bitangent;    // Pass the bitangent vector to the fragment shader
out vec3 fragPosition; // Pass the vertex position to the fragment shader


uniform mat4 mvp;

//Returns the height of the procedural terrain at a given xz position
float getTerrainHeight(vec2 p);



void main()
{
	float delta = 0.1; // Small offset for finite differences
	vec4 newPosition=position; //because position is read only
	newPosition.xz += offset.xz; // Add the offset to the x and z coordinates
	newPosition.y=getTerrainHeight(newPosition.xz);

    vec3 p = newPosition.xyz;

	// Calculate heights for neighboring vertices
	float hL = getTerrainHeight(p.xz - vec2(delta, 0.0)); // Left
	float hR = getTerrainHeight(p.xz + vec2(delta, 0.0)); // Right
	float hD = getTerrainHeight(p.xz - vec2(0.0, delta)); // Down
	float hU = getTerrainHeight(p.xz + vec2(0.0, delta)); // Up

    // Compute surface normal using cross product
    vec3 tangentX = vec3(2*delta, hR - hL,0);
    vec3 tangentZ = vec3(0, hU - hD, 2*delta);
    normal = normalize(cross(tangentX, tangentZ));

	// Normalize tangent and bitangent vectors
    tangent = normalize(tangentX);
    bitangent = normalize(tangentZ);

    // Pass the position and normal to the fragment shader
    fragPosition = newPosition.xyz;

	// Transform vertex position to clip space
	gl_Position = mvp * newPosition;
}

//source: https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
float rand(vec2 c)
{
	return 2 * fract(sin(dot(c.xy ,vec2(12.9898,78.233))) * 43758.5453) - 1;
}

float perlinNoise(vec2 p )
{
	vec2 ij = floor(p);
	vec2 xy = p - ij;
	//xy = 3.*xy*xy-2.*xy*xy*xy;
	xy = .5*(1.-cos(3.1415926 * xy));
	float a = rand((ij+vec2(0.,0.)));
	float b = rand((ij+vec2(1.,0.)));
	float c = rand((ij+vec2(0.,1.)));
	float d = rand((ij+vec2(1.,1.)));
	float x1 = mix(a, b, xy.x);
	float x2 = mix(c, d, xy.x);
	return mix(x1, x2, xy.y);
}

//based on https://www.seedofandromeda.com/blogs/58-procedural-heightmap-terrain-generation
float getTerrainHeight(vec2 p)
{
	float total = 0.0;
	float maxAmplitude = 0.0;
	float amplitude = 1.0;
	float frequency = 0.02;
	for (int i = 0; i < 11; i++) 
	{
		total +=  ((1.0 - abs(perlinNoise(p * frequency))) * 2.0 - 1.0) * amplitude;
		frequency *= 2.0;
		maxAmplitude += amplitude;
		amplitude *= 0.45;
	}
	return 15 * total / maxAmplitude;
}