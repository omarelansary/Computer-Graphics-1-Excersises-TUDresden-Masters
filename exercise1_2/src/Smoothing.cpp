// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include <memory>
#include <random>
#include "Smoothing.h"


void SmoothUniformLaplacian(HEMesh& m, float lambda)
{
    // Step 1: Create a temporary storage for the new positions of the vertices
    std::vector<OpenMesh::Vec3f> new_positions(m.n_vertices());

    // Step 2: Loop over all vertices and compute their new position
    for (auto vh : m.vertices()) {
        OpenMesh::Vec3f avg_position(0.0f, 0.0f, 0.0f);
        int neighbor_count = 0;

        // Step 2.1: Compute the average of the neighboring vertices (uniform Laplacian)
        for (auto hv : m.vv_range(vh)) {
            avg_position += m.point(hv);  // Add the neighbor's position
            neighbor_count++;  // Count the number of neighbors
        }

        // Step 2.2: Compute the new position by averaging the neighbors' positions
        if (neighbor_count > 0) {
            avg_position /= neighbor_count;  // Compute the average of neighbors
        }

        // Step 2.3: Update the new position based on the Laplacian formula
        new_positions[vh.idx()] = m.point(vh) + lambda * (avg_position - m.point(vh));
    }

    // Step 3: Apply the new positions to the mesh
    for (auto vh : m.vertices()) {
        m.set_point(vh, new_positions[vh.idx()]);
    }
}

float cotangent(const OpenMesh::Vec3f& p0, const OpenMesh::Vec3f& p1, const OpenMesh::Vec3f& p2)
{
    OpenMesh::Vec3f v1 = p1 - p0;
    OpenMesh::Vec3f v2 = p2 - p0;
    float cos_angle = OpenMesh::dot(v1, v2);
    float sin_angle = OpenMesh::cross(v1, v2).norm();

    if (sin_angle < 1e-6f) { // Prevent division by zero or very small values
        std::cerr << "Warning: Very small sin_angle, clamping to 1e-6\n";
        sin_angle = 1e-6f;
    }

    float cot = cos_angle / sin_angle;

    // Clamp cotangent to avoid extreme values
    if (std::abs(cot) > 1e6f) {
        std::cerr << "Warning: Extreme cotangent value, clamping to 1e6\n";
        cot = (cot > 0) ? 1e6f : -1e6f;
    }

    return cot;
}

void SmoothCotanLaplacian(HEMesh& m, float lambda)
{
    std::vector<OpenMesh::Vec3f> new_positions(m.n_vertices());

    for (auto vh : m.vertices()) {
        if (m.is_boundary(vh)) {
            new_positions[vh.idx()] = m.point(vh);
            continue;
        }

        OpenMesh::Vec3f laplacian(0.0f, 0.0f, 0.0f);
        float area_sum = 0.0f;

        for (auto he : m.voh_range(vh)) {
            auto neighbor = m.to_vertex_handle(he);

            auto he_next = m.next_halfedge_handle(he);
            auto he_prev = m.opposite_halfedge_handle(m.next_halfedge_handle(m.opposite_halfedge_handle(he)));

            auto p0 = m.point(vh);
            auto p1 = m.point(neighbor);
            auto p2 = m.point(m.to_vertex_handle(he_next));
            auto p3 = m.point(m.to_vertex_handle(he_prev));

            float cot_alpha = cotangent(p0, p2, p1);
            float cot_beta = cotangent(p0, p3, p1);

            if (std::isnan(cot_alpha) || std::isnan(cot_beta)) {
                std::cerr << "Warning: Invalid cotangent value for vertex " << vh.idx() << "\n";
                continue;
            }

            float weight = cot_alpha + cot_beta;

            if (std::isnan(weight) || std::isinf(weight)) {
                std::cerr << "Warning: Invalid weight for edge (" << vh.idx() << ", " << neighbor.idx() << ")\n";
                continue;
            }

            laplacian += weight * (p1 - p0);
            area_sum += weight;
        }

        if (area_sum < 1e-6f) {
            std::cerr << "Warning: Very small area_sum for vertex " << vh.idx() << ", clamping to 1e-6\n";
            area_sum = 1e-6f;
        }

        laplacian /= (2.0f * area_sum);

        if (std::isnan(laplacian[0]) || std::isnan(laplacian[1]) || std::isnan(laplacian[2])) {
            std::cerr << "Warning: Invalid Laplacian for vertex " << vh.idx() << ", resetting to zero\n";
            laplacian = OpenMesh::Vec3f(0.0f, 0.0f, 0.0f);
        }

        new_positions[vh.idx()] = m.point(vh) + lambda * laplacian;

        // Check new position validity
        auto& new_pos = new_positions[vh.idx()];
        if (std::isnan(new_pos[0]) || std::isnan(new_pos[1]) || std::isnan(new_pos[2])) {
            std::cerr << "Warning: Invalid new position for vertex " << vh.idx() << ", resetting to original\n";
            new_positions[vh.idx()] = m.point(vh);
        }
    }

    for (auto vh : m.vertices()) {
        m.set_point(vh, new_positions[vh.idx()]);
    }
}


void AddNoise (HEMesh &m, OpenMesh::MPropHandleT<Viewer::BBoxType> bbox_prop)
{
	std::mt19937 rnd;
	std::normal_distribution<float> dist;

	for (auto v : m.vertices())
	{
		OpenMesh::Vec3f n;
		m.calc_vertex_normal_correct(v, n);
		const auto diag = m.property(bbox_prop).diagonal();
		const float base_diag = std::min(diag.x(), std::min(diag.y(), diag.z())) / 20.f;
		float base_nb=0, nb_num=0;
		for (auto vnb : m.vv_range(v))
		{
			base_nb += (m.point(v) - m.point(vnb)).norm();
			nb_num++;
		}
		base_nb /= 4.f * nb_num;

		m.point(v) += std::min(base_diag, base_nb) * dist(rnd) * n.normalized();
	}
}
