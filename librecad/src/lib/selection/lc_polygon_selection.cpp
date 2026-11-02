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


#include "lc_polygon_selection.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "lc_containertraverser.h"
#include "lc_dimarrowblock.h"
#include "lc_graphicviewport.h"
#include "lc_hyperbola.h"
#include "lc_intersection.h"
#include "lc_lasso_region.h"
#include "lc_linemath.h"
#include "lc_mleader.h"
#include "lc_parabola.h"
#include "lc_quadratic.h"
#include "lc_wipeout.h"
#include "rs_arc.h"
#include "rs_circle.h"
#include "rs_ellipse.h"
#include "rs_entitycontainer.h"
#include "rs_hatch.h"
#include "rs_image.h"
#include "rs_information.h"
#include "rs_insert.h"
#include "rs_line.h"
#include "rs_math.h"
#include "rs_mtext.h"
#include "rs_point.h"
#include "rs_selection.h"
#include "rs_solid.h"
#include "rs_spline.h"

class RS_Solid;

 // namespace
namespace {

bool matches(const RS_Entity* entity, const LC_LassoRegion& region, const bool crossing);

bool matchPoint(const RS_Point* point, const LC_LassoRegion& region) {
    if (point == nullptr) {
        return false;
    }
    return region.inside(point->getPos());
}

bool matchLine(const RS_Line* line, const LC_LassoRegion& region, const bool crossing) {
    if (line == nullptr) {
        return false;
    }

    const RS_Vector start = line->getStartpoint();
    const RS_Vector end = line->getEndpoint();
    const bool startInside = region.inside(start);
    const bool endInside = region.inside(end);

    if (crossing) {
        if (startInside || endInside) {
            return true;
        }
    }
    else {
        if (!startInside || !endInside) {
            return false;
        }
    }

    if (LC_LineMath::isNotMeaningfulDistanceSquared(start, end)) {
        return startInside;
    }

    const RS_Vector lineMin = RS_Vector::minimum(start, end);
    const RS_Vector lineMax = RS_Vector::maximum(start, end);

    bool hasIntersection = false;
    std::vector<double> cuts{0.0, 1.0};
    const RS_Vector delta = end - start;
    const double sqLength = delta.squared();

    region.forEachCandidateEdge(lineMin, lineMax, [&](const RS_Line* edge) -> bool {
        const RS_VectorSolutions hits = LC_Intersection::getIntersectionSegmentSegment(line, edge);
        for (size_t i = 0; i < hits.getNumber(); ++i) {
            const RS_Vector hit = hits.get(i);
            if (hit.valid) {
                hasIntersection = true;
                if (crossing) {
                    return false;
                }
                if (sqLength > RS_TOLERANCE2) {
                    const double t = ((hit - start).x * delta.x + (hit - start).y * delta.y) / sqLength;
                    cuts.push_back(std::clamp(t, 0.0, 1.0));
                }
            }
        }
        return true;
    });

    if (crossing) {
        return hasIntersection;
    }

    if (!hasIntersection) {
        return true;
    }

    std::sort(cuts.begin(), cuts.end());
    cuts.erase(std::unique(cuts.begin(), cuts.end(), [](const double a, const double b) -> bool {
        return std::abs(a - b) <= 1e-6;
    }), cuts.end());

    for (size_t i = 1; i < cuts.size(); ++i) {
        const double midT = 0.5 * (cuts[i - 1] + cuts[i]);
        if (!region.inside(start + delta * midT)) {
            return false;
        }
    }

    return true;
}

bool matchCircle(const RS_Circle* circle, const LC_LassoRegion& region, const bool crossing) {
    if (circle == nullptr) {
        return false;
    }

    const RS_Vector center = circle->getCenter();
    const double radius = circle->getRadius();
    const RS_Vector circleMin(center.x - radius, center.y - radius);
    const RS_Vector circleMax(center.x + radius, center.y + radius);

    bool hasIntersection = false;
    region.forEachCandidateEdge(circleMin, circleMax, [&](const RS_Line* edge) -> bool {
        const RS_VectorSolutions hits = LC_Intersection::getIntersectionSegmentCircle(edge, circle);
        if (hits.hasValid()) {
            hasIntersection = true;
            return false;
        }
        return true;
    });

    const RS_Vector samplePoint = center + RS_Vector(radius, 0.0);

    if (crossing) {
        if (hasIntersection) {
            return true;
        }
        return region.inside(samplePoint);
    }

    if (hasIntersection) {
        return false;
    }

    return region.inside(samplePoint);
}

bool matchArc(const RS_Arc* arc, const LC_LassoRegion& region, const bool crossing) {
    if (arc == nullptr) {
        return false;
    }

    const RS_Vector start = arc->getStartpoint();
    const RS_Vector end = arc->getEndpoint();
    const bool startInside = region.inside(start);
    const bool endInside = region.inside(end);

    if (crossing) {
        if (startInside || endInside) {
            return true;
        }
    }
    else {
        if (!startInside || !endInside) {
            return false;
        }
    }

    const RS_Vector arcMin = arc->getMin();
    const RS_Vector arcMax = arc->getMax();
    const RS_Vector center = arc->getCenter();
    const double radius = arc->getRadius();
    const double startAngle = arc->getAngle1();
    const double direction = arc->isReversed() ? -1.0 : 1.0;
    const double span = arc->getAngleLength();

    bool hasIntersection = false;
    std::vector<double> cuts{0.0, span};

    region.forEachCandidateEdge(arcMin, arcMax, [&](const RS_Line* edge) -> bool {
        const RS_VectorSolutions hits = LC_Intersection::getIntersectionSegmentArc(edge, arc);
        for (size_t i = 0; i < hits.getNumber(); ++i) {
            const RS_Vector hit = hits.get(i);
            if (hit.valid) {
                hasIntersection = true;
                if (crossing) {
                    return false;
                }
                const double angle = center.angleTo(hit);
                const double param = RS_Math::correctAngle(direction * (angle - startAngle));
                if (param <= span + RS_TOLERANCE_ANGLE) {
                    cuts.push_back(std::clamp(param, 0.0, span));
                }
            }
        }
        return true;
    });

    if (crossing) {
        return hasIntersection;
    }

    if (!hasIntersection) {
        return true;
    }

    std::sort(cuts.begin(), cuts.end());
    cuts.erase(std::unique(cuts.begin(), cuts.end(), [](const double a, const double b) -> bool {
        return std::abs(a - b) <= RS_TOLERANCE_ANGLE;
    }), cuts.end());

    for (size_t i = 1; i < cuts.size(); ++i) {
        const double midParam = 0.5 * (cuts[i - 1] + cuts[i]);
        const RS_Vector midPt = center + RS_Vector::polar(radius, startAngle + direction * midParam);
        if (!region.inside(midPt)) {
            return false;
        }
    }

    return true;
}

bool matchEllipse(const RS_Ellipse* ellipse, const LC_LassoRegion& region, const bool crossing) {
    if (ellipse == nullptr) {
        return false;
    }

    const RS_Vector ellMin = ellipse->getMin();
    const RS_Vector ellMax = ellipse->getMax();

    if (!ellipse->isEllipticArc()) {
        bool hasIntersection = false;
        region.forEachCandidateEdge(ellMin, ellMax, [&](const RS_Line* edge) -> bool {
            const RS_VectorSolutions hits = LC_Intersection::getIntersectionSegmentEllipse(edge, ellipse);
            if (hits.hasValid()) {
                hasIntersection = true;
                return false;
            }
            return true;
        });

        const RS_Vector samplePoint = ellipse->getEllipsePoint(0.0);

        if (crossing) {
            if (hasIntersection) {
                return true;
            }
            return region.inside(samplePoint);
        }

        if (hasIntersection) {
            return false;
        }

        return region.inside(samplePoint);
    }

    const RS_Vector start = ellipse->getStartpoint();
    const RS_Vector end = ellipse->getEndpoint();
    const bool startInside = region.inside(start);
    const bool endInside = region.inside(end);

    if (crossing) {
        if (startInside || endInside) {
            return true;
        }
    }
    else {
        if (!startInside || !endInside) {
            return false;
        }
    }

    const double startAngle = ellipse->getAngle1();
    const double direction = ellipse->isReversed() ? -1.0 : 1.0;
    const double span = ellipse->getAngleLength();

    bool hasIntersection = false;
    std::vector<double> cuts{0.0, span};

    region.forEachCandidateEdge(ellMin, ellMax, [&](const RS_Line* edge) -> bool {
        const RS_VectorSolutions hits = LC_Intersection::getIntersectionSegmentEllipse(edge, ellipse);
        if (hits.hasValid()) {
            hasIntersection = true;
            if (crossing) {
                return false;
            }
            for (size_t i = 0; i < hits.getNumber(); ++i) {
                const RS_Vector hit = hits.get(i);
                if (hit.valid) {
                    const double angle = ellipse->getEllipseAngle(hit);
                    const double param = RS_Math::correctAngle(direction * (angle - startAngle));
                    if (param <= span + RS_TOLERANCE_ANGLE) {
                        cuts.push_back(std::clamp(param, 0.0, span));
                    }
                }
            }
        }
        return true;
    });

    if (crossing) {
        return hasIntersection;
    }

    if (!hasIntersection) {
        return true;
    }

    std::sort(cuts.begin(), cuts.end());
    cuts.erase(std::unique(cuts.begin(), cuts.end(), [](const double a, const double b) -> bool {
        return std::abs(a - b) <= RS_TOLERANCE_ANGLE;
    }), cuts.end());

    for (size_t i = 1; i < cuts.size(); ++i) {
        const double midParam = 0.5 * (cuts[i - 1] + cuts[i]);
        const RS_Vector midPt = ellipse->getEllipsePoint(startAngle + direction * midParam);
        if (!region.inside(midPt)) {
            return false;
        }
    }

    return true;
}

bool matchBox(const RS_Vector& min, const RS_Vector& max, const LC_LassoRegion& region, const bool crossing) {
    if (!min.valid || !max.valid) {
        return false;
    }

    const RS_Vector corners[4] = {
        min,
        RS_Vector{max.x, min.y},
        max,
        RS_Vector{min.x, max.y}
    };

    bool allInside = true;
    bool anyInside = false;
    for (int i = 0; i < 4; ++i) {
        const bool in = region.inside(corners[i]);
        allInside = allInside && in;
        anyInside = anyInside || in;
    }

    if (crossing && anyInside) {
        return true;
    }
    if (!crossing && !allInside) {
        return false;
    }

    bool edgesIntersect = false;
    for (int i = 0; i < 4; ++i) {
        const RS_Line boxEdge{nullptr, corners[i], corners[(i + 1) % 4]};
        region.forEachCandidateEdge(min, max, [&](const RS_Line* lassoEdge) -> bool {
            if (LC_Intersection::getIntersectionSegmentSegment(&boxEdge, lassoEdge).hasValid()) {
                edgesIntersect = true;
                return false;
            }
            return true;
        });
        if (edgesIntersect) {
            break;
        }
    }

    if (crossing) {
        if (edgesIntersect) {
            return true;
        }
        const RS_Vector lassoCenter = (region.getMin() + region.getMax()) * 0.5;
        return lassoCenter.x >= min.x && lassoCenter.x <= max.x &&
               lassoCenter.y >= min.y && lassoCenter.y <= max.y;
    }

    return !edgesIntersect;
}

bool matchDimArrow(const LC_DimArrow* arrow, const LC_LassoRegion& region, const bool crossing) {
    if (arrow == nullptr) {
        return false;
    }
    return matchBox(arrow->getMin(), arrow->getMax(), region, crossing);
}

bool matchSolid(const RS_Solid* solid, const LC_LassoRegion& region, const bool crossing) {
    if (solid == nullptr) {
        return false;
    }

    const int cornerCount = solid->isTriangle() ? 3 : 4;
    const RS_SolidData& data = solid->getData();

    bool allInside = true;
    bool anyInside = false;

    for (int i = 0; i < cornerCount; ++i) {
        const RS_Vector corner = data.corners[i];
        if (corner.valid) {
            const bool in = region.inside(corner);
            allInside = allInside && in;
            anyInside = anyInside || in;
        }
    }

    if (crossing && anyInside) {
        return true;
    }
    if (!crossing && !allInside) {
        return false;
    }

    std::vector<RS_Line> edges;
    edges.reserve(cornerCount);
    for (int i = 0; i < cornerCount; ++i) {
        const int next = (i + 1) % cornerCount;
        edges.emplace_back(nullptr, data.corners[i], data.corners[next]);
    }

    const RS_Vector solidMin = solid->getMin();
    const RS_Vector solidMax = solid->getMax();
    bool edgeIntersects = false;

    region.forEachCandidateEdge(solidMin, solidMax, [&](const RS_Line* lassoEdge) -> bool {
        for (const RS_Line& solidEdge : edges) {
            if (LC_Intersection::getIntersectionSegmentSegment(&solidEdge, lassoEdge).hasValid()) {
                edgeIntersects = true;
                return false;
            }
        }
        return true;
    });

    if (crossing) {
        if (edgeIntersects) {
            return true;
        }
        const RS_Vector lassoCenter = (region.getMin() + region.getMax()) * 0.5;
        double dist = RS_MAXDOUBLE;
        solid->getNearestPointOnEntity(lassoCenter, true, &dist);
        return LC_LineMath::isNotMeaningful(dist);
    }

    return !edgeIntersects;
}

bool matchSpline(const RS_Spline* spline, const LC_LassoRegion& region, const bool crossing) {
    if (spline == nullptr) {
        return false;
    }

    bool anyLineTested = false;
    for (const RS_Entity* entity : *spline) {
        if (entity == nullptr || entity->isDeleted() || entity->rtti() != RS2::EntityLine) {
            continue;
        }
        anyLineTested = true;
        const bool matched = matchLine(static_cast<const RS_Line*>(entity), region, crossing);
        if (crossing && matched) {
            return true;
        }
        if (!crossing && !matched) {
            return false;
        }
    }

    return !crossing && anyLineTested;
}

bool matchSplinePoints(const LC_SplinePoints* spline, const LC_LassoRegion& region, const bool crossing) {
    if (spline == nullptr) {
        return false;
    }

    const RS_Vector start = spline->getStartpoint();
    const RS_Vector end = spline->getEndpoint();
    const bool isClosed = spline->isClosed();

    if (!isClosed && start.valid && end.valid) {
        const bool startIn = region.inside(start);
        const bool endIn = region.inside(end);
        if (crossing && (startIn || endIn)) {
            return true;
        }
        if (!crossing && (!startIn || !endIn)) {
            return false;
        }
    }
    else if (isClosed && !spline->getPoints().empty()) {
        const bool ptIn = region.inside(spline->getPoints().front());
        if (crossing && ptIn) {
            return true;
        }
        if (!crossing && !ptIn) {
            return false;
        }
    }

    const RS_Vector spMin = spline->getMin();
    const RS_Vector spMax = spline->getMax();
    bool hasIntersection = false;

    region.forEachCandidateEdge(spMin, spMax, [&](const RS_Line* edge) -> bool {
        const RS_VectorSolutions hits = spline->getLineIntersect(edge->getStartpoint(), edge->getEndpoint());
        if (hits.hasValid()) {
            hasIntersection = true;
            return false;
        }
        return true;
    });

    if (crossing) {
        return hasIntersection;
    }
    if (hasIntersection) {
        return false;
    }

    for (const RS_Vector& pt : spline->getPoints()) {
        if (!region.inside(pt)) {
            return false;
        }
    }

    return true;
}

bool matchParabola(const LC_Parabola* parabola, const LC_LassoRegion& region, const bool crossing) {
    if (parabola == nullptr) {
        return false;
    }
    if (!matchSplinePoints(parabola, region, crossing)) {
        return false;
    }
    if (!crossing) {
        return region.inside(parabola->getVertex());
    }
    return true;
}

bool matchHyperbola(const LC_Hyperbola* hyperbola, const LC_LassoRegion& region, const bool crossing) {
    if (hyperbola == nullptr || !hyperbola->isValid()) {
        return false;
    }

    const bool infinite = hyperbola->isInfinite();
    if (infinite && !crossing) {
        return false;
    }

    const RS_Vector start = hyperbola->getStartpoint();
    const RS_Vector end = hyperbola->getEndpoint();

    if (!infinite && start.valid && end.valid) {
        const bool startIn = region.inside(start);
        const bool endIn = region.inside(end);
        if (crossing && (startIn || endIn)) {
            return true;
        }
        if (!crossing && (!startIn || !endIn)) {
            return false;
        }
    }

    const RS_Vector hypMin = hyperbola->getMin();
    const RS_Vector hypMax = hyperbola->getMax();
    const LC_Quadratic hypQuad = hyperbola->getQuadratic();
    const double phiMin = std::min(hyperbola->getAngle1(), hyperbola->getAngle2());
    const double phiMax = std::max(hyperbola->getAngle1(), hyperbola->getAngle2());
    bool hasIntersection = false;

    region.forEachCandidateEdge(hypMin, hypMax, [&](const RS_Line* edge) -> bool {
        const RS_VectorSolutions rawHits = LC_Quadratic::getIntersection(hypQuad, edge->getQuadratic());
        for (size_t i = 0; i < rawHits.getNumber(); ++i) {
            const RS_Vector hit = rawHits.get(i);
            if (!hit.valid) {
                continue;
            }
            if (!edge->isPointOnEntity(hit, RS_TOLERANCE)) {
                continue;
            }
            if (!infinite) {
                const double phi = hyperbola->getParamFromPoint(hit, hyperbola->isReversed());
                if (std::isnan(phi) || phi < phiMin - RS_TOLERANCE_ANGLE || phi > phiMax + RS_TOLERANCE_ANGLE) {
                    continue;
                }
            }
            hasIntersection = true;
            return false;
        }
        return true;
    });

    if (crossing) {
        return hasIntersection;
    }
    if (hasIntersection) {
        return false;
    }

    const RS_Vector vertex = hyperbola->getPrimaryVertex();
    if (vertex.valid) {
        return region.inside(vertex);
    }
    return true;
}

bool matchImage(const RS_Image* image, const LC_LassoRegion& region, const bool crossing) {
    if (image == nullptr) {
        return false;
    }

    const RS_VectorSolutions corners = image->getCorners();
    if (corners.getNumber() != 4) {
        return false;
    }

    bool allCornersInside = true;
    bool anyCornerInside = false;
    for (size_t i = 0; i < 4; ++i) {
        const bool insideCorner = region.inside(corners.get(i));
        allCornersInside = allCornersInside && insideCorner;
        anyCornerInside = anyCornerInside || insideCorner;
    }

    if (crossing && anyCornerInside) {
        return true;
    }
    if (!crossing && !allCornersInside) {
        return false;
    }

    bool edgesIntersect = false;
    for (size_t i = 0; i < 4; ++i) {
        const RS_Line edge(nullptr, corners.get(i), corners.get((i + 1) % 4));
        if (matchLine(&edge, region, true)) {
            edgesIntersect = true;
            break;
        }
    }

    if (crossing) {
        if (edgesIntersect) {
            return true;
        }
        const RS_Vector lassoCenter = (region.getMin() + region.getMax()) * 0.5;
        return image->containsPoint(lassoCenter);
    }

    return !edgesIntersect;
}

bool matchWipeout(const LC_Wipeout* wipeout, const LC_LassoRegion& region, const bool crossing) {
    if (wipeout == nullptr) {
        return false;
    }

    const auto& verts = wipeout->getVertices();
    const size_t count = verts.size();
    if (count < 3) {
        return false;
    }

    bool allInside = true;
    bool anyInside = false;
    for (const RS_Vector& v : verts) {
        const bool in = region.inside(v);
        allInside = allInside && in;
        anyInside = anyInside || in;
    }

    if (crossing && anyInside) {
        return true;
    }
    if (!crossing && !allInside) {
        return false;
    }

    bool edgesIntersect = false;
    for (size_t i = 0; i < count; ++i) {
        const RS_Line seg{nullptr, verts[i], verts[(i + 1) % count]};
        region.forEachCandidateEdge(wipeout->getMin(), wipeout->getMax(), [&](const RS_Line* lassoEdge) -> bool {
            if (LC_Intersection::getIntersectionSegmentSegment(&seg, lassoEdge).hasValid()) {
                edgesIntersect = true;
                return false;
            }
            return true;
        });
        if (edgesIntersect) {
            break;
        }
    }

    if (crossing) {
        if (edgesIntersect) {
            return true;
        }
        const RS_Vector lassoCenter = (region.getMin() + region.getMax()) * 0.5;
        QPolygonF poly;
        poly.reserve(static_cast<qsizetype>(count));
        for (const RS_Vector& v : verts) {
            poly << QPointF{v.x, v.y};
        }
        return poly.containsPoint(QPointF{lassoCenter.x, lassoCenter.y}, Qt::OddEvenFill);
    }

    return !edgesIntersect;
}

bool matchMLeader(const LC_MLeader* mleader, const LC_LassoRegion& region, const bool crossing) {
    if (mleader == nullptr) {
        return false;
    }

    const auto& roots = mleader->getRoots();
    bool anyElementMatched = false;

    // 1. Evaluate leader polyline segments
    for (const auto& root : roots) {
        for (const auto& ll : root.leaderLines) {
            const size_t n = ll.points.size();
            if (n < 2) {
                continue;
            }
            for (size_t i = 1; i < n; ++i) {
                const RS_Line seg{nullptr, ll.points[i - 1], ll.points[i]};
                const bool matched = matchLine(&seg, region, crossing);
                if (crossing && matched) {
                    return true;
                }
                if (!crossing && !matched) {
                    return false;
                }
                anyElementMatched = true;
            }
        }
    }

    // 2. Evaluate annotation content
    RS_MTextData td;
    if (mleader->textContentData(td)) {
        const RS_Vector loc = td.insertionPoint;
        const double width = td.width > 0.0 ? td.width : td.height * 5.0;
        const RS_Vector min = loc;
        const RS_Vector max = loc + RS_Vector{width, td.height};
        const bool textMatched = matchBox(min, max, region, crossing);

        if (crossing && textMatched) {
            return true;
        }
        if (!crossing && !textMatched) {
            return false;
        }
        anyElementMatched = true;
    }

    RS_InsertData id;
    if (mleader->blockContentData(id)) {
        const RS_Vector min = id.insertionPoint - RS_Vector{1.0, 1.0};
        const RS_Vector max = id.insertionPoint + RS_Vector{1.0, 1.0};
        const bool blockMatched = matchBox(min, max, region, crossing);

        if (crossing && blockMatched) {
            return true;
        }
        if (!crossing && !blockMatched) {
            return false;
        }
        anyElementMatched = true;
    }

    return !crossing && anyElementMatched;
}

bool matchBoundaryEntity(const RS_Entity* edge, const LC_LassoRegion& region, const bool crossing) {
    if (edge == nullptr || edge->isDeleted()) {
        return false;
    }

    switch (edge->rtti()) {
    case RS2::EntityLine:
        return matchLine(static_cast<const RS_Line*>(edge), region, crossing);
    case RS2::EntityArc:
        return matchArc(static_cast<const RS_Arc*>(edge), region, crossing);
    case RS2::EntityCircle:
        return matchCircle(static_cast<const RS_Circle*>(edge), region, crossing);
    case RS2::EntityEllipse:
        return matchEllipse(static_cast<const RS_Ellipse*>(edge), region, crossing);
    case RS2::EntitySpline:
        return matchSpline(static_cast<const RS_Spline*>(edge), region, crossing);
    case RS2::EntitySplinePoints:
        return matchSplinePoints(static_cast<const LC_SplinePoints*>(edge), region, crossing);
    default:
        break;
    }

    return false;
}

bool matchHatch(const RS_Hatch* hatch, const LC_LassoRegion& region, const bool crossing) {
    if (hatch == nullptr) {
        return false;
    }

    std::vector<const RS_EntityContainer*> loopContainers;
    for (const RS_Entity* en : *hatch) {
        if (en != nullptr && en->isContainer()) {
            loopContainers.push_back(static_cast<const RS_EntityContainer*>(en));
        }
    }

    if (loopContainers.empty()) {
        const int allLoops = hatch->countAllLoops();
        for (int i = 0; i < allLoops; ++i) {
            const RS_EntityContainer* bc = hatch->getBoundaryContainer(i);
            if (bc != nullptr) {
                loopContainers.push_back(bc);
            }
        }
    }

    if (loopContainers.empty()) {
        return false;
    }

    bool anyEdgeTested = false;

    for (const RS_EntityContainer* loop : loopContainers) {
        lc::LC_ContainerTraverser traverser{*loop, RS2::ResolveAll};
        for (const RS_Entity* edge = traverser.first(); edge != nullptr; edge = traverser.next()) {
            if (edge == nullptr || edge->isDeleted() || !edge->isEdge()) {
                continue;
            }

            anyEdgeTested = true;
            const bool matched = matchBoundaryEntity(edge, region, crossing);

            if (crossing && matched) {
                return true;
            }
            if (!crossing && !matched) {
                return false;
            }
        }
    }

    return !crossing && anyEdgeTested;
}

bool matchContainer(const RS_EntityContainer* container, const LC_LassoRegion& region, const bool crossing) {
    if (container == nullptr) {
        return false;
    }

    lc::LC_ContainerTraverser traverser{*container, RS2::ResolveAll};
    bool anyChildVisible = false;

    for (const RS_Entity* child = traverser.first(); child != nullptr; child = traverser.next()) {
        if (child == nullptr || child->isDeleted() || !child->isVisible() || child->isLocked()) {
            continue;
        }
        anyChildVisible = true;
        const bool matched = matches(child, region, crossing);
        if (crossing && matched) {
            return true;
        }
        if (!crossing && !matched) {
            return false;
        }
    }

    return !crossing && anyChildVisible;
}

bool matches(const RS_Entity* entity, const LC_LassoRegion& region, const bool crossing) {
    if (entity == nullptr || entity->isDeleted() || !entity->isVisible() || entity->isLocked()) {
        return false;
    }

    const RS_Vector entMin = entity->getMin();
    const RS_Vector entMax = entity->getMax();
    const RS_Vector regMin = region.getMin();
    const RS_Vector regMax = region.getMax();

    if (entMax.x < regMin.x - RS_TOLERANCE || entMin.x > regMax.x + RS_TOLERANCE ||
        entMax.y < regMin.y - RS_TOLERANCE || entMin.y > regMax.y + RS_TOLERANCE) {
        return false;
    }

    // 1. Specialized container intercepts (must precede generic isContainer)
    if (entity->rtti() == RS2::EntityHatch) {
        return matchHatch(static_cast<const RS_Hatch*>(entity), region, crossing);
    }
    if (entity->rtti() == RS2::EntityText || entity->rtti() == RS2::EntityMText) {
        return matchBox(entity->getMin(), entity->getMax(), region, crossing);
    }
    if (entity->rtti() == RS2::EntitySpline) {
        return matchSpline(static_cast<const RS_Spline*>(entity), region, crossing);
    }

    // 2. Generic containers (RS_Polyline, RS_Insert, RS_Dimension)
    if (entity->isContainer()) {
        return matchContainer(static_cast<const RS_EntityContainer*>(entity), region, crossing);
    }

    // 3. Atomic entities
    switch (entity->rtti()) {
    case RS2::EntityPoint:
        return matchPoint(static_cast<const RS_Point*>(entity), region);
    case RS2::EntityLine:
        return matchLine(static_cast<const RS_Line*>(entity), region, crossing);
    case RS2::EntityCircle:
        return matchCircle(static_cast<const RS_Circle*>(entity), region, crossing);
    case RS2::EntityArc:
        return matchArc(static_cast<const RS_Arc*>(entity), region, crossing);
    case RS2::EntityEllipse:
        return matchEllipse(static_cast<const RS_Ellipse*>(entity), region, crossing);
    case RS2::EntitySplinePoints:
        return matchSplinePoints(static_cast<const LC_SplinePoints*>(entity), region, crossing);
    case RS2::EntityParabola:
        return matchParabola(static_cast<const LC_Parabola*>(entity), region, crossing);
    case RS2::EntityHyperbola:
        return matchHyperbola(static_cast<const LC_Hyperbola*>(entity), region, crossing);
    case RS2::EntityImage:
        return matchImage(static_cast<const RS_Image*>(entity), region, crossing);
    case RS2::EntityWipeout:
        return matchWipeout(static_cast<const LC_Wipeout*>(entity), region, crossing);
    case RS2::EntitySolid:
        return matchSolid(static_cast<const RS_Solid*>(entity), region, crossing);
    case RS2::EntityDimArrowBlock:
        return matchDimArrow(static_cast<const LC_DimArrow*>(entity), region, crossing);
    case RS2::EntityMLeader:
        return matchMLeader(static_cast<const LC_MLeader*>(entity), region, crossing);
    default:
        break;
    }

    return false;
}

} // namespace

void LC_PolygonSelection::select(RS_Document* document, LC_GraphicViewport* viewport,
                                 const LC_LassoRegion& region, const RS2::EntityType type,
                                 const bool crossing, const bool selecting) {
    if (document == nullptr || viewport == nullptr || !region.isValid()) {
        return;
    }

    QList<RS_Entity*> matchedEntities;
    for (RS_Entity* entity : *document) {
        if (entity == nullptr || entity->isDeleted() || !entity->isVisible() || entity->isLocked()) {
            continue;
        }
        if (type != RS2::EntityUnknown && entity->rtti() != type) {
            continue;
        }
        if (matches(entity, region, crossing)) {
            matchedEntities.append(entity);
        }
    }

    const RS_Selection selection(document, viewport);
    selection.selectEntitiesList(matchedEntities, selecting);
}


void LC_PolygonSelection::select(RS_Document* document, LC_GraphicViewport* viewport,
                                 const LC_LassoRegion& region, const QList<RS2::EntityType> &typesToSelect,
                                 const bool crossing, const bool selecting) {
    if (document == nullptr || viewport == nullptr || !region.isValid()) {
        return;
    }

    if (!region.isValid()) {
        return;
    }

    QList<RS_Entity*> matchedEntities;
    for (RS_Entity* entity : *document) {
        if (entity == nullptr || entity->isDeleted() || !entity->isVisible() || entity->isLocked()) {
            continue;
        }
        if (!typesToSelect.contains(entity->rtti())) {
            continue;
        }
        if (matches(entity, region, crossing)) {
            matchedEntities.append(entity);
        }
    }

    const RS_Selection selection(document, viewport);
    selection.selectEntitiesList(matchedEntities, selecting);
}
