// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include <map>
#include <queue>
#include "util/UnionFind.h"
#include "ShellExtraction.h"


unsigned int ExtractShells(HEMesh& m, OpenMesh::FPropHandleT<int> perFaceShellIndex)
{
    //reset the shell indices to -1 for every face
    for (auto f : m.faces())
        m.property(perFaceShellIndex, f) = -1;

    /*Task 2.2.1*/
    unsigned int shellIndex = 0;  // Counter for shells
    for (auto f : m.faces()) {
        if (m.property(perFaceShellIndex, f) == -1) {//if we arrived to this at a second iteration => second shell
            std::queue<HEMesh::FaceHandle> faceQueue;
            faceQueue.push(f);
            m.property(perFaceShellIndex, f) = shellIndex; //mark it as visited (mark it with the shell index)

            while (!faceQueue.empty()) {
                HEMesh::FaceHandle currentFace = faceQueue.front();
                faceQueue.pop();

                for (auto he : m.fh_range(currentFace)) {// Iterate over all half-edges of the current face (for example if it is triangle then 3 half-edges)
                    HEMesh::FaceHandle neighborFace = m.opposite_face_handle(he); //adjacent face

                    if (!m.is_valid_handle(neighborFace)) continue; // valid face (boundry)
                    if (m.property(perFaceShellIndex, neighborFace) != -1) continue;// unvisited

                    m.property(perFaceShellIndex, neighborFace) = shellIndex; //mark it as visited (mark it with the shell index)
                    faceQueue.push(neighborFace);
                }
            }
            //new shell
            shellIndex++;
        }
    }
    return shellIndex;
}
