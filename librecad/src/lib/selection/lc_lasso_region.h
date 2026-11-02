
/*******************************************************************************
 *
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2026 LibreCAD.org
 * Copyright (C) 2026 sand1024
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#ifndef LC_LASSOREGION_H
#define LC_LASSOREGION_H

#include <functional>
#include <memory>
#include <QPolygonF>
#include <vector>

#include "lc_rtree.h"
#include "lc_rtree.h"
#include "rs_line.h"
#include "rs_vector.h"

struct LC_LassoSegment {
    RS_Vector m_p1;
    RS_Vector m_p2;
    RS_Vector m_min;
    RS_Vector m_max;
    std::unique_ptr<RS_Line> m_line;
};

class LC_LassoRegion {
public:
    LC_LassoRegion();
    explicit LC_LassoRegion(const QPolygonF& boundary, double minDistance = 0.0);
    ~LC_LassoRegion();

    void setMinDistance(double minDistance);
    bool addPoint(const RS_Vector& point);
    bool finalize(bool closed = true);
    void clear();
    bool isValid() const;
    bool isEmpty() const;
    size_t pointCount() const;
    const std::vector<RS_Vector>& getPoints() const;
    double signedArea() const;
    bool isClockwise() const;
    bool inside(const RS_Vector& point) const;
    const RS_Vector& getMin() const;
    const RS_Vector& getMax() const;
    const QPolygonF& getPolygon() const;
    size_t edgeCount() const;

    bool isClosed() const {
        return m_closed;
    }

    void forEachCandidateEdge(const RS_Vector& min, const RS_Vector& max,
                              const std::function<bool(const RS_Line* edge)>& callback) const;

private:
    static constexpr size_t HYBRID_INDEX_THRESHOLD = 64;

    std::vector<RS_Vector> m_points;
    std::vector<LC_LassoSegment> m_edges;
    RS_Vector m_min;
    RS_Vector m_max;
    QPolygonF m_polygon;
    std::unique_ptr<lc::geo::RTree> m_rtree;
    double m_minDistance = 0.0;
    bool m_finalized = false;
    bool m_closed = false;

    void clearEdges();
    void buildSpatialIndex();
};

#endif
