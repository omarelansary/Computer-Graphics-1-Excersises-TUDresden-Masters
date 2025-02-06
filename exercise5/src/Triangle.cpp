// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "Triangle.h"
#include "GridUtils.h"
#include <tuple>


//default constructor
Triangle::Triangle()
{
}
//constructs a triangle using the vertex positions v0,v1 and v2
Triangle::Triangle(const Eigen::Vector3f& v0, const Eigen::Vector3f& v1,const Eigen::Vector3f& v2): v0(v0),v1(v1),v2(v2)
{
}
//constructs a triangle from  the face f of the given halfedge mesh m
Triangle::Triangle(const HEMesh&m, const OpenMesh::FaceHandle& f):h(f)
{
	OpenMesh::HalfedgeHandle he = m.halfedge_handle(f);
	v0 = ToEigenVector(m.point(m.from_vertex_handle(he)));
	he = m.next_halfedge_handle(he);
	v1 = ToEigenVector(m.point(m.from_vertex_handle(he)));
	he = m.next_halfedge_handle(he);
	v2 = ToEigenVector(m.point(m.from_vertex_handle(he)));
}
//returns the smallest axis aligned bounding box of the triangle
Box Triangle::ComputeBounds() const
{
	/* Task 5.2.2 */
	Box b;		
	b.Insert(v0);
	b.Insert(v1);
	b.Insert(v2);
	return b;
}


// Returns true if the triangle overlaps the given box b
bool Triangle::Overlaps(const Box& b) const
{
	// Get the triangle vertices
	const Eigen::Vector3f& v0 = this->v0;
	const Eigen::Vector3f& v1 = this->v1;
	const Eigen::Vector3f& v2 = this->v2;

	// Get the box's lower and upper bounds
	const Eigen::Vector3f& boxMin = b.LowerBound();
	const Eigen::Vector3f& boxMax = b.UpperBound();

	// Helper function to project a point onto an axis
	auto projectPoint = [](const Eigen::Vector3f& point, const Eigen::Vector3f& axis) {
		return point.dot(axis);
	};

	// Helper function to project a triangle onto an axis
	auto projectTriangle = [&](const Eigen::Vector3f& axis) {
		float p0 = projectPoint(v0, axis);
		float p1 = projectPoint(v1, axis);
		float p2 = projectPoint(v2, axis);
		return std::make_pair(std::min({ p0, p1, p2 }), std::max({ p0, p1, p2 }));
	};

	// Helper function to project the box onto an axis
	auto projectBox = [&](const Eigen::Vector3f& axis) {
		Eigen::Vector3f center = b.Center();
		Eigen::Vector3f extents = b.HalfExtents();
		float projection = projectPoint(center, axis);
		float radius = extents[0] * std::abs(axis[0]) + extents[1] * std::abs(axis[1]) + extents[2] * std::abs(axis[2]);
		return std::make_pair(projection - radius, projection + radius);
	};

	// Helper function to check if two intervals overlap
	auto intervalsOverlap = [](const std::pair<float, float>& a, const std::pair<float, float>& b) {
		return !(a.second < b.first || b.second < a.first);
	};

	// Test the 3 axes of the AABB
	for (int i = 0; i < 3; ++i) {
		Eigen::Vector3f axis = Eigen::Vector3f::Zero();
		axis[i] = 1.0f;

		auto triangleProjection = projectTriangle(axis);
		auto boxProjection = projectBox(axis);

		if (!intervalsOverlap(triangleProjection, boxProjection)) {
			return false; // Separating axis found
		}
	}

	// Test the triangle's normal
	Eigen::Vector3f edge1 = v1 - v0;
	Eigen::Vector3f edge2 = v2 - v0;
	Eigen::Vector3f normal = edge1.cross(edge2).normalized();

	auto triangleProjection = projectTriangle(normal);
	auto boxProjection = projectBox(normal);

	if (!intervalsOverlap(triangleProjection, boxProjection)) {
		return false; // Separating axis found
	}

	// Test the 9 cross products of the triangle's edges and the AABB's axes
	Eigen::Vector3f edges[3] = { v1 - v0, v2 - v1, v0 - v2 };
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			Eigen::Vector3f axis = edges[i].cross(Eigen::Vector3f::Unit(j)).normalized();

			auto triangleProjection = projectTriangle(axis);
			auto boxProjection = projectBox(axis);

			if (!intervalsOverlap(triangleProjection, boxProjection)) {
				return false; // Separating axis found
			}
		}
	}

	// No separating axis found, the triangle and the box overlap
	return true;
}

//returns the barycentric coordinates of the point with the smallest distance to point p which lies on the triangle
void Triangle::ClosestPointBarycentric(const Eigen::Vector3f& p, float& l0, float& l1, float& l2) const
{
	Eigen::Vector3f edge0 = v1 - v0;
	Eigen::Vector3f edge1 = v2 - v0;
	Eigen::Vector3f v = v0 - p;

	float a = edge0.dot( edge0 );
	float b = edge0.dot( edge1 );
	float c = edge1.dot( edge1 );
	float d = edge0.dot( v );
	float e = edge1.dot( v );

	float det = a*c - b*b;
	float s = b*e - c*d;
	float t = b*d - a*e;

	if ( s + t < det )
	{
		if ( s < 0.f )
		{
			if ( t < 0.f )
			{
				if ( d < 0.f )
				{
					s=-d/a;
					s=std::min(std::max(s,0.0f),1.0f);
					t = 0.f;
				}
				else
				{
					s = 0.f;
					t = -e/c;
					t = std::min(std::max(t,0.0f),1.0f);
                 
				}
			}
			else
			{
				s = 0.f;
				t = -e/c;
				t = std::min(std::max(t,0.0f),1.0f);
			}
		}
		else if ( t < 0.f )
		{
			s =  -d/a;
			s=std::min(std::max(s,0.0f),1.0f);
			t = 0.f;
		}
		else
		{
			float invDet = 1.f / det;
			s *= invDet;
			t *= invDet;
		}
	}
	else
	{
		if ( s < 0.f )
		{
			float tmp0 = b+d;
			float tmp1 = c+e;
			if ( tmp1 > tmp0 )
			{
				float numer = tmp1 - tmp0;
				float denom = a-2*b+c;
				s = numer/denom;
				s=std::min(std::max(s,0.0f),1.0f);
				t = 1-s;
			}
			else
			{
				t = -e/c;
				t=std::min(std::max(t,0.0f),1.0f);
				s = 0.f;
			}
		}
		else if ( t < 0.f )
		{
			if ( a+d > b+e )
			{
				float numer = c+e-b-d;
				float denom = a-2*b+c;
				s = numer/denom;
				s=std::min(std::max(s,0.0f),1.0f);
               
				t = 1-s;
			}
			else
			{
				s =  -e/c;
				s=std::min(std::max(s,0.0f),1.0f);
				t = 0.f;
			}
		}
		else
		{
			float numer = c+e-b-d;
			float denom = a-2*b+c;

			s =  numer/denom;
			s=std::min(std::max(s,0.0f),1.0f);
			t = 1.f - s;
		}
	}
	l0 = 1-s-t;
	l1 = s;
	l2 = t;
}
//returns the point with smallest distance to point p which lies on the triangle
Eigen::Vector3f Triangle::ClosestPoint(const Eigen::Vector3f& p) const
{
	float l0,l1,l2;
	ClosestPointBarycentric(p,l0,l1,l2);
	return l0*v0 + l1*v1 +l2* v2;

}
//returns the squared distance between point p and the triangle
float Triangle::SqrDistance(const Eigen::Vector3f& p) const
{
	Eigen::Vector3f d = p-ClosestPoint(p);
	return d.squaredNorm();
}
//returns the euclidean distance between point p and the triangle
float Triangle::Distance(const Eigen::Vector3f& p) const
{
	return sqrt(SqrDistance(p));
}
//returns a reference point  which is on the triangle and is used to sort the primitive in the AABB tree construction
Eigen::Vector3f Triangle::ReferencePoint() const
{
	return (v0+v1+v2)/3.0f;
}



