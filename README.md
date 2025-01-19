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

# Exercise 4 Output: Infinite Procedural Terrain

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

**Image Placeholder: Geometry for the Terrain**
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

**Image Placeholder: Simple Grass Texture**
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

**Image Placeholder: Grass to Rock Transition**
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

**Image Placeholder: Specular Highlights & Roads with Alpha Map on Roads**
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

**Image Placeholder: Frustum Culling for Terrain & Instanced Terrain Rendering**
![4 2 5_a_b_1](https://github.com/user-attachments/assets/01818a79-458e-4dfc-97bc-414d62dddf92)
![4 2 5_a_b_2](https://github.com/user-attachments/assets/99136afa-61c3-4a89-ba9b-35ca9d337d59)
![4 2 5_a_b_3](https://github.com/user-attachments/assets/619b22a2-676b-4562-b5c5-1b19aabef7eb)


