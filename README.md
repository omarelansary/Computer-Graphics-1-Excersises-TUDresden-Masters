# Computer Graphics 1 - Exercises

This repository contains the source code and data sets for the CG1 exercises. To download and compile the project, you need to do the following steps:

1. Clone the project to your local computer
2. Run CMake to generate a project file for your build platform. Alternatively, you can use a development environment that supports CMake natively and open the directory directly.
3. Open the project file and compile.

## Unix Instructions

Under Unix, you could employ the command line like follows:
```bash
git clone https://bitbucket.org/cgvtud/cg1.git --recursive
cd cg1
mkdir build
cd build
cmake ..
make
```

After this, you find the compiled applications under `cg1/build/bin`.

If you are missing some dependencies you can install them on Debian using:
```bash
sudo apt install git make cmake libxxf86vm1 libxrandr2 libxinerama1 libxcursor1 libx11-6 libc6 libstdc++6 libgcc-8-dev libxext6	libxrender1 libxfixes3 libxcb1 libxau6 libxdmcp6 libbsd0
```
Note: This list may look long but most of these packages are likely already installed on your system.

## Windows Instructions
	
On Windows / Visual Studio, you can use the CMake GUI. After cloning the repository, open the CMake GUI and set *Where is the source code* to the cloned directory.
Set *Where to build the binaries* to a new subfolder `build`.
Click *Configure* and select the desired Visual Studio Version. Then, click *Configure*  and *Open Project* to generate and open the project in Visual studio.
Finally, choose the desired startup project (Exercise1-4) and compile and run.

# Tasks 1+2 Output: Mesh Processing with Halfedge Data Structures

This part of the project focuses on mesh processing using OpenMesh.

## Task 1: Polygon Mesh Processing

### 1.1 Mesh Generation
- **Task**: Implement the creation of a unit cube in C++.
- **Details**: Use OpenMesh for mesh generation and implement the function `CreateCube` in `Primitives.cpp`.

### 1.2 Surface Area and Volume Calculation
- **Task**: Implement surface area and volume calculation for triangle meshes.
  - **(a)** Calculate surface area in `ComputeSurfaceArea`.
  - **(b)** Extend to arbitrary polygon meshes.
  - **(c)** Calculate the enclosed volume of closed triangle meshes in `ComputeVolume`.

### 1.3 Computing Vertex Valence
- **Task**: Compute vertex valence, defined as the number of edges incident to a vertex.
  - **(a)** Register a custom attribute for vertex valence.
  - **(b)** Compute vertex face valence using face incidence.
  - **(c)** Compute vertex vertex valence for meshes with boundaries.

**Image: Console Output for Torus Vertex Valence**
![image](https://github.com/user-attachments/assets/9e45cf5e-91d7-4e71-88a5-2cd0d4ef88a7)

---

## Task 2: Mesh Processing with Halfedge Data Structures

### 2.1 Connected Component Analysis
- **Task**: Implement connected component extraction in `ShellExtraction.cpp`.
  - **Details**: Use either region growing or union-find to extract connected components and assign shell ids to faces.

### 2.2 Triangle Stripification
- **Task**: Implement triangle strip extraction using the greedy algorithm in `Stripification.cpp`.
  - **Details**: Extract triangle strips from the mesh and assign strip ids to faces. Ensure tessellation consistency for OpenGL rendering.

### 2.3 Smoothing of Vertex Positions
- **Task**: Implement Laplace-Beltrami smoothing with both uniform and cotangent discretization.
  - **(a)** Implement uniform discretization smoothing in `Smoothing.cpp`.
  - **(b)** Implement cotangent discretization smoothing. Ensure the update rule applies correctly and that smoothing is not done in-place to prevent overwriting mesh information.

**Video: Operations Implemented in Tasks 1 and 2**
https://github.com/user-attachments/assets/e76f29e9-e6ef-42d6-b3ba-cb0425354b40

---

# Task 3 Output: Infinite Procedural Terrain
# Task 3: Realtime Rendering

This task involves loading and compiling shader programs, augmenting shaders to handle vertex colors, defining a tetrahedron, and implementing object transformations with perspective projections and texturing using a Julia Fractal.

## 3.2.1 Loading, Compiling, and Linking Shader Programs
- **Objective**: Load, compile, and link vertex and fragment shaders.
- **Steps**:
  1. Load shaders from `glsl` folder.
  2. Compile and link them into a shader program.
  3. Check for compilation errors.
  
  **Image: Triangle colored by its 3D position**
  ![image](https://github.com/user-attachments/assets/9489184d-8dc1-4bc9-8f2a-512bb7f87721)


## 3.2.2 Augmenting the VAO and Extending Shaders

### (a) Create Color Buffer
- **Objective**: Add a color buffer to the VAO for each vertex.
- **Steps**:
  1. Create a color buffer with values: red, blue, and green.
  2. Bind this buffer to the `in_color` attribute in the shader.

### (b) Use Color Attribute in Shader
- **Objective**: Modify shaders to read and apply color data.
- **Steps**:
  1. In the vertex shader, pass the color to the fragment shader.
  2. In the fragment shader, use the passed color data.

  **Image: Triangle vertices colored red, blue, and green**
   ![image](https://github.com/user-attachments/assets/5c8df946-8a79-452c-9887-106fbadde4c4)
  

## 3.2.3 and 3.2.4 Implementing Transforms

### (a) Use Matrices in the Shader
- **Objective**: Apply model-view and projection transformations in the vertex shader.
- **Steps**:
  1. Create two `mat4` uniform variables in the vertex shader.
  2. Use the matrices for transformation and projection.

### (b) Set Matrices from CPU Code
- **Objective**: Transfer matrices to the GPU.
- **Steps**:
  1. Use `glGetUniformLocation` and `glUniformMatrix4fv` to upload the matrices.
  2. Ensure the correct transformation of the tetrahedron in the scene.

  **Video: Tetrahedron with proper transformations visible and manipulable with the mouse**
https://github.com/user-attachments/assets/f359a2eb-4bad-464f-b3ee-70c59b5bfedc

 

## 3.2.5 Texturing with the Julia Fractal
- **Objective**: Texture the tetrahedron with the Julia Fractal.
- **Steps**:
  1. Implement Julia Fractal algorithm in the fragment shader.
  2. Set values for `c` and `m` (e.g., `c = (0.45, -0.3)`, `m = 1`).
  3. Pass untransformed vertex positions from the vertex shader to the fragment shader.
  
  **Iamge: Tetrahedron colored by the Julia Fractal pattern**
https://github.com/user-attachments/assets/adb7afce-1d5e-4b6b-b5c3-740874cbf5b1



---

# Task 4 Output: Infinite Procedural Terrain

This part provides an overview of the steps to implement an infinite procedural terrain, as outlined in Task 4 of the exercise. The implementation involves generating terrain geometry, applying procedural height maps, adding textures, and rendering an infinite landscape using instanced rendering.

---

## 4.2.1 Geometry for the Terrain
### Objective
Create a tessellated flat square patch geometry using vertex and index buffers.

### Implementation
- Generate positions and indices for a flat square patch in the `CreateGeometry` function (Viewer.cpp).
- Use triangle strips with degenerate triangles or a primitive restart index.
- Ensure the patch extends along the xz-plane, starting at the origin and lying in the positive quadrant.

### Expected Output
A flat gray plane appears in the scene.

**Image: Geometry for the Terrain**
![4 2 1](https://github.com/user-attachments/assets/144539c9-2eb7-4ce7-a8b4-8ac69c94ec84)

---

## 4.2.3 Simple Texturing
### Objective
Apply a repeating grass texture to the terrain.

### Implementation
- Create and configure a texture object in the `CreateTexture` function (Viewer.cpp).
- Use scaled xz world-space coordinates to sample the texture in the fragment shader (`terrain.frag`).

### Expected Output
A shaded terrain covered with a repeating grass texture.

**Image: Simple Grass Texture**
![4 2 3](https://github.com/user-attachments/assets/cd6eefcc-9aed-4e41-b3f3-ecbec74ae671)

---

## 4.2.4 Advanced Texturing
### (a) Rock Texture for Steep Slopes
#### Objective
Blend grass and rock textures based on terrain steepness.

#### Implementation
- Add a rock texture to the fragment shader.
- Use the terrain's normal vector to determine steep slopes and blend the textures smoothly with the GLSL `mix` function.

#### Expected Output
Grass texture transitions to rock texture on steep slopes.

**Image: Grass to Rock Transition**
![4 2 4_a](https://github.com/user-attachments/assets/a91a3a7b-268f-4c6e-af00-904613b61004)

---

### (b) Alpha Map for Roads
#### Objective
Overlay a road texture on the terrain using an alpha map.

#### Implementation
- Use the alpha map to define road regions on the terrain.
- Scale texture coordinates appropriately for the alpha map to ensure accurate placement.

#### Expected Output
Roads appear in specified regions of the terrain.

---

### (c) Specular Highlights for Roads
#### Objective
Add localized specular highlights to the road using a specular map.

#### Implementation
- Utilize the `roadSpecularMap` to define areas with specular intensity.
- Pass camera position to the fragment shader to compute realistic specular reflections.

#### Expected Output
Shiny highlights on the road surface under lighting.

**Image: Specular Highlights & Roads with Alpha Map on Roads**
![4 2 4_b_c](https://github.com/user-attachments/assets/a3571842-7319-4ef4-a47f-95f2c464fea6)

---

## 4.2.5 Infinite Terrain
### (a) Instanced Rendering
#### Objective
Render multiple instances of the terrain patch with distinct offsets.

#### Implementation
- Add an offset attribute to the vertex shader and configure it as an instance attribute.
- Use instanced rendering to draw multiple terrain patches.

#### Expected Output
The scene displays multiple instances of the terrain patch.

---

### (b) View Frustum Culling
#### Objective
Optimize rendering by displaying only terrain patches visible to the camera.

#### Implementation
- Perform frustum culling to determine visible patches based on the view and camera position.
- Load offsets of visible patches into the buffer for rendering.

#### Expected Output
An efficient, infinite terrain with only visible patches rendered.

**Image: Frustum Culling for Terrain & Instanced Terrain Rendering**
![4 2 5_a_b_1](https://github.com/user-attachments/assets/01818a79-458e-4dfc-97bc-414d62dddf92)
![4 2 5_a_b_2](https://github.com/user-attachments/assets/99136afa-61c3-4a89-ba9b-35ca9d337d59)
![4 2 5_a_b_3](https://github.com/user-attachments/assets/619b22a2-676b-4562-b5c5-1b19aabef7eb)


