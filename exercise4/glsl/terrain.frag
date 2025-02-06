	#version 330 core
	// This source code is property of the Computer Graphics and Visualization 
	// chair of the TU Dresden. Do not distribute! 
	// Copyright (C) CGV TU Dresden - All Rights Reserved

	
	in vec3 normal; // Interpolated normal from the vertex shader
	in vec3 tangent;      // Tangent vector
	in vec3 bitangent;    // Bitangent vector
	in vec3 fragPosition; // Fragment position 

	out vec4 color;

	uniform vec3 cameraPos;
	uniform sampler2D grassTexture; // Grass texture sampler
	uniform sampler2D rockTexture;
	uniform sampler2D alphaMap;       // Alpha map
	uniform sampler2D roadColorTexture; // Road texture
	uniform sampler2D roadSpecularMap;  // Road specular map sampler
	uniform sampler2D roadNormalMap;    // Road normal map sampler
	uniform sampler2D background;
	uniform vec2 screenSize;

	const vec3 dirToLight = normalize(vec3(1, 3, 1));	

	//Calculates the visible surface color based on the Blinn-Phong illumination model
	vec4 calculateLighting(vec4 materialColor, float specularIntensity, vec3 normalizedNormal, vec3 directionToViewer)
	{
		vec4 color = materialColor;
		vec3 h = normalize(dirToLight + directionToViewer);
		color.xyz *= 0.9 * max(abs(dot(normalizedNormal, dirToLight)), 0) + 0.1;
		color.xyz += specularIntensity * pow(max(dot(h, normalizedNormal), 0), 50);
		return color;
	}

	vec4 getBackgroundColor()
	{
		return texture(background, gl_FragCoord.xy / screenSize);
	}

	void main()
	{
		//surface geometry
		vec3 n = normalize(normal);
		vec3 dirToViewer = normalize(cameraPos - fragPosition);


		// Compute texture coordinates
		vec2 texCoords = fragPosition.xz / 25.5; // (255 / 10)
		texCoords = fract(texCoords); // Ensure the texture repeats

		// Sample the grass texture and rock texture
		vec3 grassColor = texture(grassTexture, texCoords).rgb;
		vec3 rockColor = texture(rockTexture, texCoords).rgb;

		// Compute the steepness based on the normal (dot product with up vector)
		float steepness = abs(dot(n, vec3(0.0, 1.0, 0.0)));

		// Blend between grass and rock textures based on steepness
		vec3 terrainColor  = mix( rockColor,grassColor, smoothstep(0.5, 1.0, steepness));

		// Compute texture coordinates for the alpha map (scaled to 255x255 world-space size)
		vec2 alphaMapCoords = fragPosition.xz / 255.0;

		// Sample the alpha map (white = road, black = terrain, gray = blend)
		float alpha = texture(alphaMap, alphaMapCoords).r;

		// Sample the road texture (use the same coordinates as the alpha map)
		vec3 roadColor = texture(roadColorTexture, alphaMapCoords).rgb;

		// Sample the road specular map to get the specular intensity
		float roadSpecularIntensity = texture(roadSpecularMap, alphaMapCoords).r;

		// Sample the road normal map to get the tangent-space normal
		vec3 tangentSpaceNormal = texture(roadNormalMap, alphaMapCoords).rgb;
		tangentSpaceNormal = normalize(tangentSpaceNormal * 2.0 - 1.0); // Convert from [0, 1] to [-1, 1]
		tangentSpaceNormal.y = -tangentSpaceNormal.y; // Invert the y-component

		// Create the TBN matrix (Tangent, Bitangent, Normal)
		vec3 T = normalize(tangent);
		vec3 B = normalize(bitangent);
		vec3 N = normalize(normal);
		mat3 TBN = mat3(T, B, N);

		// Transform the tangent-space normal to world space
		vec3 worldSpaceNormal = TBN * tangentSpaceNormal;

		// Blend between terrain and road textures using the alpha map
		vec3 finalColor = mix(terrainColor, roadColor, alpha);

		// Blend the normals for lighting calculations
		vec3 finalNormal = mix(n, worldSpaceNormal, alpha);

		// Set the final color of the fragment
		float specularIntensity =  mix(0.5, roadSpecularIntensity, alpha); // Calculate the specular intensity for the final color
		
		//color = vec4(finalNormal, 1.0);

		//Calculate light
		color = calculateLighting(vec4(finalColor, 1.0), specularIntensity, finalNormal, dirToViewer);

	
	}