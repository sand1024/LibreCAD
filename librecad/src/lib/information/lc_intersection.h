
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

#ifndef LC_INTERSECTION_H
#define LC_INTERSECTION_H

#include "rs_vector.h"

class RS_Ellipse;
class RS_Arc;
class RS_Circle;
class RS_Line;

namespace LC_Intersection {
    RS_VectorSolutions getIntersectionSegmentSegment(const RS_Line* seg1, const RS_Line* seg2);
    RS_VectorSolutions getIntersectionSegmentCircle(const RS_Line* segment, const RS_Circle* circle);
    RS_VectorSolutions getIntersectionSegmentArc(const RS_Line* segment, const RS_Arc* arc);
    RS_VectorSolutions getIntersectionSegmentEllipse(const RS_Line* segment, const RS_Ellipse* ellipse);

};

#endif
