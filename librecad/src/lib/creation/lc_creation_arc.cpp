/*
 * ********************************************************************************
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
 * ********************************************************************************
 */

#include "lc_creation_arc.h"

#include "rs_debug.h"
#include "rs_information.h"
#include "rs_line.h"
#include "rs_math.h"

/**
 * Creates this arc from 3 given points which define the arc line.
 *
 * @param p1 1st point.
 * @param p2 2nd point.
 * @param p3 3rd point.
 */
bool LC_CreationArc::createFrom3P(const RS_Vector& p1, const RS_Vector& p2, const RS_Vector& p3, RS_ArcData& data) {
    const RS_Vector vra = p2 - p1;
    const RS_Vector vrb = p3 - p1;
    const double ra2 = vra.squared() * 0.5;
    const double rb2 = vrb.squared() * 0.5;
    double crossp = vra.x * vrb.y - vra.y * vrb.x;
    if (fabs(crossp) < RS_TOLERANCE2) {
        RS_DEBUG->print(RS_Debug::D_WARNING, "RS_Arc::createFrom3P(): " "Cannot create a arc with radius 0.0.");
        return false;
    }
    crossp = 1. / crossp;
    data.center.set((ra2 * vrb.y - rb2 * vra.y) * crossp, (rb2 * vra.x - ra2 * vrb.x) * crossp);
    data.radius = data.center.magnitude();
    data.center += p1;
    data.angle1 = data.center.angleTo(p1);
    data.angle2 = data.center.angleTo(p3);
    data.reversed = RS_Math::isAngleBetween(data.center.angleTo(p2),data.angle1, data.angle2, true);
    return true;
}

/**
 * Creates an arc from its startpoint, endpoint, start direction (angle)
 * and radius.
 *
 * @retval true Successfully created arc
 * @retval false Cannot create arc (radius to small or endpoint to far away)
 */
bool LC_CreationArc::createFrom2PDirectionRadius(const RS_Vector& startPoint, const RS_Vector& endPoint, const double direction1,
                                         const double radius, RS_ArcData& data) {
    const RS_Vector ortho = RS_Vector::polar(radius, direction1 + M_PI_2);
    const RS_Vector center1 = startPoint + ortho;
    const RS_Vector center2 = startPoint - ortho;

    if (center1.distanceTo(endPoint) < center2.distanceTo(endPoint)) {
        data.center = center1;
    }
    else {
        data.center = center2;
    }

    data.radius = radius;
    data.angle1 = data.center.angleTo(startPoint);
    data.angle2 = data.center.angleTo(endPoint);
    data.reversed = false;

    const double diff = RS_Math::correctAngle(data.getDirection1() - direction1);
    if (fabs(diff - M_PI) < 1.0e-1) {
        data.reversed = true;
    }
    // calculateBorders();

    return true;
}

/**
 * Creates an arc from its startpoint, endpoint, start direction (angle)
 * and angle length.
 *
 * @retval true Successfully created arc
 * @retval false Cannot create arc (radius to small or endpoint to far away)
 */
bool LC_CreationArc::createFrom2PDirectionAngle(const RS_Vector& startPoint, const RS_Vector& endPoint, double direction1, double angleLength, RS_ArcData& data) {
    if (angleLength <= RS_TOLERANCE_ANGLE || angleLength > 2. * M_PI - RS_TOLERANCE_ANGLE) {
        return false;
    }
    RS_Line l0{nullptr, startPoint, startPoint - RS_Vector{direction1}};
    const double halfA = 0.5 * angleLength;
    l0.rotate(startPoint, halfA);

    double d0;
    RS_Vector vEnd0 = l0.getNearestPointOnEntity(endPoint, false, &d0);
    RS_Line l1 = l0;
    l1.rotate(startPoint, -angleLength);
    double d1;
    RS_Vector vEnd1 = l1.getNearestPointOnEntity(endPoint, false, &d1);
    if (d1 < d0) {
        vEnd0 = vEnd1;
        l0 = l1;
    }

    l0.rotate((startPoint + vEnd0) * 0.5, M_PI_2);

    l1 = RS_Line{nullptr, startPoint, startPoint + RS_Vector{direction1 + M_PI_2}};

    const auto sol = RS_Information::getIntersection(&l0, &l1, false);
    if (sol.size() == 0) {
        return false;
    }

    data.center = sol.at(0);

    data.radius = data.center.distanceTo(startPoint);
    data.angle1 = data.center.angleTo(startPoint);
    data.reversed = false;

    double diff = RS_Math::correctAngle(data.getDirection1() - direction1);
    if (fabs(diff - M_PI) < 1.0e-1) {
        data.angle2 = RS_Math::correctAngle(data.angle1 - angleLength);
        data.reversed = true;
    }
    else {
        data.angle2 = RS_Math::correctAngle(data.angle1 + angleLength);
    }
    // calculateBorders();
    return true;
}

/**
 * Creates an arc from its startpoint, endpoint and bulge.
 */
bool LC_CreationArc::createFrom2PBulge(const RS_Vector& startPoint, const RS_Vector& endPoint, const double bulge, RS_ArcData& data) {
    data.reversed = bulge < 0.0;
    const double alpha = std::atan(bulge) * 4.0;

    const RS_Vector middle = (startPoint + endPoint) / 2.0;
    const double dist = startPoint.distanceTo(endPoint) / 2.0;

    // alpha can't be 0.0 at this point
    data.radius = std::abs(dist / std::sin(alpha / 2.0));

    const double wu = std::abs(data.radius * data.radius - dist * dist);
    double angle = startPoint.angleTo(endPoint);
    const bool reversed = std::signbit(bulge);
    angle = reversed ? angle - M_PI_2 : angle + M_PI_2;

    const double h = (std::abs(alpha) > M_PI) ? -std::sqrt(wu) : std::sqrt(wu);

    data.center.setPolar(h, angle);
    data.center += middle;
    data.angle1 = data.center.angleTo(startPoint);
    data.angle2 = data.center.angleTo(endPoint);
    return true;
}
