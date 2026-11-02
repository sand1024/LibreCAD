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

#ifndef LC_RTree_H
#define LC_RTree_H

#include <functional>
#include <memory>
#include <vector>

class RS_Vector;
class RS_VectorSolutions;

namespace lc {

namespace geo {
class Area;
using Box = Area;

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
class RTree {
public:
    /**
     * @brief RTree Constructor of a RTree for efficient point look up
     * @param toleranceSize - the default box size to use to find nearest points,
     *                        i.e. the node box centers
     */
    RTree(double toleranceSize=1e-10);

    // Bulk-loading constructor for packed spatial index construction
    RTree(const std::vector<std::tuple<RS_Vector, RS_Vector, uintptr_t>>& items, double toleranceSize = 1e-10);

    ~RTree();

    // Direct payload insertion
    bool insert(const RS_Vector& min, const RS_Vector& max, uintptr_t payload);
    bool insert(const Area& area, uintptr_t payload);

    // Bounding-box based removal
    bool remove(const RS_Vector& min, const RS_Vector& max, uintptr_t payload);
    bool remove(const Area& area, uintptr_t payload);

    // Spatial queries
    void query(const RS_Vector& min, const RS_Vector& max,
               const std::function<bool(uintptr_t payload)>& callback) const;
    std::vector<uintptr_t> query(const RS_Vector& min, const RS_Vector& max) const;
    void clear();

private:
    struct RTreeImpl;
    std::unique_ptr<RTreeImpl> m_pRTree;
};
} // geo
} // lc

using LC_RTree = lc::geo::RTree;

#endif
