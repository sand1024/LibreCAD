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

#include "lc_lasso_region.h"

#include <algorithm>
#include "rs_math.h"

LC_LassoRegion::LC_LassoRegion()
    : m_min{false}
    , m_max{false} {
}

LC_LassoRegion::LC_LassoRegion(const QPolygonF& boundary, double minDistance)
    : m_minDistance{minDistance}
    , m_min{false}
    , m_max{false} {
    for (const QPointF& pt : boundary) {
        addPoint(RS_Vector{pt.x(), pt.y()});
    }
    finalize();
}

LC_LassoRegion::~LC_LassoRegion() = default;

void LC_LassoRegion::setMinDistance(double minDistance) {
    m_minDistance = std::max(0.0, minDistance);
}

bool LC_LassoRegion::addPoint(const RS_Vector& point) {
    if (!point.valid) {
        return false;
    }
    if (!m_points.empty()) {
        const double thresholdSq = LC_LineMath::isMeaningful(m_minDistance)  ? (m_minDistance * m_minDistance) : RS_TOLERANCE2;
        if (LC_LineMath::isNotMeaningfulDistanceSquared(point, m_points.back(), thresholdSq)) {
            return false;
        }
    }
    m_points.push_back(point);
    m_finalized = false;
    return true;
}

void LC_LassoRegion::clearEdges() {
    m_edges.clear();
    m_rtree.reset();
}

bool LC_LassoRegion::finalize(const bool closed) {
    clearEdges();
    m_closed = closed;

    const size_t minRequired = m_closed ? 3 : 2;
    if (m_points.size() < minRequired) {
        m_finalized = false;
        return false;
    }

    if (m_closed) {
        // Strip duplicate closure endpoint if it already exists
        while (m_points.size() >= 3 && LC_LineMath::isNotMeaningfulDistanceSquared(m_points.back(), m_points.front())) {
            m_points.pop_back();
        }
    }

    const size_t vertexCount = m_points.size();
    if (vertexCount < minRequired) {
        m_finalized = false;
        return false;
    }

    const size_t edgeCount = m_closed ? vertexCount : (vertexCount - 1);
    m_edges.reserve(edgeCount);
    m_polygon.clear();
    m_polygon.reserve(static_cast<qsizetype>(vertexCount));

    m_min = m_points.front();
    m_max = m_points.front();

    for (size_t i = 0; i < vertexCount; ++i) {
        const RS_Vector& p1 = m_points[i];
        m_polygon.append(QPointF(p1.x, p1.y));
        m_min = RS_Vector::minimum(m_min, p1);
        m_max = RS_Vector::maximum(m_max, p1);

        if (i < edgeCount) {
            const RS_Vector& p2 = m_points[(i + 1) % vertexCount];
            if (LC_LineMath::isNotMeaningfulDistanceSquared(p1, p2)) {
                continue;
            }

            LC_LassoSegment seg;
            seg.m_p1 = p1;
            seg.m_p2 = p2;
            seg.m_min = RS_Vector::minimum(p1, p2);
            seg.m_max = RS_Vector::maximum(p1, p2);
            seg.m_line = std::make_unique<RS_Line>(nullptr, p1, p2);
            m_edges.push_back(std::move(seg));
        }
    }

    if (m_edges.empty()) {
        clearEdges();
        m_finalized = false;
        return false;
    }

    buildSpatialIndex();
    m_finalized = true;
    return true;
}



void LC_LassoRegion::buildSpatialIndex() {
    if (m_edges.size() > HYBRID_INDEX_THRESHOLD) {
        std::vector<std::tuple<RS_Vector, RS_Vector, uintptr_t>> items;
        items.reserve(m_edges.size());
        for (size_t i = 0; i < m_edges.size(); ++i) {
            items.emplace_back(m_edges[i].m_min, m_edges[i].m_max, static_cast<uintptr_t>(i));
        }
        m_rtree = std::make_unique<lc::geo::RTree>(items);
    }
}

bool LC_LassoRegion::isValid() const {
    return m_finalized && m_edges.size() >= 3;
}

const RS_Vector& LC_LassoRegion::getMin() const {
    return m_min;
}

const RS_Vector& LC_LassoRegion::getMax() const {
    return m_max;
}

const QPolygonF& LC_LassoRegion::getPolygon() const {
    return m_polygon;
}

size_t LC_LassoRegion::edgeCount() const {
    return m_edges.size();
}

void LC_LassoRegion::forEachCandidateEdge(const RS_Vector& min, const RS_Vector& max,
                                          const std::function<bool(const RS_Line* edge)>& callback) const {
    if (callback == nullptr || !isValid()) {
        return;
    }

    if (m_rtree != nullptr) {
        m_rtree->query(min, max, [this, &callback](uintptr_t index) -> bool {
            if (index < m_edges.size()) {
                return callback(m_edges[index].m_line.get());
            }
            return true;
        });
    } else {
        for (const auto& edge : m_edges) {
            if (max.x < edge.m_min.x || min.x > edge.m_max.x ||
                max.y < edge.m_min.y || min.y > edge.m_max.y) {
                continue;
            }
            if (!callback(edge.m_line.get())) {
                break;
            }
        }
    }
}

bool LC_LassoRegion::inside(const RS_Vector& point) const {
    if (!m_closed || !point.valid || !isValid()) {
        return false;
    }

    // Cartesian bounding box rejection
    if (point.x < m_min.x - RS_TOLERANCE || point.x > m_max.x + RS_TOLERANCE ||
        point.y < m_min.y - RS_TOLERANCE || point.y > m_max.y + RS_TOLERANCE) {
        return false;
    }

    if (m_polygon.containsPoint(QPointF(point.x, point.y), Qt::OddEvenFill)) {
        return true;
    }

    // Spatial lookup checking only candidate edges within tolerance
    const RS_Vector tolMin(point.x - RS_TOLERANCE, point.y - RS_TOLERANCE);
    const RS_Vector tolMax(point.x + RS_TOLERANCE, point.y + RS_TOLERANCE);
    bool onBoundary = false;

    forEachCandidateEdge(tolMin, tolMax, [&point, &onBoundary](const RS_Line* edge) -> bool {
        if (edge != nullptr) {
            const RS_Vector nearest = LC_LineMath::getNearestPointOnLine(edge, point, false);
            if (nearest.valid && LC_LineMath::isNotMeaningfulDistanceSquared(point, nearest)) {
                onBoundary = true;
                return false;
            }
        }
        return true;
    });

    return onBoundary;
}


void LC_LassoRegion::clear() {
    m_points.clear();
    m_polygon.clear();
    clearEdges();
    m_min = RS_Vector{false};
    m_max = RS_Vector{false};
    m_finalized = false;
}

bool LC_LassoRegion::isEmpty() const {
    return m_points.empty();
}

size_t LC_LassoRegion::pointCount() const {
    return m_points.size();
}

const std::vector<RS_Vector>& LC_LassoRegion::getPoints() const {
    return m_points;
}

/** calculates area based on Shoelace formula
 * (signed area / Green's theorem):
 */
double LC_LassoRegion::signedArea() const {
    const size_t n = m_points.size();
    if (n < 3) {
        return 0.0;
    }

    double doubleArea = 0.0;
    for (size_t i = 0; i < n; ++i) {
        const RS_Vector& p1 = m_points[i];
        const RS_Vector& p2 = m_points[(i + 1) % n];
        doubleArea += (p1.x * p2.y - p2.x * p1.y);
    }

    return 0.5 * doubleArea;
}

bool LC_LassoRegion::isClockwise() const {
    // In Cartesian CAD world coordinates (Y points UP):
    // Positive signed area = Counter-Clockwise (CCW -> Window Selection)
    // Negative signed area = Clockwise (CW -> Crossing Selection)
    return signedArea() < 0.0;
}
