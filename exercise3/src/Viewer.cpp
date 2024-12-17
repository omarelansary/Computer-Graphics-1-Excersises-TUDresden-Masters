// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Viewer.h"

#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/checkbox.h>

#include <gui/SliderHelper.h>

#include <iostream>

#include <OpenMesh/Core/IO/MeshIO.hh>

#include "glsl.h"

Viewer::Viewer()
	: AbstractViewer("CG1 Exercise 3"), vertex_shader_id(0), fragment_shader_id(0), program_id(0)
{ 
	SetupGUI();

	CreateShaders();
	CreateVertexBuffers();

	modelViewMatrix.setIdentity();
	projectionMatrix.setIdentity();

	camera().FocusOnBBox(nse::math::BoundingBox<float, 3>(Eigen::Vector3f(-1, -1, -1), Eigen::Vector3f(1, 1, 1)));
}

void Viewer::SetupGUI()
{
	auto mainWindow = SetupMainWindow();

	//Create GUI elements for the various options
	chkHasDepthTesting = new nanogui::CheckBox(mainWindow, "Perform Depth Testing");
	chkHasDepthTesting->setChecked(true);

	chkHasFaceCulling = new nanogui::CheckBox(mainWindow, "Perform backface Culling");
	chkHasFaceCulling->setChecked(true);

	sldJuliaCX = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "JuliaC.X", std::make_pair(-1.0f, 1.0f), 0.45f, 2);
	sldJuliaCY = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "JuliaC.Y", std::make_pair(-1.0f, 1.0f), -0.3f, 2);
	sldJuliaZoom = nse::gui::AddLabeledSliderWithDefaultDisplay(mainWindow, "Julia Zoom", std::make_pair(0.01f, 10.0f), 1.0f, 2);

	performLayout();
}

// Create and define the vertex array and add a number of vertex buffers
void Viewer::CreateVertexBuffers()
{
	/*** Begin of task 3.2.3 ***
	Fill the positions-array and your color array with 12 rows, each
	containing 4 entries, to define a tetrahedron. */
	GLfloat positions[] = {
		// Face 1: Apex (0, 1, 0), Base (-1, -1, -1), Base (1, -1, -1)
		1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f, 1.0f,   // Apex
		1.0f, -1.0f, -1.0f, 1.0f, // Base vertex 1

		// Face 2: Apex (0, 1, 0), Base (1, -1, -1), Base (0, -1, 1)
		-1.0f, -1.0f, 1.0f, 1.0f,   // Apex
		1.0f, 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f, 1.0f,  // Base vertex 2

		// Face 3: Apex (0, 1, 0), Base (0, -1, 1), Base (-1, -1, -1)
		-1.0f, 1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f, 1.0f,   // Apex

		// Face 4: Base (-1, -1, -1), Base (1, -1, -1), Base (0, -1, 1)
		1.0f, 1.0f, 1.0f, 1.0f, // Base vertex 1
		1.0f, -1.0f, -1.0f, 1.0f    // Base vertex 3
		- 1.0f, 1.0f, -1.0f, 1.0f,
	};



	// Define colors for each face (one color per face)
	GLfloat colors[] = {
		// Face 1 (Red)
		1.0f, 0.0f, 0.0f, 1.0f, // Red
		1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,

		// Face 2 (Green)
		0.0f, 1.0f, 0.0f, 1.0f, // Green
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,

		// Face 3 (Blue)
		0.0f, 0.0f, 1.0f, 1.0f, // Blue
		0.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,

		// Face 4 (Yellow)
		1.0f, 1.0f, 0.0f, 1.0f, // Yellow
		1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f
	};



	// Generate the vertex array 
	glGenVertexArrays(1, &vertex_array_id);
	glBindVertexArray(vertex_array_id);

	// Generate a position buffer to be appended to the vertex array
	glGenBuffers(1, &position_buffer_id);
	// Bind the buffer for subsequent settings
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer_id);
	// Supply the position data
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
	// The buffer shall now be linked to the shader attribute
	// "in_position". First, get the location of this attribute in 
	// the shader program
	GLuint vid = glGetAttribLocation(program_id, "in_position");
	
	// Enable this vertex attribute array
	glEnableVertexAttribArray(vid);
	// Set the format of the data to match the type of "in_position"
	glVertexAttribPointer(vid, 4, GL_FLOAT, GL_FALSE, 0, 0);

	/*** Begin of task 3.2.2 (a) ***
	Create another buffer that will store color information. This works nearly
	similar to the code above that creates the position buffer. Store the buffer
	id into the variable "color_buffer_id" and bind the color buffer to the
	shader variable "in_color". */
	
	// Generate a color buffer to be appended to the vertex array
	glGenBuffers(1, &color_buffer_id);
	// Bind the buffer for subsequent settings
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer_id);
	// Supply the color data
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	// The buffer shall now be linked to the shader attribute
	// "in_colors". First, get the location of this attribute in 
	// the shader program
	GLuint cid = glGetAttribLocation(program_id, "in_color");

	// Enable this vertex attribute array
	glEnableVertexAttribArray(cid);
	// Set the format of the data to match the type of "in_position"
	glVertexAttribPointer(cid, 4, GL_FLOAT, GL_FALSE, 0, 0);


	
	/*** End of task 3.2.2 (a) ***/
	
	

	// Unbind the vertex array to leave OpenGL in a clean state
	glBindVertexArray(0);
}

//Checks if the given shader has been compiled successfully. Otherwise, prints an
//error message and throws an exception.
//  shaderId - the id of the shader object
//  name - a human readable name for the shader that is printed together with the error
void CheckShaderCompileStatus(GLuint shaderId, std::string name)
{
	GLint status;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);

	if (status != GL_TRUE)
	{
		char buffer[512];
		std::cerr << "Error while compiling shader \"" << name << "\":" << std::endl;
		glGetShaderInfoLog(shaderId, 512, nullptr, buffer);
		std::cerr << "Error: " << std::endl << buffer << std::endl;
		throw std::runtime_error("Shader compilation failed!");
	}
}

// Read, Compile and link the shader codes to a shader program
void Viewer::CreateShaders()
{
	// Convert shader source code to C-style strings
	std::string vs((char*)shader_vert, shader_vert_size);
	const char* vertex_content = vs.c_str();

	std::string fs((char*)shader_frag, shader_frag_size);
	const char* fragment_content = fs.c_str();

	/*** Begin of task 3.2.1 ***/

	// 1. Create a vertex shader object
	GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader_id, 1, &vertex_content, NULL); // Set source code
	glCompileShader(vertex_shader_id);                          // Compile vertex shader
	CheckShaderCompileStatus(vertex_shader_id, "Vertex Shader");                 // Check compilation status

	// 2. Create a fragment shader object
	GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader_id, 1, &fragment_content, NULL); // Set source code
	glCompileShader(fragment_shader_id);                            // Compile fragment shader
	CheckShaderCompileStatus(fragment_shader_id, "Fragment Shader");                   // Check compilation status

	// 3. Create a shader program
	GLuint program_id = glCreateProgram();

	// Attach shaders to the program
	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);

	// Link the program
	glLinkProgram(program_id);

	// Check for linking errors (optional, recommended)
	GLint success;
	glGetProgramiv(program_id, GL_LINK_STATUS, &success);
	if (!success) {
		GLchar infoLog[512];
		glGetProgramInfoLog(program_id, 512, NULL, infoLog);
		std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}

	// 4. Store shader IDs (optional, depending on your setup)
	this->vertex_shader_id = vertex_shader_id;
	this->fragment_shader_id = fragment_shader_id;
	this->program_id = program_id;

	/*** End of task 3.2.1 ***/
}


void Viewer::drawContents()
{
	Eigen::Vector2f juliaC(sldJuliaCX->value(), sldJuliaCY->value());
	float juliaZoom = sldJuliaZoom->value();

	//Get the transform matrices
	camera().ComputeCameraMatrices(modelViewMatrix, projectionMatrix);

	// If has_faceculling is set then enable backface culling
	// and disable it otherwise
	if (chkHasFaceCulling->checked())
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);

	// If has_depthtesting is set then enable depth testing
	// and disable it otherwise
	if (chkHasDepthTesting->checked())
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);

	// Activate the shader program
	glUseProgram(program_id);

	/*** Begin of task 3.2.4 (b) ***
	Set the shader variables for the modelview and projection matrix.
	First, find the location of these variables using glGetUniformLocation and
	then set them with the command glUniformMatrix4fv. 
	*/

	GLint modelViewLoc = glGetUniformLocation(program_id, "modelViewMatrix");
	GLint projectionLoc = glGetUniformLocation(program_id, "projectionMatrix");

	// Upload the modelView and projection matrices to the GPU
	glUniformMatrix4fv(modelViewLoc, 1, GL_TRUE, modelViewMatrix.data());
	glUniformMatrix4fv(projectionLoc, 1, GL_TRUE, projectionMatrix.data());


	// Bind the vertex array 
	glBindVertexArray(vertex_array_id);
	// Draw the tetrahedron (12 vertices, 4 faces, 3 vertices per face)
	glDrawArrays(GL_TRIANGLES, 0, 12);

	/*** End of task 3.2.4 (b) ***/
	
	// Unbind the vertex array
	glBindVertexArray(0);
	// Deactivate the shader program
	glUseProgram(0);
}
