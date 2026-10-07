/****************************************************************************
**
* Action that creates a set of lines, with support of angle and "snake" mode

Copyright (C) 2024 LibreCAD.org
Copyright (C) 2024 Dongxu Li (dongxuli2011 at gmail.com)

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

#ifndef RS_COMMANDITEMS_H
#define RS_COMMANDITEMS_H

#include <vector>
#include <QObject>

#include "rs.h"

struct LC_CommandTrigger {
    const char* text{nullptr};
    const char* disambiguation{nullptr};

    constexpr LC_CommandTrigger() = default;
    constexpr LC_CommandTrigger(const char* t, const char* d = nullptr)
        : text(t), disambiguation(d) {
    }

    bool isEmpty() const {
        return text == nullptr || *text == '\0';
    }
};

struct LC_CommandItem {
    RS2::ActionType actionType{RS2::ActionNone};
    LC_CommandTrigger primary{};
    LC_CommandTrigger keycode{};
    std::vector<LC_CommandTrigger> aliases{};
};

struct LC_KeywordItem {
    LC_CommandTrigger primary{};
    std::vector<LC_CommandTrigger> aliases{};
};

const LC_CommandItem g_commandList[] = {

    /* =========================================================================
     * LAYER COMMANDS
     * ========================================================================= */
    {
        RS2::ActionLayersAddCmd,
        QT_TRANSLATE_NOOP3("cmd", "newlayer", "add layer - command"),
        QT_TRANSLATE_NOOP3("cmd", "nl",       "add layer - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "cnly",    "add layer - alias"),
            QT_TRANSLATE_NOOP3("cmd", "laynew",  "add layer - alias"),
            QT_TRANSLATE_NOOP3("cmd", "cnlayer", "add layer - alias")
        }
    },
    {
        RS2::ActionLayersActivateCmd,
        QT_TRANSLATE_NOOP3("cmd", "layer", "activate layer - command"),
        QT_TRANSLATE_NOOP3("cmd", "ly",    "activate layer - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "csly",    "activate layer - alias"),
            QT_TRANSLATE_NOOP3("cmd", "laycur",  "activate layer - alias"),
            QT_TRANSLATE_NOOP3("cmd", "cslayer", "activate layer - alias")
        }
    },

    /* =========================================================================
     * LINE & POLYGON COMMANDS
     * ========================================================================= */
    {
        RS2::ActionDrawLine,
        QT_TRANSLATE_NOOP3("cmd", "line", "draw line - command"),
        QT_TRANSLATE_NOOP3("cmd", "li",   "draw line - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "l",      "draw line - alias"),
            QT_TRANSLATE_NOOP3("cmd", "line2p", "draw line - alias")
        }
    },
    {
        RS2::ActionDrawSnakeLine,
        QT_TRANSLATE_NOOP3("cmd", "sline", "draw snake line - command"),
        QT_TRANSLATE_NOOP3("cmd", "sl",    "draw snake line - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "sli", "draw snake line - alias")
        }
    },
    {
        RS2::ActionDrawSnakeLineX,
        QT_TRANSLATE_NOOP3("cmd", "slinex", "draw snake line x - command"),
        QT_TRANSLATE_NOOP3("cmd", "sx",     "draw snake line x - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "slx",  "draw snake line x - alias"),
            QT_TRANSLATE_NOOP3("cmd", "slix", "draw snake line x - alias")
        }
    },
    {
        RS2::ActionDrawSnakeLineY,
        QT_TRANSLATE_NOOP3("cmd", "sliney", "draw snake line y - command"),
        QT_TRANSLATE_NOOP3("cmd", "sy",     "draw snake line y - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "sly",  "draw snake line y - alias"),
            QT_TRANSLATE_NOOP3("cmd", "sliy", "draw snake line y - alias")
        }
    },
    {
        RS2::ActionDrawLineDirect,
        QT_TRANSLATE_NOOP3("cmd", "dfast", "draw fast - command"),
        QT_TRANSLATE_NOOP3("cmd", "df",    "draw fast - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "fastline", "draw fast - alias")
        }
    },
    {
        RS2::ActionDrawLineAngle,
        QT_TRANSLATE_NOOP3("cmd", "lineang", "line angle - command"),
        QT_TRANSLATE_NOOP3("cmd", "la",      "line angle - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "lang",    "line angle - alias"),
            QT_TRANSLATE_NOOP3("cmd", "angline", "line angle - alias")
        }
    },
    {
        RS2::ActionDrawLineHorizontal,
        QT_TRANSLATE_NOOP3("cmd", "linehor", "line horizontal - command"),
        QT_TRANSLATE_NOOP3("cmd", "lh",      "line horizontal - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "hor",     "line horizontal - alias"),
            QT_TRANSLATE_NOOP3("cmd", "lhor",    "line horizontal - alias"),
            QT_TRANSLATE_NOOP3("cmd", "horline", "line horizontal - alias")
        }
    },
    {
        RS2::ActionDrawLineVertical,
        QT_TRANSLATE_NOOP3("cmd", "linever", "line vertical - command"),
        QT_TRANSLATE_NOOP3("cmd", "lv",      "line vertical - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ver",     "line vertical - alias"),
            QT_TRANSLATE_NOOP3("cmd", "lver",    "line vertical - alias"),
            QT_TRANSLATE_NOOP3("cmd", "verline", "line vertical - alias")
        }
    },

    // Rectangles: r1, r2, r3 series
    {
        RS2::ActionDrawLineRectangle,
        QT_TRANSLATE_NOOP3("cmd", "rectangle", "rectangle - command"),
        QT_TRANSLATE_NOOP3("cmd", "re",        "rectangle - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "rec",     "rectangle - alias"),
            QT_TRANSLATE_NOOP3("cmd", "rect",    "rectangle - alias"),
            QT_TRANSLATE_NOOP3("cmd", "linerec", "rectangle - alias")
        }
    },
    {
        RS2::ActionDrawRectangle1Point,
        QT_TRANSLATE_NOOP3("cmd", "rect1", "rectangle 1 point - command"),
        QT_TRANSLATE_NOOP3("cmd", "r1",    "rectangle 1 point - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "re1", "rectangle 1 point - alias")
        }
    },
    {
        RS2::ActionDrawRectangle2Points,
        QT_TRANSLATE_NOOP3("cmd", "rect2", "rectangle 2 points - command"),
        QT_TRANSLATE_NOOP3("cmd", "r2",    "rectangle 2 points - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "rc",  "rectangle 2 points - alias"),
            QT_TRANSLATE_NOOP3("cmd", "re2", "rectangle 2 points - alias")
        }
    },
    {
        RS2::ActionDrawRectangle3Points,
        QT_TRANSLATE_NOOP3("cmd", "rect3", "rectangle 3 points - command"),
        QT_TRANSLATE_NOOP3("cmd", "r3",    "rectangle 3 points - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "re3", "rectangle 3 points - alias")
        }
    },

    // Dividing, Marks & Radiants
    {
        RS2::ActionDrawSliceDivideLine,
        QT_TRANSLATE_NOOP3("cmd", "sliceline", "slice line - command"),
        QT_TRANSLATE_NOOP3("cmd", "ls",        "slice line - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "sll",    "slice line - alias"),
            QT_TRANSLATE_NOOP3("cmd", "slicel", "slice line - alias")
        }
    },
    {
        RS2::ActionDrawSliceDivideCircle,
        QT_TRANSLATE_NOOP3("cmd", "slicecircle", "slice circle - command"),
        QT_TRANSLATE_NOOP3("cmd", "cs",          "slice circle - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "slc",    "slice circle - alias"),
            QT_TRANSLATE_NOOP3("cmd", "slicec", "slice circle - alias")
        }
    },
    {
        RS2::ActionDrawStar,
        QT_TRANSLATE_NOOP3("cmd", "star", "draw star - command"),
        QT_TRANSLATE_NOOP3("cmd", "st",   "draw star - keycode"),
        {}
    },
    {
        RS2::ActionDrawCenterMark,
        QT_TRANSLATE_NOOP3("cmd", "cross", "center mark - command"),
        QT_TRANSLATE_NOOP3("cmd", "cx",    "center mark - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "cm",         "center mark - alias"),
            QT_TRANSLATE_NOOP3("cmd", "centermark", "center mark - alias")
        }
    },
    {
        RS2::ActionDrawBoundingBox,
        QT_TRANSLATE_NOOP3("cmd", "bbox", "bounding box - command"),
        QT_TRANSLATE_NOOP3("cmd", "bb",   "bounding box - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "boundingbox", "bounding box - alias")
        }
    },
    {
        RS2::ActionDrawCenterLine,
        QT_TRANSLATE_NOOP3("cmd", "centerline", "centerline - command"),
        QT_TRANSLATE_NOOP3("cmd", "cl",         "centerline - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ml",      "centerline - alias"),
            QT_TRANSLATE_NOOP3("cmd", "midline", "centerline - alias")
        }
    },
    {
        RS2::ActionDrawLineRadiant,
        QT_TRANSLATE_NOOP3("cmd", "radiant", "radiant line - command"),
        QT_TRANSLATE_NOOP3("cmd", "rl",      "radiant line - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "lineradiant", "radiant line - alias")
        }
    },
    {
        RS2::ActionDrawPointsLine,
        QT_TRANSLATE_NOOP3("cmd", "linepoints", "line of points - command"),
        QT_TRANSLATE_NOOP3("cmd", "ln",         "line of points - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "lpoints", "line of points - alias"),
            QT_TRANSLATE_NOOP3("cmd", "ptsline", "line of points - alias")
        }
    },
    {
        RS2::ActionDrawPointsMiddle,
        QT_TRANSLATE_NOOP3("cmd", "midpoint", "middle points - command"),
        QT_TRANSLATE_NOOP3("cmd", "pm",       "middle points - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "mpoint", "middle points - alias")
        }
    },
    {
        RS2::ActionDrawCircleByArc,
        QT_TRANSLATE_NOOP3("cmd", "circlebyarc", "circle by arc - command"),
        QT_TRANSLATE_NOOP3("cmd", "ca",          "circle by arc - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "cba", "circle by arc - alias")
        }
    },

    // Parallel, Bisector & Tangents
    {
        RS2::ActionDrawLineParallel,
        QT_TRANSLATE_NOOP3("cmd", "parallel", "parallel line - command"),
        QT_TRANSLATE_NOOP3("cmd", "pa",       "parallel line - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ll",         "parallel line - alias"),
            QT_TRANSLATE_NOOP3("cmd", "linepar",    "parallel line - alias"),
            QT_TRANSLATE_NOOP3("cmd", "lineoff",    "parallel line - alias"),
            QT_TRANSLATE_NOOP3("cmd", "offsetline", "parallel line - alias")
        }
    },
    {
        RS2::ActionDrawLineParallelThrough,
        QT_TRANSLATE_NOOP3("cmd", "lineparthro", "parallel through point - command"),
        QT_TRANSLATE_NOOP3("cmd", "lp",          "parallel through point - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ptp", "parallel through point - alias")
        }
    },
    {
        RS2::ActionDrawLineBisector,
        QT_TRANSLATE_NOOP3("cmd", "bisector", "line bisector - command"),
        QT_TRANSLATE_NOOP3("cmd", "bi",       "line bisector - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "bisect",     "line bisector - alias"),
            QT_TRANSLATE_NOOP3("cmd", "linebisect", "line bisector - alias")
        }
    },
    {
        RS2::ActionDrawLineTangent1,
        QT_TRANSLATE_NOOP3("cmd", "linetancp", "tangent point circle - command"),
        QT_TRANSLATE_NOOP3("cmd", "lt",        "tangent point circle - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "tanpc",     "tangent point circle - alias"),
            QT_TRANSLATE_NOOP3("cmd", "tangentpc", "tangent point circle - alias")
        }
    },
    {
        RS2::ActionDrawLineTangent2,
        QT_TRANSLATE_NOOP3("cmd", "linetan2c", "tangent two circles - command"),
        QT_TRANSLATE_NOOP3("cmd", "lc",        "tangent two circles - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "tan2c",     "tangent two circles - alias"),
            QT_TRANSLATE_NOOP3("cmd", "tangent2c", "tangent two circles - alias")
        }
    },
    {
        RS2::ActionDrawLineOrthTan,
        QT_TRANSLATE_NOOP3("cmd", "linetancper", "tangent line circle - command"),
        QT_TRANSLATE_NOOP3("cmd", "or",          "tangent line circle - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "tanorth", "tangent line circle - alias")
        }
    },
    {
        RS2::ActionDrawLineOrthogonal,
        QT_TRANSLATE_NOOP3("cmd", "perpendicular", "line orthogonal - command"),
        QT_TRANSLATE_NOOP3("cmd", "lo",            "line orthogonal - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ortho",    "line orthogonal - alias"),
            QT_TRANSLATE_NOOP3("cmd", "lineperp", "line orthogonal - alias")
        }
    },
    {
        RS2::ActionDrawLineRelAngle,
        QT_TRANSLATE_NOOP3("cmd", "linerelang", "line relative angle - command"),
        QT_TRANSLATE_NOOP3("cmd", "lr",         "line relative angle - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "relangle", "line relative angle - alias")
        }
    },
    {
        RS2::ActionDrawLineAngleRel,
        QT_TRANSLATE_NOOP3("cmd", "angleline", "line angle from line - command"),
        QT_TRANSLATE_NOOP3("cmd", "ag", "line angle from line - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "aline", "line angle from line - alias")
        }
    },
    {
        RS2::ActionDrawLineOrthogonalRel,
        QT_TRANSLATE_NOOP3("cmd", "ortline", "line orthogonal from line - command"),
        QT_TRANSLATE_NOOP3("cmd", "ol",      "line orthogonal from line - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "oline", "line orthogonal from line - alias")
        }
    },
    {
        RS2::ActionDrawLineFromPointToLine,
        QT_TRANSLATE_NOOP3("cmd", "point2line", "line point to line - command"),
        QT_TRANSLATE_NOOP3("cmd", "fl",         "line point to line - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "p2l", "line point to line - alias")
        }
    },

    // Polygons
    {
        RS2::ActionDrawLinePolygonCenCor,
        QT_TRANSLATE_NOOP3("cmd", "polygoncencor", "polygon center corner - command"),
        QT_TRANSLATE_NOOP3("cmd", "pp",            "polygon center corner - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "pcp",    "polygon center corner - alias"),
            QT_TRANSLATE_NOOP3("cmd", "polycp", "polygon center corner - alias")
        }
    },
    {
        RS2::ActionDrawLinePolygonCenTan,
        QT_TRANSLATE_NOOP3("cmd", "polygoncentan", "polygon center tangent - command"),
        QT_TRANSLATE_NOOP3("cmd", "pv",            "polygon center tangent - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "pct",    "polygon center tangent - alias"),
            QT_TRANSLATE_NOOP3("cmd", "polyct", "polygon center tangent - alias")
        }
    },
    {
        RS2::ActionDrawLinePolygonSideSide,
        QT_TRANSLATE_NOOP3("cmd", "polygonvv", "polygon side side - command"),
        QT_TRANSLATE_NOOP3("cmd", "ps",        "polygon side side - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "pvv",      "polygon side side - alias"),
            QT_TRANSLATE_NOOP3("cmd", "polyside", "polygon side side - alias")
        }
    },
    {
        RS2::ActionDrawLinePolygonCorCor,
        QT_TRANSLATE_NOOP3("cmd", "polygon2v", "polygon 2 corners - command"),
        QT_TRANSLATE_NOOP3("cmd", "p2",        "polygon 2 corners - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "p2v",   "polygon 2 corners - alias"),
            QT_TRANSLATE_NOOP3("cmd", "poly2", "polygon 2 corners - alias")
        }
    },

    /* =========================================================================
     * CIRCLE COMMANDS
     * ========================================================================= */
    {
        RS2::ActionDrawCircleCenterPoint,
        QT_TRANSLATE_NOOP3("cmd", "circle", "draw circle - command"),
        QT_TRANSLATE_NOOP3("cmd", "ci",     "draw circle - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "c", "draw circle - alias")
        }
    },
    {
        RS2::ActionDrawCircle2Points,
        QT_TRANSLATE_NOOP3("cmd", "circle2p", "circle 2 points - command"),
        QT_TRANSLATE_NOOP3("cmd", "c2",       "circle 2 points - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "c2p", "circle 2 points - alias")
        }
    },
    {
        RS2::ActionDrawCircle2PointsRadius,
        QT_TRANSLATE_NOOP3("cmd", "circle2pr", "circle 2 points radius - command"),
        QT_TRANSLATE_NOOP3("cmd", "cc",        "circle 2 points radius - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "c2r", "circle 2 points radius - alias")
        }
    },
    {
        RS2::ActionDrawCircle3Points,
        QT_TRANSLATE_NOOP3("cmd", "circle3p", "circle 3 points - command"),
        QT_TRANSLATE_NOOP3("cmd", "c3",       "circle 3 points - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "c3p", "circle 3 points - alias")
        }
    },
    {
        RS2::ActionDrawCircleCenterRadius,
        QT_TRANSLATE_NOOP3("cmd", "circlecr", "circle center radius - command"),
        QT_TRANSLATE_NOOP3("cmd", "cr",       "circle center radius - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ccr", "circle center radius - alias")
        }
    },
    // Tangent Circles
    {
        RS2::ActionDrawCircleTangental2Entities1Point,
        QT_TRANSLATE_NOOP3("cmd", "circletan2cp", "circle tangent 2 entities 1 point - command"),
        QT_TRANSLATE_NOOP3("cmd", "tr",           "circle tangent 2 entities 1 point - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ttp",   "circle tangent 2 entities 1 point - alias"),
            QT_TRANSLATE_NOOP3("cmd", "cttp",  "circle tangent 2 entities 1 point - alias"),
            QT_TRANSLATE_NOOP3("cmd", "tan2p", "circle tangent 2 entities 1 point - alias")
        }
    },
    {
        RS2::ActionDrawCircleTangental1Entity2Points,
        QT_TRANSLATE_NOOP3("cmd", "circletan2p", "circle tangent 1 entity 2 points - command"),
        QT_TRANSLATE_NOOP3("cmd", "td",          "circle tangent 1 entity 2 points - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "tpp",    "circle tangent 1 entity 2 points - alias"),
            QT_TRANSLATE_NOOP3("cmd", "ctpp",   "circle tangent 1 entity 2 points - alias"),
            QT_TRANSLATE_NOOP3("cmd", "tan1p2", "circle tangent 1 entity 2 points - alias")
        }
    },
    {
        RS2::ActionDrawCircleTan2EntitiesRadius,
        QT_TRANSLATE_NOOP3("cmd", "circletan2cr", "circle tangent 2 entities radius - command"),
        QT_TRANSLATE_NOOP3("cmd", "tc",           "circle tangent 2 entities radius - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ttr",   "circle tangent 2 entities radius - alias"),
            QT_TRANSLATE_NOOP3("cmd", "cttr",  "circle tangent 2 entities radius - alias"),
            QT_TRANSLATE_NOOP3("cmd", "tan2r", "circle tangent 2 entities radius - alias")
        }
    },
    {
        RS2::ActionDrawCircleTan3Entities,
        QT_TRANSLATE_NOOP3("cmd", "circletan3", "circle tangent 3 entities - command"),
        QT_TRANSLATE_NOOP3("cmd", "t3",         "circle tangent 3 entities - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ct3",  "circle tangent 3 entities - alias"),
            QT_TRANSLATE_NOOP3("cmd", "ttt",  "circle tangent 3 entities - alias"),
            QT_TRANSLATE_NOOP3("cmd", "tan3", "circle tangent 3 entities - alias")
        }
    },

    /* =========================================================================
     * CURVE & ARC COMMANDS
     * ========================================================================= */
    {
        RS2::ActionDrawArc,
        QT_TRANSLATE_NOOP3("cmd", "arc", "draw arc - command"),
        QT_TRANSLATE_NOOP3("cmd", "ar",  "draw arc - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "a", "draw arc - alias")
        }
    },
    {
        RS2::ActionDrawArc3P,
        QT_TRANSLATE_NOOP3("cmd", "arc3p", "arc 3 points - command"),
        QT_TRANSLATE_NOOP3("cmd", "a3",    "arc 3 points - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "a3p", "arc 3 points - alias")
        }
    },
    {
        RS2::ActionDrawArcTangential,
        QT_TRANSLATE_NOOP3("cmd", "arctan", "arc tangential - command"),
        QT_TRANSLATE_NOOP3("cmd", "at",     "arc tangential - keycode"),
        {}
    },
    {
        RS2::ActionDrawArc2PRadius,
        QT_TRANSLATE_NOOP3("cmd", "arc2pr", "arc 2 points radius - command"),
        QT_TRANSLATE_NOOP3("cmd", "a2",     "arc 2 points radius - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "a2r",       "arc 2 points radius - alias"),
            QT_TRANSLATE_NOOP3("cmd", "arc2r",     "arc 2 points radius - alias"),
            QT_TRANSLATE_NOOP3("cmd", "arcradius", "arc 2 points radius - alias")
        }
    },
    {
        RS2::ActionDrawArc2PLength,
        QT_TRANSLATE_NOOP3("cmd", "arc2pl", "arc 2 points length - command"),
        {},
        {
            QT_TRANSLATE_NOOP3("cmd", "a2l",       "arc 2 points length - alias"),
            QT_TRANSLATE_NOOP3("cmd", "arc2l",     "arc 2 points length - alias"),
            QT_TRANSLATE_NOOP3("cmd", "arclength", "arc 2 points length - alias")
        }
    },
    {
        RS2::ActionDrawArc2PHeight,
        QT_TRANSLATE_NOOP3("cmd", "arc2ph", "arc 2 points height - command"),
        QT_TRANSLATE_NOOP3("cmd", "ah",     "arc 2 points height - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "a2h",        "arc 2 points height - alias"),
            QT_TRANSLATE_NOOP3("cmd", "arc2h",      "arc 2 points height - alias"),
            QT_TRANSLATE_NOOP3("cmd", "arcsagitta", "arc 2 points height - alias")
        }
    },
    {
        RS2::ActionDrawArc2PAngle,
        QT_TRANSLATE_NOOP3("cmd", "arc2pa", "arc 2 points angle - command"),
        QT_TRANSLATE_NOOP3("cmd", "ap",     "arc 2 points angle - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "a2a",      "arc 2 points angle - alias"),
            QT_TRANSLATE_NOOP3("cmd", "arc2a",    "arc 2 points angle - alias"),
            QT_TRANSLATE_NOOP3("cmd", "arcangle", "arc 2 points angle - alias")
        }
    },

    /* =========================================================================
     * SPLINE, PARABOLA & ELLIPSE
     * ========================================================================= */
    {
        RS2::ActionDrawSpline,
        QT_TRANSLATE_NOOP3("cmd", "spline", "draw spline - command"),
        QT_TRANSLATE_NOOP3("cmd", "sf",     "draw spline - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "spl", "draw spline - alias")
        }
    },
    {
        RS2::ActionDrawSplinePoints,
        QT_TRANSLATE_NOOP3("cmd", "splinepoints", "spline through points - command"),
        QT_TRANSLATE_NOOP3("cmd", "sp",           "spline through points - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "stp",     "spline through points - alias"),
            QT_TRANSLATE_NOOP3("cmd", "spline2", "spline through points - alias")
        }
    },
    {
        RS2::ActionDrawEllipseArcAxis,
        QT_TRANSLATE_NOOP3("cmd", "arcellc2ax", "ellipse arc axis - command"),
        QT_TRANSLATE_NOOP3("cmd", "ea",         "ellipse arc axis - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ellipsearc", "ellipse arc axis - alias")
        }
    },
    {
        RS2::ActionDrawEllipseArc1Point,
        QT_TRANSLATE_NOOP3("cmd", "arcellc1ax", "ellipse arc 1 point - command"),
        {},
        {
            QT_TRANSLATE_NOOP3("cmd", "ae1", "ellipse arc 1 point - alias")
        }
    },
    {
        RS2::ActionDrawParabola4Points,
        QT_TRANSLATE_NOOP3("cmd", "parabola4p", "parabola 4 points - command"),
        QT_TRANSLATE_NOOP3("cmd", "p4",         "parabola 4 points - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "pl4", "parabola 4 points - alias")
        }
    },
    {
        RS2::ActionDrawParabolaFocusDiretrix,
        QT_TRANSLATE_NOOP3("cmd", "parabolafd", "parabola focus directrix - command"),
        QT_TRANSLATE_NOOP3("cmd", "pf",         "parabola focus directrix - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "plfd", "parabola focus directrix - alias")
        }
    },
    {
        RS2::ActionDrawLineFreehand,
        QT_TRANSLATE_NOOP3("cmd", "freehand", "freehand line - command"),
        QT_TRANSLATE_NOOP3("cmd", "fh",       "freehand line - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "fhl",  "freehand line - alias"),
            QT_TRANSLATE_NOOP3("cmd", "free", "freehand line - alias")
        }
    },
    {
        RS2::ActionDrawEllipseAxis,
        QT_TRANSLATE_NOOP3("cmd", "ellipse", "ellipse axis - command"),
        QT_TRANSLATE_NOOP3("cmd", "el",      "ellipse axis - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ellipsec2p", "ellipse axis - alias")
        }
    },
    {
        RS2::ActionDrawEllipse1Point,
        QT_TRANSLATE_NOOP3("cmd", "ellipsec1p", "ellipse 1 point - command"),
        QT_TRANSLATE_NOOP3("cmd", "e1",         "ellipse 1 point - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ea1", "ellipse 1 point - alias"),
            QT_TRANSLATE_NOOP3("cmd", "el1", "ellipse 1 point - alias")
        }
    },
    {
        RS2::ActionDrawEllipseFociPoint,
        QT_TRANSLATE_NOOP3("cmd", "ellipse3p", "ellipse foci - command"),
        QT_TRANSLATE_NOOP3("cmd", "ef",        "ellipse foci - keycode"),
        {}
    },
    {
        RS2::ActionDrawEllipse4Points,
        QT_TRANSLATE_NOOP3("cmd", "ellipse4p", "ellipse 4 points - command"),
        QT_TRANSLATE_NOOP3("cmd", "e4",        "ellipse 4 points - keycode"),
        {}
    },
    {
        RS2::ActionDrawEllipseCenter3Points,
        QT_TRANSLATE_NOOP3("cmd", "ellipsec3p", "ellipse center 3 points - command"),
        QT_TRANSLATE_NOOP3("cmd", "e3",         "ellipse center 3 points - keycode"),
        {}
    },
    {
        RS2::ActionDrawEllipseInscribe,
        QT_TRANSLATE_NOOP3("cmd", "ellipseinscribed", "ellipse inscribe - command"),
        QT_TRANSLATE_NOOP3("cmd", "ei",               "ellipse inscribe - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ie", "ellipse inscribe - alias")
        }
    },

    /* =========================================================================
     * POLYLINE COMMANDS
     * ========================================================================= */
    {
        RS2::ActionDrawPolyline,
        QT_TRANSLATE_NOOP3("cmd", "polyline", "draw polyline - command"),
        QT_TRANSLATE_NOOP3("cmd", "pl",       "draw polyline - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "pline", "draw polyline - alias")
        }
    },
    {
        RS2::ActionPolylineAdd,
        QT_TRANSLATE_NOOP3("cmd", "plineadd", "polyline add node - command"),
        QT_TRANSLATE_NOOP3("cmd", "pi",       "polyline add node - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "padd", "polyline add node - alias")
        }
    },
    {
        RS2::ActionPolylineAppend,
        QT_TRANSLATE_NOOP3("cmd", "plineapp", "polyline append node - command"),
        QT_TRANSLATE_NOOP3("cmd", "pn",       "polyline append node - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "papp", "polyline append node - alias")
        }
    },
    {
        RS2::ActionPolylineDel,
        QT_TRANSLATE_NOOP3("cmd", "plinedel", "polyline delete node - command"),
        QT_TRANSLATE_NOOP3("cmd", "pd",       "polyline delete node - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "pdel", "polyline delete node - alias")
        }
    },
    {
        RS2::ActionPolylineDelBetween,
        QT_TRANSLATE_NOOP3("cmd", "plinedeltwn", "polyline delete between nodes - command"),
        QT_TRANSLATE_NOOP3("cmd", "pb",          "polyline delete between nodes - keycode"),
        {
             QT_TRANSLATE_NOOP3("cmd", "pdelbetween", "polyline delete between nodes - alias")
        }
    },
    {
        RS2::ActionPolylineTrim,
        QT_TRANSLATE_NOOP3("cmd", "plinetrm", "polyline trim - command"),
        QT_TRANSLATE_NOOP3("cmd", "pt",       "polyline trim - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ptrim", "polyline trim - alias")
        }
    },
    {
        RS2::ActionPolylineEquidistant,
        QT_TRANSLATE_NOOP3("cmd", "plinepar", "polyline equidistant - command"),
        QT_TRANSLATE_NOOP3("cmd", "pe",       "polyline equidistant - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "poffset", "polyline equidistant - alias")
        }
    },
    {
        RS2::ActionPolylineSegment,
        QT_TRANSLATE_NOOP3("cmd", "plinejoin", "polyline from segments - command"),
        QT_TRANSLATE_NOOP3("cmd", "pj",        "polyline from segments - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "pjoin", "polyline from segments - alias")
        }
    },
    {
        RS2::ActionDrawDual,
        QT_TRANSLATE_NOOP3("cmd", "dual", "dual curve - command"),
        QT_TRANSLATE_NOOP3("cmd", "du",   "dual curve - keycode"),
        {}
    },

    /* =========================================================================
     * SELECT COMMANDS
     * ========================================================================= */
    {
        RS2::ActionSelectAll,
        QT_TRANSLATE_NOOP3("cmd", "selectall", "select all - command"),
        QT_TRANSLATE_NOOP3("cmd", "sa",        "select all - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "selall", "select all - alias")
        }
    },
    {
        RS2::ActionDeselectAll,
        QT_TRANSLATE_NOOP3("cmd", "deselectall", "deselect all - command"),
        QT_TRANSLATE_NOOP3("cmd", "dx",          "deselect all - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "tn",       "deselect all - alias"),
            QT_TRANSLATE_NOOP3("cmd", "deselall", "deselect all - alias")
        }
    },
    {
        RS2::ActionSelectInvert,
        QT_TRANSLATE_NOOP3("cmd", "invertselect", "select invert - command"),
        QT_TRANSLATE_NOOP3("cmd", "is",           "select invert - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "selinv", "select invert - alias")
        }
    },
    {
        RS2::ActionSelectQuick,
        QT_TRANSLATE_NOOP3("cmd", "selectquick", "select quick - command"),
        QT_TRANSLATE_NOOP3("cmd", "sq",          "select quick - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "qs",      "select quick - alias"),
            QT_TRANSLATE_NOOP3("cmd", "qselect", "select quick - alias")
        }
    },
    {
        RS2::ActionSelectModeToggle,
        QT_TRANSLATE_NOOP3("cmd", "smtoggle", "select mode toggle - command"),
        QT_TRANSLATE_NOOP3("cmd", "ms",       "select mode toggle - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "smt", "select mode toggle - alias")
        }
    },

    /* =========================================================================
     * DIMENSION COMMANDS
     * ========================================================================= */
    {
        RS2::ActionDimAligned,
        QT_TRANSLATE_NOOP3("cmd", "dimaligned", "dimension aligned - command"),
        QT_TRANSLATE_NOOP3("cmd", "ds",         "dimension aligned - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "dal", "dimension aligned - alias")
        }
    },
    {
        RS2::ActionDimLinear,
        QT_TRANSLATE_NOOP3("cmd", "dimlinear", "dimension linear - command"),
        QT_TRANSLATE_NOOP3("cmd", "dl",        "dimension linear - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "dli", "dimension linear - alias")
        }
    },
    {
        RS2::ActionDimOrdinate,
        QT_TRANSLATE_NOOP3("cmd", "dimord", "dimension ordinate - command"),
        QT_TRANSLATE_NOOP3("cmd", "do",     "dimension ordinate - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "dor",         "dimension ordinate - alias"),
            QT_TRANSLATE_NOOP3("cmd", "dimordinate", "dimension ordinate - alias")
        }
    },
    {
        RS2::ActionDimOrdRebase,
        QT_TRANSLATE_NOOP3("cmd", "dimordrebase", "dimension ordinate rebase - command"),
        QT_TRANSLATE_NOOP3("cmd", "db",            "dimension ordinate rebase - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "rebase", "dimension ordinate rebase - alias")
        }
    },
    {
        RS2::ActionDimLinearHor,
        QT_TRANSLATE_NOOP3("cmd", "dimhorizontal", "dimension horizontal - command"),
        QT_TRANSLATE_NOOP3("cmd", "dh",            "dimension horizontal - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "dhor",   "dimension horizontal - alias"),
            QT_TRANSLATE_NOOP3("cmd", "dimhor", "dimension horizontal - alias")
        }
    },
    {
        RS2::ActionDimLinearVer,
        QT_TRANSLATE_NOOP3("cmd", "dimvertical", "dimension vertical - command"),
        QT_TRANSLATE_NOOP3("cmd", "dv",          "dimension vertical - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "dver",   "dimension vertical - alias"),
            QT_TRANSLATE_NOOP3("cmd", "dimver", "dimension vertical - alias")
        }
    },
    {
        RS2::ActionDimRadial,
        QT_TRANSLATE_NOOP3("cmd", "dimradius", "dimension radius - command"),
        QT_TRANSLATE_NOOP3("cmd", "dr",        "dimension radius - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "dra",       "dimension radius - alias"),
            QT_TRANSLATE_NOOP3("cmd", "dimradial", "dimension radius - alias")
        }
    },
    {
        RS2::ActionDimDiametric,
        QT_TRANSLATE_NOOP3("cmd", "dimdiameter", "dimension diameter - command"),
        QT_TRANSLATE_NOOP3("cmd", "dd",          "dimension diameter - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ddi",          "dimension diameter - alias"),
            QT_TRANSLATE_NOOP3("cmd", "dimdiametric", "dimension diameter - alias")
        }
    },
    {
        RS2::ActionDimAngular,
        QT_TRANSLATE_NOOP3("cmd", "dimangular", "dimension angular - command"),
        QT_TRANSLATE_NOOP3("cmd", "da",         "dimension angular - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "dan", "dimension angular - alias")
        }
    },
    {
        RS2::ActionDimLeader,
        QT_TRANSLATE_NOOP3("cmd", "dimleader", "dimension leader - command"),
        QT_TRANSLATE_NOOP3("cmd", "ld",        "dimension leader - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "leader",  "dimension leader - alias"),
            QT_TRANSLATE_NOOP3("cmd", "qleader", "dimension leader - alias")
        }
    },
    {
        RS2::ActionDimRegenerate,
        QT_TRANSLATE_NOOP3("cmd", "dimregen", "dimension regenerate - command"),
        QT_TRANSLATE_NOOP3("cmd", "dg",       "dimension regenerate - keycode"),
        {}
    },

    /* =========================================================================
     * MODIFY COMMANDS
     * ========================================================================= */
    {
        RS2::ActionModifyMove,
        QT_TRANSLATE_NOOP3("cmd", "move", "move - command"),
        QT_TRANSLATE_NOOP3("cmd", "mv",   "move - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "m",       "move - alias"),
            QT_TRANSLATE_NOOP3("cmd", "modmove", "move - alias")
        }
    },
    {
        RS2::ActionModifyCopy,
        QT_TRANSLATE_NOOP3("cmd", "copy", "copy - command"),
        QT_TRANSLATE_NOOP3("cmd", "cp",   "copy - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "co", "copy - alias")
        }
    },
    {
        RS2::ActionModifyDuplicate,
        QT_TRANSLATE_NOOP3("cmd", "duplicate", "duplicate - command"),
        QT_TRANSLATE_NOOP3("cmd", "dc",        "duplicate - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "dup", "duplicate - alias")
        }
    },
    {
        RS2::ActionModifyRotate,
        QT_TRANSLATE_NOOP3("cmd", "rotate", "rotate - command"),
        QT_TRANSLATE_NOOP3("cmd", "ro",     "rotate - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "modrotate", "rotate - alias")
        }
    },
    {
        RS2::ActionModifyScale,
        QT_TRANSLATE_NOOP3("cmd", "scale", "scale - command"),
        QT_TRANSLATE_NOOP3("cmd", "sz",    "scale - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "modscale", "scale - alias")
        }
    },
    {
        RS2::ActionModifyMirror,
        QT_TRANSLATE_NOOP3("cmd", "mirror", "mirror - command"),
        QT_TRANSLATE_NOOP3("cmd", "mi",     "mirror - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "modmirror", "mirror - alias")
        }
    },
    {
        RS2::ActionModifyMoveRotate,
        QT_TRANSLATE_NOOP3("cmd", "moverotate", "move rotate - command"),
        QT_TRANSLATE_NOOP3("cmd", "mr",         "move rotate - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "modmovrot", "move rotate - alias")
        }
    },
    {
        RS2::ActionModifyRotateTwice,
        QT_TRANSLATE_NOOP3("cmd", "rotatetwice", "rotate twice - command"),
        QT_TRANSLATE_NOOP3("cmd", "rt",          "rotate twice - keycode"),
        {
           QT_TRANSLATE_NOOP3("cmd", "ro2",     "rotate twice - alias"),
           QT_TRANSLATE_NOOP3("cmd", "rot2",    "rotate twice - alias"),
           QT_TRANSLATE_NOOP3("cmd", "mod2rot", "rotate twice - alias")
        }
    },
    {
        RS2::ActionModifyRevertDirection,
        QT_TRANSLATE_NOOP3("cmd", "reverse", "reverse - command"),
        QT_TRANSLATE_NOOP3("cmd", "vr",      "reverse - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "rev",       "reverse - alias"),
            QT_TRANSLATE_NOOP3("cmd", "modrevert", "reverse - alias")
        }
    },
    {
        RS2::ActionModifyTrim,
        QT_TRANSLATE_NOOP3("cmd", "trim", "trim - command"),
        QT_TRANSLATE_NOOP3("cmd", "tm",   "trim - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "modtrim", "trim - alias")
        }
    },
    {
        RS2::ActionModifyTrim2,
        QT_TRANSLATE_NOOP3("cmd", "trim2", "trim two - command"),
        QT_TRANSLATE_NOOP3("cmd", "t2",    "trim two - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "tm2",     "trim two - alias"),
            QT_TRANSLATE_NOOP3("cmd", "modtrim2", "trim two - alias")
        }
    },
    {
        RS2::ActionModifyTrimAmount,
        QT_TRANSLATE_NOOP3("cmd", "lengthen", "lengthen - command"),
        QT_TRANSLATE_NOOP3("cmd", "le",       "lengthen - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "len",         "lengthen - alias"),
            QT_TRANSLATE_NOOP3("cmd", "modlengthen", "lengthen - alias")
        }
    },
    {
        RS2::ActionModifyOffset,
        QT_TRANSLATE_NOOP3("cmd", "offset", "offset - command"),
        QT_TRANSLATE_NOOP3("cmd", "mo",     "offset - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "o",         "offset - alias"),
            QT_TRANSLATE_NOOP3("cmd", "moff",      "offset - alias"),
            QT_TRANSLATE_NOOP3("cmd", "modoffset", "offset - alias")
        }
    },
    {
        RS2::ActionModifyBevel,
        QT_TRANSLATE_NOOP3("cmd", "chamfer", "bevel chamfer - command"),
        QT_TRANSLATE_NOOP3("cmd", "ch",      "bevel chamfer - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "bev",      "bevel chamfer - alias"),
            QT_TRANSLATE_NOOP3("cmd", "cha",      "bevel chamfer - alias"),
            QT_TRANSLATE_NOOP3("cmd", "bevel",    "bevel chamfer - alias"),
            QT_TRANSLATE_NOOP3("cmd", "modbevel", "bevel chamfer - alias")
        }
    },
    {
        RS2::ActionModifyRound,
        QT_TRANSLATE_NOOP3("cmd", "fillet", "fillet round - command"),
        QT_TRANSLATE_NOOP3("cmd", "fi",     "fillet round - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "f",         "fillet round - alias"),
            QT_TRANSLATE_NOOP3("cmd", "round",     "fillet round - alias"),
            QT_TRANSLATE_NOOP3("cmd", "modfillet", "fillet round - alias")
        }
    },
    {
        RS2::ActionModifyCut,
        QT_TRANSLATE_NOOP3("cmd", "divide", "divide cut - command"),
        QT_TRANSLATE_NOOP3("cmd", "di",     "divide cut - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "cut",       "divide cut - alias"),
            QT_TRANSLATE_NOOP3("cmd", "div",       "divide cut - alias"),
            QT_TRANSLATE_NOOP3("cmd", "moddivide", "divide cut - alias")
        }
    },
    {
        RS2::ActionModifyStretch,
        QT_TRANSLATE_NOOP3("cmd", "stretch", "stretch - command"),
        QT_TRANSLATE_NOOP3("cmd", "ss",      "stretch - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "s",          "stretch - alias"),
            QT_TRANSLATE_NOOP3("cmd", "modstretch", "stretch - alias")
        }
    },
    {
        RS2::ActionModifyEntity,
        QT_TRANSLATE_NOOP3("cmd", "properties", "properties - command"),
        QT_TRANSLATE_NOOP3("cmd", "mp",         "properties - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "pr",            "properties - alias"),
            QT_TRANSLATE_NOOP3("cmd", "prop",          "properties - alias"),
            QT_TRANSLATE_NOOP3("cmd", "props",         "properties - alias"),
            QT_TRANSLATE_NOOP3("cmd", "modproperties", "properties - alias")
        }
    },
    {
        RS2::ActionModifyAttributes,
        QT_TRANSLATE_NOOP3("cmd", "attributes", "attributes - command"),
        QT_TRANSLATE_NOOP3("cmd", "ma",         "attributes - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "attr",    "attributes - alias"),
            QT_TRANSLATE_NOOP3("cmd", "modattr", "attributes - alias")
        }
    },
    {
        RS2::ActionModifyExplodeText,
        QT_TRANSLATE_NOOP3("cmd", "explodetext", "explode text - command"),
        QT_TRANSLATE_NOOP3("cmd", "xt",          "explode text - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "txtxp",       "explode text - alias"),
            QT_TRANSLATE_NOOP3("cmd", "txtexp",      "explode text - alias"),
            QT_TRANSLATE_NOOP3("cmd", "modexpltext", "explode text - alias")
        }
    },
    {
        RS2::ActionBlocksExplode,
        QT_TRANSLATE_NOOP3("cmd", "explode", "explode - command"),
        QT_TRANSLATE_NOOP3("cmd", "xp",      "explode - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "modexplode", "explode - alias")
        }
    },
    {
        RS2::ActionModifyDelete,
        QT_TRANSLATE_NOOP3("cmd", "erase", "delete erase - command"),
        QT_TRANSLATE_NOOP3("cmd", "er",    "delete erase - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "e",         "delete erase - alias"),
            QT_TRANSLATE_NOOP3("cmd", "del",       "delete erase - alias"),
            QT_TRANSLATE_NOOP3("cmd", "delete",    "delete erase - alias"),
            QT_TRANSLATE_NOOP3("cmd", "moddelete", "delete erase - alias")
        }
    },
    {
        RS2::ActionModifyAlign,
        QT_TRANSLATE_NOOP3("cmd", "align", "align - command"),
        QT_TRANSLATE_NOOP3("cmd", "al",    "align - keycode"),
        {}
    },
    {
        RS2::ActionModifyAlignOne,
        QT_TRANSLATE_NOOP3("cmd", "align1", "align 1 point - command"),
        QT_TRANSLATE_NOOP3("cmd", "a1",     "align 1 point - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "al1", "align 1 point - alias")
        }
    },
    {
        RS2::ActionModifyAlignRef,
        QT_TRANSLATE_NOOP3("cmd", "alignref", "align reference - command"),
        QT_TRANSLATE_NOOP3("cmd", "af",       "align reference - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "alr", "align reference - alias")
        }
    },
    {
        RS2::ActionModifyLineJoin,
        QT_TRANSLATE_NOOP3("cmd", "join", "line join - command"),
        QT_TRANSLATE_NOOP3("cmd", "lj",   "line join - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "j",        "line join - alias"),
            QT_TRANSLATE_NOOP3("cmd", "linejoin", "line join - alias")
        }
    },
    {
        RS2::ActionModifyBreakDivide,
        QT_TRANSLATE_NOOP3("cmd", "break", "break - command"),
        QT_TRANSLATE_NOOP3("cmd", "bd",    "break - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "br",          "break - alias"),
            QT_TRANSLATE_NOOP3("cmd", "breakdivide", "break - alias")
        }
    },
    {
        RS2::ActionModifyLineGap,
        QT_TRANSLATE_NOOP3("cmd", "linegap", "line gap - command"),
        QT_TRANSLATE_NOOP3("cmd", "gl",      "line gap - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "gapline", "line gap - alias")
        }
    },

    /* =========================================================================
     * INFO COMMANDS (Unified i* family)
     * ========================================================================= */
    {
        RS2::ActionInfoDistPoint2Point,
        QT_TRANSLATE_NOOP3("cmd", "distance", "distance point to point - command"),
        QT_TRANSLATE_NOOP3("cmd", "id",       "distance point to point - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "dpp",          "distance point to point - alias"),
            QT_TRANSLATE_NOOP3("cmd", "dist",         "distance point to point - alias"),
            QT_TRANSLATE_NOOP3("cmd", "infodistance", "distance point to point - alias")
        }
    },
    {
        RS2::ActionInfoDistEntity2Point,
        QT_TRANSLATE_NOOP3("cmd", "distep", "distance entity to point - command"),
        QT_TRANSLATE_NOOP3("cmd", "i2",     "distance entity to point - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "dep",        "distance entity to point - alias"),
            QT_TRANSLATE_NOOP3("cmd", "infodistep", "distance entity to point - alias")
        }
    },
    {
        RS2::ActionInfoAngle,
        QT_TRANSLATE_NOOP3("cmd", "infoangle", "measure angle - command"),
        QT_TRANSLATE_NOOP3("cmd", "ia",        "measure angle - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ang",     "measure angle - alias"),
            QT_TRANSLATE_NOOP3("cmd", "measang", "measure angle - alias")
        }
    },
    {
        RS2::ActionInfoArea,
        QT_TRANSLATE_NOOP3("cmd", "area", "measure area - command"),
        QT_TRANSLATE_NOOP3("cmd", "ir",   "measure area - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "aa",       "measure area - alias"),
            QT_TRANSLATE_NOOP3("cmd", "infoarea", "measure area - alias")
        }
    },

    /* =========================================================================
     * ANNOTATION & HATCH
     * ========================================================================= */
    {
        RS2::ActionDrawMText,
        QT_TRANSLATE_NOOP3("cmd", "mtext", "draw mtext - command"),
        QT_TRANSLATE_NOOP3("cmd", "mt",    "draw mtext - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "mtxt", "draw mtext - alias")
        }
    },
    {
        RS2::ActionDrawText,
        QT_TRANSLATE_NOOP3("cmd", "text", "draw text - command"),
        QT_TRANSLATE_NOOP3("cmd", "tx",   "draw text - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "txt", "draw text - alias")
        }
    },
    {
        RS2::ActionDrawHatch,
        QT_TRANSLATE_NOOP3("cmd", "hatch", "draw hatch - command"),
        QT_TRANSLATE_NOOP3("cmd", "ha",    "draw hatch - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "h", "draw hatch - alias")
        }
    },
    {
        RS2::ActionDrawPoint,
        QT_TRANSLATE_NOOP3("cmd", "point", "draw point - command"),
        QT_TRANSLATE_NOOP3("cmd", "po",    "draw point - keycode"),
        {}
    },

    /* =========================================================================
     * SNAPPING & RESTRICTIONS
     * ========================================================================= */
    {
        RS2::ActionSnapFree,
        QT_TRANSLATE_NOOP3("cmd", "snapfree", "snap free - command"),
        QT_TRANSLATE_NOOP3("cmd", "so",       "snap free - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "os", "snap free - alias")
        }
    },
    {
        RS2::ActionSnapCenter,
        QT_TRANSLATE_NOOP3("cmd", "snapcenter", "snap center - command"),
        QT_TRANSLATE_NOOP3("cmd", "sc",         "snap center - keycode"),
        {}
    },
    {
        RS2::ActionSnapDist,
        QT_TRANSLATE_NOOP3("cmd", "snapdist", "snap distance - command"),
        QT_TRANSLATE_NOOP3("cmd", "sd",       "snap distance - keycode"),
        {}
    },
    {
        RS2::ActionSnapEndpoint,
        QT_TRANSLATE_NOOP3("cmd", "snapend", "snap endpoint - command"),
        QT_TRANSLATE_NOOP3("cmd", "se",      "snap endpoint - keycode"),
        {}
    },
    {
        RS2::ActionSnapGrid,
        QT_TRANSLATE_NOOP3("cmd", "snapgrid", "snap grid - command"),
        QT_TRANSLATE_NOOP3("cmd", "sg",       "snap grid - keycode"),
        {}
    },
    {
        RS2::ActionSnapIntersection,
        QT_TRANSLATE_NOOP3("cmd", "snapintersection", "snap intersection - command"),
        QT_TRANSLATE_NOOP3("cmd", "si",               "snap intersection - keycode"),
        {}
    },
    {
        RS2::ActionSnapMiddle,
        QT_TRANSLATE_NOOP3("cmd", "snapmiddle", "snap middle - command"),
        QT_TRANSLATE_NOOP3("cmd", "sm",         "snap middle - keycode"),
        {}
    },
    {
        RS2::ActionSnapOnEntity,
        QT_TRANSLATE_NOOP3("cmd", "snaponentity", "snap on entity - command"),
        QT_TRANSLATE_NOOP3("cmd", "sn",           "snap on entity - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "np", "snap on entity - alias")
        }
    },
    {
        RS2::ActionSnapMiddleManual,
        QT_TRANSLATE_NOOP3("cmd", "snapmiddlemanual", "snap middle manual - command"),
        QT_TRANSLATE_NOOP3("cmd", "mm",               "snap middle manual - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "smm",        "snap middle manual - alias"),
            QT_TRANSLATE_NOOP3("cmd", "snapmanual", "snap middle manual - alias")
        }
    },
    {
        RS2::ActionSetRelativeZero,
        QT_TRANSLATE_NOOP3("cmd", "setrelativezero", "set relative zero - command"),
        QT_TRANSLATE_NOOP3("cmd", "rz",              "set relative zero - keycode"),
        {}
    },
    {
        RS2::ActionRestrictNothing,
        QT_TRANSLATE_NOOP3("cmd", "restrictnothing", "restrict nothing - command"),
        QT_TRANSLATE_NOOP3("cmd", "rn",              "restrict nothing - keycode"),
        {}
    },
    {
        RS2::ActionRestrictOrthogonal,
        QT_TRANSLATE_NOOP3("cmd", "restrictorthogonal", "restrict orthogonal - command"),
        QT_TRANSLATE_NOOP3("cmd", "rr",                 "restrict orthogonal - keycode"),
        {}
    },
    {
        RS2::ActionRestrictHorizontal,
        QT_TRANSLATE_NOOP3("cmd", "restricthorizontal", "restrict horizontal - command"),
        QT_TRANSLATE_NOOP3("cmd", "rh",                 "restrict horizontal - keycode"),
        {}
    },
    {
        RS2::ActionRestrictVertical,
        QT_TRANSLATE_NOOP3("cmd", "restrictvertical", "restrict vertical - command"),
        QT_TRANSLATE_NOOP3("cmd", "rv",               "restrict vertical - keycode"),
        {}
    },

    /* =========================================================================
     * EDIT & OPTIONS
     * ========================================================================= */
    {
        RS2::ActionEditKillAllActions,
        QT_TRANSLATE_NOOP3("cmd", "kill", "kill actions - command"),
        QT_TRANSLATE_NOOP3("cmd", "ki",   "kill actions - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "k",   "kill actions - alias"),
            QT_TRANSLATE_NOOP3("cmd", "esc", "kill actions - alias")
        }
    },
    {
        RS2::ActionEditUndo,
        QT_TRANSLATE_NOOP3("cmd", "undo", "undo cycle - command"),
        QT_TRANSLATE_NOOP3("cmd", "un",   "undo cycle - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "u", "undo cycle - alias")
        }
    },
    {
        RS2::ActionEditRedo,
        QT_TRANSLATE_NOOP3("cmd", "redo", "redo cycle - command"),
        QT_TRANSLATE_NOOP3("cmd", "rd",   "redo cycle - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "r", "redo cycle - alias")
        }
    },
    {
        RS2::ActionOptionsDrawing,
        QT_TRANSLATE_NOOP3("cmd", "drawpref", "drawing preferences - command"),
        QT_TRANSLATE_NOOP3("cmd", "dp",       "drawing preferences - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "drawingoptions",     "drawing preferences - alias"),
            QT_TRANSLATE_NOOP3("cmd", "drawingpreferences", "drawing preferences - alias")
        }
    },

    /* =========================================================================
     * VIEW & ZOOM COMMANDS
     * ========================================================================= */
    {
        RS2::ActionZoomRedraw,
        QT_TRANSLATE_NOOP3("cmd", "regen", "zoom redraw - command"),
        QT_TRANSLATE_NOOP3("cmd", "rg",    "zoom redraw - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "zr",     "zoom redraw - alias"),
            QT_TRANSLATE_NOOP3("cmd", "redraw", "zoom redraw - alias")
        }
    },
    {
        RS2::ActionZoomIn,
        QT_TRANSLATE_NOOP3("cmd", "zoomin", "zoom in - command"),
        QT_TRANSLATE_NOOP3("cmd", "zi",     "zoom in - keycode"),
        {}
    },
    {
        RS2::ActionZoomOut,
        QT_TRANSLATE_NOOP3("cmd", "zoomout", "zoom out - command"),
        QT_TRANSLATE_NOOP3("cmd", "zo",      "zoom out - keycode"),
        {}
    },
    {
        RS2::ActionZoomAuto,
        QT_TRANSLATE_NOOP3("cmd", "zoomauto", "zoom auto - command"),
        QT_TRANSLATE_NOOP3("cmd", "za",       "zoom auto - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ze",          "zoom auto - alias"),
            QT_TRANSLATE_NOOP3("cmd", "zoomextents", "zoom auto - alias")
        }
    },
    {
        RS2::ActionZoomPrevious,
        QT_TRANSLATE_NOOP3("cmd", "zoomprevious", "zoom previous - command"),
        QT_TRANSLATE_NOOP3("cmd", "zv",           "zoom previous - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "zoomprev", "zoom previous - alias")
        }
    },
    {
        RS2::ActionZoomWindow,
        QT_TRANSLATE_NOOP3("cmd", "zoomwindow", "zoom window - command"),
        QT_TRANSLATE_NOOP3("cmd", "zw",         "zoom window - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "zoomwin", "zoom window - alias")
        }
    },
    {
        RS2::ActionZoomPan,
        QT_TRANSLATE_NOOP3("cmd", "pan", "zoom pan - command"),
        QT_TRANSLATE_NOOP3("cmd", "zp",  "zoom pan - keycode"),
        {
            QT_TRANSLATE_NOOP3("cmd", "p",       "zoom pan - alias"),
            QT_TRANSLATE_NOOP3("cmd", "zoompan", "zoom pan - alias")
        }
    }
};

/* =============================================================================
 * IN-PROMPT KEYWORDS (SUB-COMMANDS)
 * ============================================================================= */
const LC_KeywordItem g_keywordList[] = {

    /* --- General Parameters & Geometry --- */
    {
        QT_TRANSLATE_NOOP3("cmd", "angle", "angle - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "a",      "angle - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "an",     "angle - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "ang",    "angle - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "angle1", "angle - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "angle2", "angle - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "center", "center - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "c",   "center - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "ce",  "center - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "cen", "center - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "chordlen", "chordlen - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "c",  "chordlen - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "cl", "chordlen - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "ch", "chordlen - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "close", "close - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "c", "close - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "columns", "columns - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "c",    "columns - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "co",   "columns - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "col",  "columns - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "cols", "columns - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "columnspacing", "columnspacing - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "s",          "columnspacing - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "cs",         "columnspacing - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "csp",        "columnspacing - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "colspacing", "columnspacing - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "equation", "equation - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "e",   "equation - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "eq",  "equation - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "eqn", "equation - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "factor", "factor - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "f",    "factor - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "fact", "factor - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "length", "length - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "l",   "length - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "len", "length - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "length1", "length1 - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "1",    "length1 - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "l1",   "length1 - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "d1",   "length1 - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "len1", "length1 - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "length2", "length2 - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "2",    "length2 - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "l2",   "length2 - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "d2",   "length2 - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "len2", "length2 - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "number", "number - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "n",   "number - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "num", "number - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "radius", "radius - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "r",   "radius - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "ra",  "radius - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "rad", "radius - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "rows", "rows - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "r",   "rows - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "row", "rows - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "rowspacing", "rowspacing - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "s",   "rowspacing - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "rs",  "rowspacing - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "rsp", "rowspacing - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "through", "through - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "t", "through - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "trim", "trim - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "t",  "trim - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "tr", "trim - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "help", "help - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "?", "help - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "h", "help - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "reversed", "reversed - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "r",   "reversed - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "rev", "reversed - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "undo", "undo - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "u", "undo - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "redo", "redo - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "r", "redo - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "back", "back - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "b", "back - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "dpi", "dpi - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "d", "dpi - keyword alias")
        }
    },

    /* --- Point & Relative Input --- */
    {
        QT_TRANSLATE_NOOP3("cmd", "x", "x - keyword"),
        {}
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "y", "y - keyword"),
        {}
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "p", "p - keyword"),
        {}
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "anglerel", "anglerel - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "a",  "anglerel - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "ar", "anglerel - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "start", "start - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "s",  "start - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "st", "start - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "offset", "offset - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "o",   "offset - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "off", "offset - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "linesnap", "linesnap - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "l",  "linesnap - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "ls", "linesnap - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "ticksnap", "ticksnap - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "t",  "ticksnap - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "ts", "ticksnap - keyword alias")
        }
    },

    /* --- Rectangle & Polygon Options --- */
    {
        QT_TRANSLATE_NOOP3("cmd", "width", "width - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "w", "width - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "height", "height - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "h", "height - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "pos", "pos - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "p", "pos - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "size", "size - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "s",  "size - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "sz", "size - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "bevels", "bevels - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "b",       "bevels - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "c",       "bevels - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "bev",     "bevels - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "chamfer", "bevels - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "nopoly", "nopoly - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "n",  "nopoly - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "np", "nopoly - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "usepoly", "usepoly - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "p",    "usepoly - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "u",    "usepoly - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "up",   "usepoly - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "poly", "usepoly - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "corners", "corners - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "c",  "corners - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "cr", "corners - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "round", "round - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "f",      "round - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "r",      "round - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "rnd",    "round - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "fillet", "round - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "snap1", "snap1 - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "1",  "snap1 - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "s1", "snap1 - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "snap2", "snap2 - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "2",  "snap2 - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "s2", "snap2 - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "topl", "topl - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "tl", "topl - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "top", "top - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "t", "top - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "topr", "topr - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "tr", "topr - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "left", "left - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "l", "left - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "middle", "middle - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "m",   "middle - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "mid", "middle - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "right", "right - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "r", "right - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "bottoml", "bottoml - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "bl", "bottoml - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "bottom", "bottom - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "b", "bottom - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "bottomr", "bottomr - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "br", "bottomr - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "snapcorner", "snapcorner - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "sc", "snapcorner - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "snapshift", "snapshift - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ss", "snapshift - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "sizein", "sizein - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "i",  "sizein - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "in", "sizein - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "si", "sizein - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "sizeout", "sizeout - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "o",   "sizeout - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "so",  "sizeout - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "out", "sizeout - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "hor", "hor - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "h", "hor - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "vert", "vert - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "v", "vert - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "corner", "corner - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "c", "corner - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "mid-vert", "mid-vert - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "mv", "mid-vert - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "mid-hor", "mid-hor - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "mh", "mid-hor - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "quad", "quad - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "q", "quad - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "noquad", "noquad - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "nq", "noquad - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "angle_inner", "angle_inner - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ai", "angle_inner - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "ia", "angle_inner - keyword alias")
        }
    },

    /* --- Line of Points Options --- */
    {
        QT_TRANSLATE_NOOP3("cmd", "edges", "edges - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "e",  "edges - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "ed", "edges - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "edge-none", "edge-none - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "en", "edge-none - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "edge-both", "edge-both - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "eb", "edge-both - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "edge-start", "edge-start - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "es", "edge-start - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "edge-end", "edge-end - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ee", "edge-end - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "end", "end - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "e", "end - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "both", "both - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "b", "both - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "none", "none - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "n", "none - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "fit", "fit - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "f", "fit - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "nofit", "nofit - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "nf", "nofit - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "dist_fixed", "dist_fixed - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "df", "dist_fixed - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "dist_flex", "dist_flex - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "fl", "dist_flex - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "dx", "dist_flex - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "distance", "distance - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "d",    "distance - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "dist", "distance - keyword alias")
        }
    },

    /* --- Star Options --- */
    {
        QT_TRANSLATE_NOOP3("cmd", "sym", "sym - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "s", "sym - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "nosym", "nosym - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "ns", "nosym - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "snap", "snap - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "s", "snap - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "m", "snap - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "e", "snap - keyword alias")
        }
    },

    /* --- Radiant (Ray) Options --- */
    {
        QT_TRANSLATE_NOOP3("cmd", "radiant", "radiant - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "r",   "radiant - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "rad", "radiant - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "active", "active - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "a",   "active - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "act", "active - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "lentype", "lentype - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "lt", "lentype - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "fixed", "fixed - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "f",   "fixed - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "fix", "fixed - keyword alias")
        }
    },

    /* --- Print Preview Options --- */
    {
        QT_TRANSLATE_NOOP3("cmd", "bw", "bw - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "b",          "bw - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "blackwhite", "bw - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "color", "color - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "c",   "color - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "col", "color - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "paperoffset", "paperoffset - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "po",    "paperoffset - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "paper", "paperoffset - keyword alias")
        }
    },
    {
        QT_TRANSLATE_NOOP3("cmd", "graphoffset", "graphoffset - keyword"),
        {
            QT_TRANSLATE_NOOP3("cmd", "go",    "graphoffset - keyword alias"),
            QT_TRANSLATE_NOOP3("cmd", "graph", "graphoffset - keyword alias")
        }
    }
};

#endif
