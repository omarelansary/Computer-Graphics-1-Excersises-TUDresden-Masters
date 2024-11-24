// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Volume.h"

#include <iostream>

float ComputeVolume(const HEMesh& m)
{
    float vol = 0;
    /*Task 1.2.2*/
    for (auto face : m.faces())
    {
        std::vector<OpenMesh::Vec3f> vertices;
        for (auto vert : m.fv_range(face))
        {
            vertices.push_back(m.point(vert));
        }

        size_t numVertices = vertices.size();
        if (numVertices == 3) //face is a triangle
        {
            auto p0 = vertices[0]; 
            auto p1 = vertices[1]; 
            auto p2 = vertices[2]; 
            auto v1 = p1 - p0;
            auto v2 = p2 - p0;
            auto v3 = OpenMesh::Vec3f(0.0f, 0.0f, 0.0f) - p0; // Vector from the origin to p0
            auto crossProduct = v2.cross(v3);
            float tetrahedronVolume = v1.dot(crossProduct) / 6.0f;
            vol += tetrahedronVolume;
        }
    }
    vol = abs(vol);
    return vol;
}