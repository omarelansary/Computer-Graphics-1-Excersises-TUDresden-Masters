// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Viewer.h"

#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/checkbox.h>

#include <gui/SliderHelper.h>

#include <iostream>

#include <stb_image.h>

#include "glsl.h"
#include "textures.h"

const uint32_t PATCH_SIZE = 256; //number of vertices along one side of the terrain patch

Viewer::Viewer()
	: AbstractViewer("CG1 Exercise 4"),
	terrainPositions(nse::gui::VertexBuffer), terrainIndices(nse::gui::IndexBuffer),
	offsetBuffer(nse::gui::VertexBuffer)
{
	LoadShaders();
	CreateGeometry();
	
	//Create a texture and framebuffer for the background
	glGenFramebuffers(1, &backgroundFBO);
	glGenTextures(1, &backgroundTexture);
	ensureFBO();

	//Align camera to view a reasonable part of the terrain
	camera().SetSceneExtent(nse::math::BoundingBox<float, 3>(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(PATCH_SIZE - 1, 0, PATCH_SIZE - 1)));
	camera().FocusOnPoint(0.5f * Eigen::Vector3f(PATCH_SIZE - 1, 15, PATCH_SIZE - 1));	
	camera().Zoom(-30);
	camera().RotateAroundFocusPointLocal(Eigen::AngleAxisf(-0.5f, Eigen::Vector3f::UnitY()) * Eigen::AngleAxisf(-0.05f, Eigen::Vector3f::UnitX()));
	camera().FixClippingPlanes(0.1f, 1000.f);
}

bool Viewer::resizeEvent(const Eigen::Vector2i&)
{
	//Re-generate the texture and FBO for the background
	ensureFBO();
	return false;
}

void Viewer::LoadShaders()
{
	skyShader.init("Sky Shader", std::string((const char*)sky_vert, sky_vert_size), std::string((const char*)sky_frag, sky_frag_size));
	terrainShader.init("Terrain Shader", std::string((const char*)terrain_vert, terrain_vert_size), std::string((const char*)terrain_frag, terrain_frag_size));
}

GLuint CreateTexture(const unsigned char* fileData, size_t fileLength, bool repeat = true)
{
	GLuint textureName;
	int textureWidth, textureHeight, textureChannels;

	// Load pixel data from the file
	auto pixelData = stbi_load_from_memory(fileData, (int)fileLength, &textureWidth, &textureHeight, &textureChannels, 3);
	if (!pixelData)
	{
		std::cerr << "Failed to load texture data!" << std::endl;
		return 0; // Return 0 for failure
	}

	// Generate a texture object
	glGenTextures(1, &textureName);
	glBindTexture(GL_TEXTURE_2D, textureName);

	// Upload the pixel data to the GPU
	glTexImage2D(
		GL_TEXTURE_2D,
		0,                  // Mipmap level
		GL_RGB,             // Internal format
		textureWidth,       // Texture width
		textureHeight,      // Texture height
		0,                  // Border (must be 0)
		GL_RGB,             // Format of the input data
		GL_UNSIGNED_BYTE,   // Data type of the input data
		pixelData           // Pointer to pixel data
	);

	// Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

	// Unbind the texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// Free the pixel data
	stbi_image_free(pixelData);

	return textureName;
}

void Viewer::CreateGeometry()
{
	//empty VAO for sky
	emptyVAO.generate();

	//terrain VAO	
	terrainVAO.generate();
	terrainVAO.bind();
	
	std::vector<Eigen::Vector4f> positions;
	std::vector<uint32_t> indices;
	
	/*Generate positions and indices for a terrain patch with a
	  single triangle strip */

	  // Generate positions
	for (int z = 0; z < PATCH_SIZE; ++z) {
		for (int x = 0; x < PATCH_SIZE; ++x) {
			positions.emplace_back(x , 0.0f, z , 1.0f);
		}
	}

	// Generate indices for a triangle strip
	for (int z = 0; z < PATCH_SIZE - 1; ++z) {
		for (int x = 0; x < PATCH_SIZE; ++x) {
			indices.push_back(z * PATCH_SIZE + x);        // Current row
			indices.push_back((z + 1) * PATCH_SIZE + x);  // Next row
		}
		// Add degenerate triangles if not on the last strip 
		// for this forloop size if indices will be 131068
		if (z < PATCH_SIZE - 2) {
			indices.push_back((z + 1) * PATCH_SIZE + (PATCH_SIZE - 1));
			indices.push_back((z + 1) * PATCH_SIZE);
		}
	}

	terrainPositions.bind();
	terrainIndices.bind();

	terrainShader.bind();
	terrainPositions.uploadData(positions).bindToAttribute("position");
	terrainIndices.uploadData((uint32_t)indices.size() * sizeof(uint32_t), indices.data());

	// Set up the offset attribute for instanced rendering
	offsetBuffer.bind();
	std::vector<Eigen::Vector2f> offsets = { Eigen::Vector2f(0.0f, 0.0f) }; // Single offset for now
	offsetBuffer.uploadData(offsets).bindToAttribute("offset");
	
	// Set the attribute divisor for instanced rendering
	GLuint offsetAttrib = terrainShader.attrib("offset");
	glEnableVertexAttribArray(offsetAttrib);
	glVertexAttribPointer(offsetAttrib, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glVertexAttribDivisor(offsetAttrib, 1); // Set divisor to 1 for instanced rendering

	// Unbind VAO
	terrainVAO.unbind();

	//textures
	grassTexture = CreateTexture((unsigned char*)grass_jpg, grass_jpg_size);
	rockTexture = CreateTexture((unsigned char*)rock_jpg, rock_jpg_size);
	roadColorTexture = CreateTexture((unsigned char*)roadcolor_jpg, roadcolor_jpg_size);
	roadNormalMap = CreateTexture((unsigned char*)roadnormals_jpg, roadnormals_jpg_size);
	roadSpecularMap = CreateTexture((unsigned char*)roadspecular_jpg, roadspecular_jpg_size);
	alphaMap = CreateTexture((unsigned char*)alpha_jpg, alpha_jpg_size, false);
}

void Viewer::ensureFBO()
{
	//Re-generate the texture and FBO for the background
	glBindFramebuffer(GL_FRAMEBUFFER, backgroundFBO);
	glBindTexture(GL_TEXTURE_2D, backgroundTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width(), height(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, backgroundTexture, 0);
	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Warning: Background framebuffer is not complete: " << fboStatus << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Viewer::RenderSky()
{
	Eigen::Matrix4f skyView = view;
	for (int i = 0; i < 3; ++i)
		skyView.col(i).normalize();
	skyView.col(3).head<3>().setZero();
	Eigen::Matrix4f skyMvp = proj * skyView;
	glDepthMask(GL_FALSE);
	glEnable(GL_DEPTH_CLAMP);
	emptyVAO.bind();
	skyShader.bind();
	skyShader.setUniform("mvp", skyMvp);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);
	glDisable(GL_DEPTH_CLAMP);
	glDepthMask(GL_TRUE);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, backgroundFBO);
	glBlitFramebuffer(0, 0, width(), height(), 0, 0, width(), height(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void CalculateViewFrustum(const Eigen::Matrix4f& mvp, Eigen::Vector4f* frustumPlanes, nse::math::BoundingBox<float, 3>& bbox)
{
	frustumPlanes[0] = (mvp.row(3) + mvp.row(0)).transpose();
	frustumPlanes[1] = (mvp.row(3) - mvp.row(0)).transpose();
	frustumPlanes[2] = (mvp.row(3) + mvp.row(1)).transpose();
	frustumPlanes[3] = (mvp.row(3) - mvp.row(1)).transpose();
	frustumPlanes[4] = (mvp.row(3) + mvp.row(2)).transpose();
	frustumPlanes[5] = (mvp.row(3) - mvp.row(2)).transpose();

	Eigen::Matrix4f invMvp = mvp.inverse();
	bbox.reset();
	for(int x = -1; x <= 1; x += 2)
		for(int y = -1; y <= 1; y += 2)
			for (int z = -1; z <= 1; z += 2)
	{
		Eigen::Vector4f corner = invMvp * Eigen::Vector4f((float)x, (float)y, (float)z, 1);
		corner /= corner.w();
		bbox.expand(corner.head<3>());
	}
}

bool IsBoxCompletelyBehindPlane(const Eigen::Vector3f& boxMin, const Eigen::Vector3f& boxMax, const Eigen::Vector4f& plane)
{
	return
		plane.dot(Eigen::Vector4f(boxMin.x(), boxMin.y(), boxMin.z(), 1)) < 0 &&
		plane.dot(Eigen::Vector4f(boxMin.x(), boxMin.y(), boxMax.z(), 1)) < 0 &&
		plane.dot(Eigen::Vector4f(boxMin.x(), boxMax.y(), boxMin.z(), 1)) < 0 &&
		plane.dot(Eigen::Vector4f(boxMin.x(), boxMax.y(), boxMax.z(), 1)) < 0 &&
		plane.dot(Eigen::Vector4f(boxMax.x(), boxMin.y(), boxMin.z(), 1)) < 0 &&
		plane.dot(Eigen::Vector4f(boxMax.x(), boxMin.y(), boxMax.z(), 1)) < 0 &&
		plane.dot(Eigen::Vector4f(boxMax.x(), boxMax.y(), boxMin.z(), 1)) < 0 &&
		plane.dot(Eigen::Vector4f(boxMax.x(), boxMax.y(), boxMin.z(), 1)) < 0;
}

void Viewer::drawContents() 
{
	camera().ComputeCameraMatrices(view, proj);

	Eigen::Matrix4f mvp = proj * view;
	Eigen::Vector3f cameraPosition = view.inverse().col(3).head<3>();
	int visiblePatches = 0;

	RenderSky();

	// Step 1: Calculate the view frustum and its bounding box
	Eigen::Vector4f frustumPlanes[6];
	nse::math::BoundingBox<float, 3> frustumBBox;
	CalculateViewFrustum(mvp, frustumPlanes, frustumBBox);

	// Step 2: Determine visible patches
	std::vector<Eigen::Vector2f> visibleOffsets;
	int patchCount = 10; // Number of patches to check in each direction (adjust as needed)
	for (int z = -patchCount; z <= patchCount; ++z) {
		for (int x = -patchCount; x <= patchCount; ++x) {
			// Calculate the offset for this patch
			Eigen::Vector2f offset(x * (PATCH_SIZE - 1), z * (PATCH_SIZE - 1));

			// Define the bounding box for this patch
			Eigen::Vector3f patchMin(offset.x(), 0.0f, offset.y());
			Eigen::Vector3f patchMax(offset.x() + (PATCH_SIZE - 1), 15.0f, offset.y() + (PATCH_SIZE - 1));

			// Check if the patch is visible (not completely behind any frustum plane)
			bool isVisible = true;
			for (int i = 0; i < 6; ++i) {
				if (IsBoxCompletelyBehindPlane(patchMin, patchMax, frustumPlanes[i])) {
					isVisible = false;
					break;
				}
			}

			// If the patch is visible, add its offset to the list
			if (isVisible) {
				visibleOffsets.push_back(offset);
			}
		}
	}

	// Step 3: Update the offset buffer with visible patch offsets
	offsetBuffer.bind();
	offsetBuffer.uploadData(visibleOffsets);

	// Step 4: Render the terrain using instanced rendering
	glEnable(GL_DEPTH_TEST);
	terrainVAO.bind();
	terrainShader.bind();


	terrainShader.setUniform("screenSize", Eigen::Vector2f(width(), height()), false);
	terrainShader.setUniform("mvp", mvp);
	terrainShader.setUniform("cameraPos", cameraPosition, false);
	/* Task: Render the terrain */

	// Set the texture unit for the grass and rock textures using the GLShader uniform function
	terrainShader.setUniform("grassTexture", 0);  // Texture unit 0 for grass
	terrainShader.setUniform("rockTexture", 1);   // Texture unit 1 for rock  //
	terrainShader.setUniform("roadColorTexture", 2);   // Texture unit 2 for road  //
	terrainShader.setUniform("alphaMap", 3);   // Texture unit 2 for road  //
	terrainShader.setUniform("roadNormalMap", 4);   // Texture unit 2 for road  //
	terrainShader.setUniform("roadSpecularMap", 5);   // Texture unit 2 for road  //

	//// Bind the grass texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, grassTexture);

	// Bind the rock texture (for steep slopes)
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, rockTexture);

	// Bind the road texture (from alpha map and road color)
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, roadColorTexture);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, alphaMap);

	// Bind the road normal map (for normal mapping)
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, roadNormalMap);

	// Bind the road specular map (for specular highlights)
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, roadSpecularMap);

	// Draw the terrain using instanced rendering
	glDrawElementsInstanced(GL_TRIANGLE_STRIP, terrainIndices.bufferSize(), GL_UNSIGNED_INT, 0, visibleOffsets.size());

	// Update the number of visible patches
	visiblePatches = visibleOffsets.size();

	// Render text (number of visible patches)
	nvgBeginFrame(mNVGContext, (float)width(), (float)height(), mPixelRatio);
	std::string text = "Patches visible: " + std::to_string(visiblePatches);
	nvgText(mNVGContext, 10, 20, text.c_str(), nullptr);
	nvgEndFrame(mNVGContext);
}
