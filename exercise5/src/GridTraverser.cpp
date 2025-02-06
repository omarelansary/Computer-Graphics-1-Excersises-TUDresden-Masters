// This source code is property of the Computer Graphics and Visualization 
// chair of the TU Dresden. Do not distribute! 
// Copyright (C) CGV TU Dresden - All Rights Reserved

#include "GridTraverser.h"
#include "GridUtils.h"


GridTraverser::GridTraverser()
{ }

GridTraverser::GridTraverser(const Eigen::Vector3f& o, const Eigen::Vector3f&d, const Eigen::Vector3f cell_extents)
	: orig(o), dir(d), cellExtents(cell_extents)
{
	dir.normalize();
	Init();
}

Eigen::Vector3f& GridTraverser::Origin()
{
	return orig;
}
const Eigen::Vector3f& GridTraverser::Origin() const
{
	return orig;
}

Eigen::Vector3f& GridTraverser::Direction()
{
	return dir;
}

const Eigen::Vector3f& GridTraverser::Direction() const
{
	return dir;
}

void GridTraverser::SetCellExtents(const Eigen::Vector3f& cellExtent)
{
	this->cellExtents = cellExtent;
	Init();
}

void GridTraverser::Init()
{
    current = PositionToCellIndex(orig, cellExtents);

    // Compute step direction for each axis (+1 or -1)
    step.x() = (dir.x() >= 0) ? 1 : -1;
    step.y() = (dir.y() >= 0) ? 1 : -1;
    step.z() = (dir.z() >= 0) ? 1 : -1;

    // Compute next cell boundary (tMax) and step size (tDelta)
    Eigen::Vector3f nextBoundary = (current.cast<float>().cwiseProduct(cellExtents) +
        step.cast<float>().cwiseMax(Eigen::Vector3f::Zero()).cwiseProduct(cellExtents));

    tMax.x() = (dir.x() != 0) ? (nextBoundary.x() - orig.x()) / dir.x() : std::numeric_limits<float>::infinity();
    tMax.y() = (dir.y() != 0) ? (nextBoundary.y() - orig.y()) / dir.y() : std::numeric_limits<float>::infinity();
    tMax.z() = (dir.z() != 0) ? (nextBoundary.z() - orig.z()) / dir.z() : std::numeric_limits<float>::infinity();

    tDelta.x() = (dir.x() != 0) ? std::abs(cellExtents.x() / dir.x()) : std::numeric_limits<float>::infinity();
    tDelta.y() = (dir.y() != 0) ? std::abs(cellExtents.y() / dir.y()) : std::numeric_limits<float>::infinity();
    tDelta.z() = (dir.z() != 0) ? std::abs(cellExtents.z() / dir.z()) : std::numeric_limits<float>::infinity();
}


void GridTraverser::operator++(int)
{
    // Traverse one step along the ray
    if (tMax.x() < tMax.y() && tMax.x() < tMax.z())
    {
        current.x() += step.x();
        tMax.x() += tDelta.x();
    }
    else if (tMax.y() < tMax.z())
    {
        current.y() += step.y();
        tMax.y() += tDelta.y();
    }
    else
    {
        current.z() += step.z();
        tMax.z() += tDelta.z();
    }
}

Eigen::Vector3i GridTraverser::operator*()
{
	return current;
}

	
