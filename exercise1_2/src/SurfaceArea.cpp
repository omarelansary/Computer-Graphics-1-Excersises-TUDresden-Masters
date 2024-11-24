// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "SurfaceArea.h"

#include <iostream>

float ComputeSurfaceArea(const HEMesh& m)
{
    float area = 0.0f;
    for (auto face : m.faces())
    {
        std::vector<OpenMesh::Vec3f> vertices;
        for (auto vert : m.fv_range(face))
        {
            vertices.push_back(m.point(vert));
        }
        size_t numVertices = vertices.size();
        if (numVertices >= 3)
        {
            auto p0 = vertices[0];
            for (size_t i = 1; i < numVertices - 1; ++i)
            {
                auto p1 = vertices[i];
                auto p2 = vertices[i + 1];
                auto edge1 = p1 - p0;
                auto edge2 = p2 - p0;
                auto crossProduct = edge1.cross(edge2);
                //triangle area = 1/2* |cross product|
                float triangleArea = 0.5f * crossProduct.norm();
                area += triangleArea;
            }
        }
    }
    return area;
}







