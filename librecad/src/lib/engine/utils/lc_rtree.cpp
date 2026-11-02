/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**

** Copyright (C) 2024 LibreCAD.org
** Copyright (C) 2024 Dongxu Li (dongxuli2011@gmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**********************************************************************/


#include "lc_rtree.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/numeric/ublas/matrix.hpp>

#include "lc_rect.h"
#include "rs.h"
#include "rs_vector.h"

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
using BPoint = bg::model::point<double, 2, bg::cs::cartesian>;
using BBox = bg::model::box<BPoint>;
using TreeValue = std::pair<BBox, uintptr_t>;

namespace lc {

namespace geo {

struct RTree::RTreeImpl: bgi::rtree< TreeValue, bgi::quadratic<16> >
{
    RTreeImpl(const double tolerance)
        : m_tolerance{tolerance}
        , m_halfBox{0.5 * m_tolerance, 0.5 * m_tolerance} {}

    RTreeImpl(const std::vector<TreeValue>& values, const double tolerance)
        : bgi::rtree<TreeValue, bgi::quadratic<16>>(values.begin(), values.end())
        , m_tolerance{tolerance}
        , m_halfBox{0.5 * m_tolerance, 0.5 * m_tolerance} {}

    BPoint toPoint(const RS_Vector& point) const
    {
        return {point.x, point.y};
    }

    static RS_Vector toPoint(const BBox& box)
    {
        const auto min = box.min_corner();
        const auto max = box.max_corner();
        const auto center = RS_Vector{min.get<0>()+max.get<0>(), min.get<1>()+max.get<1>() } * 0.5;
        return center;
    }

    BBox toBox(const RS_Vector& point) const
    {
        return {toPoint(point - m_halfBox), toPoint(point + m_halfBox)};
    }

    BBox toBox(const RS_Vector& min, const RS_Vector& max) const {
        return {toPoint(min), toPoint(max)};
    }

    BBox toBox(const LC_Rect& rect) const
    {
        return {toPoint(rect.lowerLeftCorner()), toPoint(rect.upperRightCorner())};
    }

    RS_VectorSolutions Intersects(const BBox& box) const
    {
        std::vector<TreeValue> result_s;
        query(bgi::intersects(box), std::back_inserter(result_s));
        RS_VectorSolutions ret;
        for(const auto& [bbox, index]: result_s) {
            ret.push_back(toPoint(bbox));
        }
        return ret;
    }

    double m_tolerance = RS_TOLERANCE;
    RS_Vector m_halfBox{false};
};

/**
 * @brief Provide the R-Tree container
 *        R-Tree is an efficient container with spatial indexing
 *        The current implementation only uses a value pair of {Rect, index}
 *        with the Rect as a bounding box and index an unsigned number
 *        as the counting number of nodes.
 *        The implemented query methods:
 *          NearestNeighbors: return all nodes intersect a tolerance sized box
 *          PointsInBox: return all node box centers intersect a given box
 * @author Dongxu Li
 */
/**
     * @brief RTree Constructor of a RTree for efficient point look up
     * @param toleranceSize - the default box size to use to find nearest points,
     *                        i.e. the node box centers
     */
RTree::RTree(double toleranceSize):
    m_pRTree{std::make_unique<RTreeImpl>(toleranceSize)}
{}


RTree::RTree(const std::vector<std::tuple<RS_Vector, RS_Vector, uintptr_t>>& items, double toleranceSize) {
    std::vector<TreeValue> treeValues;
    treeValues.reserve(items.size());
    const double halfTol = 0.5 * toleranceSize;
    const RS_Vector halfBox{halfTol, halfTol};

    for (const auto& [min, max, payload] : items) {
        const BPoint pMin{min.x, min.y};
        const BPoint pMax{max.x, max.y};
        treeValues.emplace_back(BBox{pMin, pMax}, payload);
    }
    m_pRTree = std::make_unique<RTreeImpl>(treeValues, toleranceSize);
}

    RTree::~RTree() = default;

bool RTree::insert(const RS_Vector& min, const RS_Vector& max, uintptr_t payload) {
    if (m_pRTree == nullptr) {
        return false;
    }
    const BBox b = m_pRTree->toBox(min, max);
    m_pRTree->insert({b, payload});
    return true;
}

bool RTree::insert(const Area& area, uintptr_t payload) {
    if (m_pRTree == nullptr) {
        return false;
    }
    const BBox b = m_pRTree->toBox(area);
    m_pRTree->insert({b, payload});
    return true;
}

bool RTree::remove(const RS_Vector& min, const RS_Vector& max, uintptr_t payload) {
    if (m_pRTree == nullptr) {
        return false;
    }
    const BBox b = m_pRTree->toBox(min, max);
    return m_pRTree->remove(std::make_pair(b, payload)) > 0;
}

bool RTree::remove(const Area& area, uintptr_t payload) {
    if (m_pRTree == nullptr) {
        return false;
    }
    const BBox b = m_pRTree->toBox(area);
    return m_pRTree->remove(std::make_pair(b, payload)) > 0;
}

void RTree::query(const RS_Vector& min, const RS_Vector& max,
                  const std::function<bool(uintptr_t payload)>& callback) const {
    if (m_pRTree == nullptr || callback == nullptr) {
        return;
    }
    const BBox box = m_pRTree->toBox(min, max);
    for (auto it = m_pRTree->qbegin(bgi::intersects(box)); it != m_pRTree->qend(); ++it) {
        if (!callback(it->second)) {
            break;
        }
    }
}

std::vector<uintptr_t> RTree::query(const RS_Vector& min, const RS_Vector& max) const {
    std::vector<uintptr_t> result;
    query(min, max, [&result](uintptr_t payload) -> bool {
        result.push_back(payload);
        return true;
    });
    return result;
}

void RTree::clear() {
    if (m_pRTree != nullptr) {
        m_pRTree->clear();
    }
}

} // namespace geo
} // namespace lc
//EOF
