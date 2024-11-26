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


unsigned int ExtractTriStrips(HEMesh& mesh, OpenMesh::FPropHandleT<int> perFaceStripIdProperty, unsigned int nTrials)
{
	/* Prepare random engine*/
    std::mt19937 eng(std::random_device{}());

    // Initialize strip index to -1 for each face
    sample_set<OpenMesh::FaceHandle> my_set;
    my_set.reserve(mesh.n_faces());
    for (auto f : mesh.faces()) {
        mesh.property(perFaceStripIdProperty, f) = -1;
        my_set.insert(f);
    }
    int nStrips = 0;
  
    HEMesh::HalfedgeHandle hei_init = mesh.halfedge_handle(mesh.faces_begin());
    //mesh.property(perFaceStripIdProperty, mesh.faces_begin()) = 3;
    int parity = 0;
    for (unsigned int trial = 0; trial < nTrials; ++trial) {
        OpenMesh::FaceHandle random_face = my_set.sample(eng);
        if (mesh.property(perFaceStripIdProperty, random_face) == -1) {
            nStrips++;
            // Get the HalfedgeHandle associated with this random face (assuming I want to start from any halfedge)
            hei_init = mesh.halfedge_handle(random_face);
            //mesh.property(perFaceStripIdProperty, random_face) = 100;
            while (mesh.property(perFaceStripIdProperty, mesh.opposite_face_handle(hei_init)) == -1)
            {
                mesh.property(perFaceStripIdProperty, mesh.opposite_face_handle(hei_init)) = nStrips;
                my_set.remove(mesh.opposite_face_handle(hei_init));

                // // FORWARD
                //if (parity == 0) {
                //    hei_init = mesh.prev_halfedge_handle(mesh.opposite_halfedge_handle(hei_init));
                //}
                //else {
                //    hei_init = mesh.next_halfedge_handle(mesh.opposite_halfedge_handle(hei_init));
                //}
                //parity = 1 - parity;
                
                // // BACKWARD
                if (parity == 0) {
                    hei_init = mesh.opposite_halfedge_handle(mesh.prev_halfedge_handle(hei_init));
                }
                else {
                    hei_init = mesh.opposite_halfedge_handle(mesh.next_halfedge_handle(hei_init));
                }
                parity = 1 - parity;
            }
            
        }
    }
    return nStrips;
}





