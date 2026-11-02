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

#include "lc_intersection.h"

#include "rs_circle.h"
#include "rs_line.h"
#include "rs_vector.h"
#include "rs_circle.h"
#include "rs_arc.h"
#include "rs_ellipse.h"
#include "rs_information.h"

RS_VectorSolutions LC_Intersection::getIntersectionSegmentSegment(const RS_Line* seg1, const RS_Line* seg2) {
    if (seg1 == nullptr || seg2 == nullptr) {
        return {};
    }

    const RS_Vector p1 = seg1->getStartpoint();
    const RS_Vector p2 = seg1->getEndpoint();
    const RS_Vector p3 = seg2->getStartpoint();
    const RS_Vector p4 = seg2->getEndpoint();

    const RS_Vector d1 = p2 - p1;
    const RS_Vector d2 = p4 - p3;

    const double div = d1.x * d2.y - d1.y * d2.x;

    // Parallel or collinear condition
    if (std::abs(div) <= RS_TOLERANCE) {
        const double sqLen1 = d1.squared();
        if (sqLen1 <= RS_TOLERANCE2) {
            if (seg2->isPointOnEntity(p1, RS_TOLERANCE)) {
                return {p1};
            }
            return {};
        }

        // Check collinearity
        const double cross = (p3.x - p1.x) * d1.y - (p3.y - p1.y) * d1.x;
        if (std::abs(cross) > RS_TOLERANCE * std::sqrt(sqLen1)) {
            return {};
        }

        // Collinear: test endpoint projections
        const double t3 = (p3 - p1).dotP(d1) / sqLen1;
        const double t4 = (p4 - p1).dotP(d1) / sqLen1;
        const double minT = std::min(t3, t4);
        const double maxT = std::max(t3, t4);

        if (maxT >= 0.0 && minT <= 1.0) {
            const double overlapT = std::clamp((std::max(0.0, minT) + std::min(1.0, maxT)) * 0.5, 0.0, 1.0);
            return {p1 + d1 * overlapT};
        }
        return {};
    }

    const RS_Vector delta = p3 - p1;
    const double t1 = (delta.x * d2.y - delta.y * d2.x) / div;
    const double t2 = (delta.x * d1.y - delta.y * d1.x) / div;

    constexpr double paramTol = 1e-6;
    if (t1 >= -paramTol && t1 <= 1.0 + paramTol && t2 >= -paramTol && t2 <= 1.0 + paramTol) {
        const double clampedT = std::clamp(t1, 0.0, 1.0);
        return {p1 + d1 * clampedT};
    }

    return {};
}

RS_VectorSolutions LC_Intersection::getIntersectionSegmentCircle(const RS_Line* segment, const RS_Circle* circle) {
    if (segment == nullptr || circle == nullptr) {
        return {};
    }

    const RS_Vector p1 = segment->getStartpoint();
    const RS_Vector p2 = segment->getEndpoint();
    const RS_Vector d = p2 - p1;
    const double sqLen = d.squared();
    if (sqLen <= RS_TOLERANCE2) {
        return {};
    }

    const RS_Vector center = circle->getCenter();
    const double radius = circle->getRadius();
    const RS_Vector delta = p1 - center;

    // Solve quadratic equation: |p1 + t*d - center|^2 = radius^2
    const double a = sqLen;
    const double b = 2.0 * delta.dotP(d);
    const double c = delta.squared() - radius * radius;
    const double disc = b * b - 4.0 * a * c;

    if (disc < -RS_TOLERANCE) {
        return {};
    }

    RS_VectorSolutions solutions;
    constexpr double paramTol = 1e-6;

    if (disc <= RS_TOLERANCE) {
        const double t = -b / (2.0 * a);
        if (t >= -paramTol && t <= 1.0 + paramTol) {
            solutions.push_back(p1 + d * std::clamp(t, 0.0, 1.0));
            solutions.setTangent(true);
        }
        return solutions;
    }

    const double sqrtDisc = std::sqrt(disc);
    const double t1 = (-b - sqrtDisc) / (2.0 * a);
    const double t2 = (-b + sqrtDisc) / (2.0 * a);

    if (t1 >= -paramTol && t1 <= 1.0 + paramTol) {
        solutions.push_back(p1 + d * std::clamp(t1, 0.0, 1.0));
    }
    if (t2 >= -paramTol && t2 <= 1.0 + paramTol) {
        solutions.push_back(p1 + d * std::clamp(t2, 0.0, 1.0));
    }

    return solutions;
}

RS_VectorSolutions LC_Intersection::getIntersectionSegmentArc(const RS_Line* segment, const RS_Arc* arc) {
    if (segment == nullptr || arc == nullptr) {
        return {};
    }

    const RS_Circle circle(nullptr, {arc->getCenter(), arc->getRadius()});
    const RS_VectorSolutions circleHits = getIntersectionSegmentCircle(segment, &circle);
    if (circleHits.empty()) {
        return {};
    }

    const RS_Vector center = arc->getCenter();
    const double startAngle = arc->getAngle1();
    const double direction = arc->isReversed() ? -1.0 : 1.0;
    const double span = arc->getAngleLength();

    RS_VectorSolutions solutions;
    for (const RS_Vector& hit : circleHits) {
        const double angle = center.angleTo(hit);
        const double param = RS_Math::correctAngle(direction * (angle - startAngle));
        if (param <= span + RS_TOLERANCE_ANGLE) {
            solutions.push_back(hit);
        }
    }

    return solutions;
}

RS_VectorSolutions LC_Intersection::getIntersectionSegmentEllipse(const RS_Line* segment, const RS_Ellipse* ellipse) {
    if (segment == nullptr || ellipse == nullptr) {
        return {};
    }

    const RS_VectorSolutions rawHits = RS_Information::getIntersectionEllipseLine(segment, ellipse);
    if (rawHits.empty()) {
        return {};
    }

    const RS_Vector p1 = segment->getStartpoint();
    const RS_Vector p2 = segment->getEndpoint();
    const RS_Vector d = p2 - p1;
    const double sqLen = d.squared();
    if (sqLen <= RS_TOLERANCE2) {
        return {};
    }

    constexpr double paramTol = 1e-5;
    RS_VectorSolutions solutions;

    const bool isArc = ellipse->isEllipticArc();
    const double startAngle = isArc ? ellipse->getAngle1() : 0.0;
    const double direction = ellipse->isReversed() ? -1.0 : 1.0;
    const double span = ellipse->getAngleLength();

    for (const RS_Vector& hit : rawHits) {
        if (!hit.valid) {
            continue;
        }

        const double t = (hit - p1).dotP(d) / sqLen;
        if (t < -paramTol || t > 1.0 + paramTol) {
            continue;
        }

        if (isArc) {
            const double angle = ellipse->getEllipseAngle(hit);
            const double param = RS_Math::correctAngle(direction * (angle - startAngle));
            if (param > span + RS_TOLERANCE_ANGLE) {
                continue;
            }
        }
        solutions.push_back(hit);
    }

    return solutions;
}
