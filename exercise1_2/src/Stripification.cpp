// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Stripification.h"

#include <random>
#include "sample_set.h"
#include <queue>

typedef OpenMesh::PolyMesh_ArrayKernelT<> HEMesh;
typedef OpenMesh::FaceHandle FH;
typedef OpenMesh::HalfedgeHandle HH;
typedef OpenMesh::VertexHandle VH;

// Function to calculate the length of an edge
double edgeLength(const HEMesh& mesh, const HEMesh::HalfedgeHandle& he) {
    const auto& p1 = mesh.point(mesh.from_vertex_handle(he)); // Start point of the edge
    const auto& p2 = mesh.point(mesh.to_vertex_handle(he));   // End point of the edge
    return (p1 - p2).length();  // Euclidean distance
}

// Function to get the longest edge (hypotenuse) of a triangle using a for loop
HEMesh::HalfedgeHandle getHypotenuse(const HEMesh& mesh, const HEMesh::FaceHandle& f) {
    // Start from the first halfedge of the triangle
    HEMesh::HalfedgeHandle he = mesh.halfedge_handle(f);  // First half-edge of the face
    HEMesh::HalfedgeHandle longest_he = he;
    double max_length = 0.0;

    // Traverse all three edges of the triangle (using next() to loop over the edges)
    for (int i = 0; i < 3; ++i) {
        double len = edgeLength(mesh, he);
        if (len > max_length) {
            max_length = len;
            longest_he = he;  // Update the longest edge
        }
        he = mesh.next_halfedge_handle(he);  // Move to the next half-edge
    }

    return longest_he;  // Return the longest edge
}

int traverseForwardStrip(HEMesh& mesh,
    OpenMesh::FPropHandleT<int> perFaceStripIdProperty,
    sample_set<OpenMesh::FaceHandle>& my_set, sample_set<OpenMesh::FaceHandle>& forward_set,
    HEMesh::HalfedgeHandle& hei_init,
    int nStrips,
    int parity)
{
    int nTriangles = 0;
    while (!mesh.is_boundary(hei_init) &&
        mesh.property(perFaceStripIdProperty, mesh.opposite_face_handle(hei_init)) == -1) {
        mesh.property(perFaceStripIdProperty, mesh.opposite_face_handle(hei_init)) = nStrips;
        forward_set.insert(mesh.opposite_face_handle(hei_init));
        //my_set.remove(mesh.opposite_face_handle(hei_init));
        // Move forward along the edge based on parity
        if (parity == 0) {
            hei_init = mesh.prev_halfedge_handle(mesh.opposite_halfedge_handle(hei_init));
        }
        else {
            hei_init = mesh.next_halfedge_handle(mesh.opposite_halfedge_handle(hei_init));
        }
        parity = 1 - parity;
        nTriangles++;
    }

    for (const auto& face : forward_set.elements) {
        mesh.property(perFaceStripIdProperty, face) = -1;
    }
    return nTriangles;
}

int traverseBackwardStrip(HEMesh& mesh,
    OpenMesh::FPropHandleT<int> perFaceStripIdProperty,
    sample_set<OpenMesh::FaceHandle>& my_set, sample_set<OpenMesh::FaceHandle>& backward_set,
    HEMesh::HalfedgeHandle& hei_init,
    int nStrips,
    int parity)
{
    int nTriangles = 0;
    while (!mesh.is_boundary(hei_init) &&
        mesh.property(perFaceStripIdProperty, mesh.opposite_face_handle(hei_init)) == -1) {
        mesh.property(perFaceStripIdProperty, mesh.opposite_face_handle(hei_init)) = nStrips;
        backward_set.insert(mesh.opposite_face_handle(hei_init));

        // Move backward along the edge based on parity
        if (parity == 0) {
            hei_init = mesh.opposite_halfedge_handle(mesh.prev_halfedge_handle(hei_init));
        }
        else {
            hei_init = mesh.opposite_halfedge_handle(mesh.next_halfedge_handle(hei_init));
        }
        parity = 1 - parity;
        nTriangles++;
    }

    // Reset the property of the faces in the backward set
    for (const auto& face : backward_set.elements) {
        mesh.property(perFaceStripIdProperty, face) = -1;
    }
    return nTriangles;
}

void findStartFace(
    HEMesh& mesh,
    OpenMesh::FPropHandleT<int> perFaceStripIdProperty,
    sample_set<OpenMesh::FaceHandle>& my_set,
    std::mt19937& eng,
    unsigned int nTrials,
    int& max_traingles,
    OpenMesh::FaceHandle& start_face,
    char& direction, sample_set<OpenMesh::FaceHandle>& forward_set, sample_set<OpenMesh::FaceHandle>& backward_set,
    int nStrips)
{
    // Run the specified number of trials
    for (unsigned int trial = 0; trial < nTrials; ++trial) {
        if (my_set.empty()) {
            break;  // If no more faces to sample from, stop.
        }
        int parity = 0;

        // Sample a random face from the set
        OpenMesh::FaceHandle random_face = my_set.sample(eng);
        if (mesh.property(perFaceStripIdProperty, random_face) == -1) {
            // Increment strip index
            nStrips++;

            // Get the longest edge (hypotenuse) of the random face to start
            HEMesh::HalfedgeHandle hei_init = getHypotenuse(mesh, random_face);

            // Initialize temp_Sets
            sample_set<OpenMesh::FaceHandle> forward_trial_set;
            sample_set<OpenMesh::FaceHandle> backward_trial_set;

            // Traverse the half-edges of the current strip (FORWARD)
            int forward_triangles = traverseForwardStrip(mesh, perFaceStripIdProperty, my_set, forward_trial_set, hei_init, nStrips, parity);
            if (!forward_set.empty()) {
                std::cout << forward_triangles << std::endl;
            }

            // Reverse traversal (BACKWARD) to create the strip in the opposite direction
            parity = 0; // Reset parity for backward direction
            int backward_triangles = traverseBackwardStrip(mesh, perFaceStripIdProperty, my_set, backward_trial_set, hei_init, nStrips, parity);
            if (!backward_set.empty()) {
                std::cout << backward_triangles << std::endl;
            }

            if (forward_triangles >= backward_triangles)
            {
                // Compare forward and backward triangles to determine which direction has more triangles
                if (forward_triangles > max_traingles) {
                    forward_set = forward_trial_set;
                    max_traingles = forward_triangles;
                    start_face = random_face;  // Set start face
                    direction = 'f';  // Set direction to forward
                }
            }
            else {
                if (backward_triangles > max_traingles) {
                    backward_set = backward_trial_set;
                    max_traingles = backward_triangles;
                    start_face = random_face;  // Set start face
                    direction = 'b';  // Set direction to backward
                }
            }
        }
    }
}


unsigned int ExtractTriStrips(HEMesh& mesh, OpenMesh::FPropHandleT<int> perFaceStripIdProperty, unsigned int nTrials)
{
    // Prepare random engine
    std::mt19937 eng(std::random_device{}());
    //int nStrips = 0;

    // Initialize strip index to -1 for each face
    sample_set<OpenMesh::FaceHandle> my_set;
    my_set.reserve(mesh.n_faces());
    for (auto f : mesh.faces()) {
        mesh.property(perFaceStripIdProperty, f) = -1;
        my_set.insert(f);
    }

    int nStrips = 0;
    // Run the specified number of trials
    while (!my_set.empty()) {

        int max_traingles = -1;
        OpenMesh::FaceHandle start_face;
        char direction = 'n';
        int parity = 0;


        // Initialize temp_Sets
        sample_set<OpenMesh::FaceHandle> forward_set;
        sample_set<OpenMesh::FaceHandle> backward_set;
        // Sample a random face from the set
        findStartFace(mesh, perFaceStripIdProperty, my_set, eng, nTrials, max_traingles, start_face, direction, forward_set, backward_set, nStrips);
        OpenMesh::FaceHandle random_face = start_face;

        if (mesh.property(perFaceStripIdProperty, random_face) == -1) {
            if (direction == 'f')
            {
                for (const auto& face : forward_set.elements) {
                    mesh.property(perFaceStripIdProperty, face) = nStrips;
                    my_set.remove(face);
                }
            }
            else if (direction == 'b')
            {
                for (const auto& face : backward_set.elements) {
                    mesh.property(perFaceStripIdProperty, face) = nStrips;
                    my_set.remove(face);
                }
            }
            nStrips++;
        }
    }

    return nStrips;
}



