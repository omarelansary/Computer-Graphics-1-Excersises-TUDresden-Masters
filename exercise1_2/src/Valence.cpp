// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include <iostream>
#include <set>
#include <unordered_map>
#include <memory>
#include "Valence.h"


// Add the properties storing vertex valences to the given mesh
VertexValenceProperties AddValenceProperties (HEMesh &m)
{
	/* Task 1.2.3 */
	/* Add your properties to the mesh and return their handles. */
	// Create a VertexValenceProperties container to hold the property handles
	VertexValenceProperties valenceProps;

	// Add a property to store the number of faces connected to each vertex
	m.add_property(valenceProps.faceValences, "face_valence");
	m.add_property(valenceProps.vertexValences, "vertex_valence");

	// Return the property handles inside a VertexValenceProperties container
	return valenceProps;
}

// Compute a histogram for the given vertex valence property
ValenceHistogram ComputeValenceHistogram (const HEMesh &m, const VertexValenceProperty valence)
{
	ValenceHistogram ret;
	/* Task 1.2.3 - create a histogram of vertex valences from the values stored in your
	                custom mesh property. */
		// Compute a histogram for the given vertex valence propert
		// Iterate over each vertex in the mesh
		for (auto v : m.vertices())
		{
			// Get the valence (number of incident faces) of the vertex
			int vertexValence = m.property(valence, v);

			// Increment the count for this valence in the histogram
			ret[vertexValence]++;
		}

		return ret;
}

// Computes the per-vertex face incidence count (aka. vertex face valences) for the given mesh,
// using the indicated property to store the results
void ComputeVertexFaceValences(HEMesh& m, const VertexValenceProperty valence)
{
	// Loop through all vertices in the mesh
	for (auto v : m.vertices())
	{
		int faceCount = 0; // Initialize a counter for incident faces

		// Loop over all faces around this vertex
		for (auto f : m.vf_range(v)) // vf_range(v) gives the incident faces of vertex v
		{
			faceCount++; // Increment the counter for each incident face
		}

		// Store the face count in the custom valence property for this vertex
		m.property(valence, v) = faceCount;
	}
}


// Computes the vertex valences for the given mesh, using the indicated property to store the results
void ComputeVertexVertexValences (HEMesh &m, const VertexValenceProperty valence)
{
	using SetOfVertices = std::set<OpenMesh::VertexHandle>;
	using VertexAdjacencyMap = std::unordered_map<OpenMesh::VertexHandle, SetOfVertices>;

	/* Task 1.2.3 - compute vertex valences using only simple-mesh capabilities and store them
	                in the given custom mesh property.
	   Hint 1: to replicate the ordered list of face vertices you have in a simple mesh data structure,
	           iterate through the edges of a face - for example by using the SmartFaceHandle::edges()
	           range object!
	   Hint 2: OpenMesh smart handles are automatically down-casted to ordinary handles, so you can
	           directly use the two above helper types SetOfVertices and VertexAdjacencyMap with smart
	           handles also, if you decide to use the smart handle APIs.*/
	
	// Loop through all vertices in the mesh
	for (auto v : m.vertices())
	{
		int vertexValence = 0;  // Initialize counter for adjacent vertices (1-ring)

		// Iterate through all half-edges around the vertex
		for (auto vh : m.vv_range(v))  // vv_range(v) gives all vertices adjacent to v
		{
			vertexValence++;  // Count each adjacent vertex
		}

		// Store the valence in the custom valence property
		m.property(valence, v) = vertexValence;
	}
}
