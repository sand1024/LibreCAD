/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

 Copyright (C) 2024 LibreCAD.org
 Copyright (C) 2024 sand1024

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
 ******************************************************************************/

#ifndef LC_CROSSHAIR_H
#define LC_CROSSHAIR_H

#include "lc_defaults.h"
#include "lc_overlayentity.h"
#include "lc_settings_colors.h"
#include "lc_settings_snap.h"
#include "rs_pen.h"
#include "rs_vector.h"

struct SnapIndicatorOptions;

class LC_Crosshair:public LC_OverlayDrawable{
public:
    enum IndicatorShape{
        Circle,
        Point,
        Square,
        Gap,
        NoShape
    };

    enum LinesShape{
        Adaptive,
        Crosshair,
        Spiderweb,
        NoLines
    };

    LC_Crosshair(const RS_Vector &coord, SnapIndicatorOptions& options);
    void draw(RS_Painter *painter) override;
    void setLinesPen(const RS_Pen &pen);
    void setPointType(int type);
    void setPointSize(int size);
    void setShapesPen(const RS_Pen &p) {m_shapePen = p;}
protected:
    int m_linesShape;
    int m_indicatorShape;
    RS_Pen m_linesPen;
    RS_Pen m_shapePen;
    int m_pointType;
    int m_shapeSize;
    int m_pointSize;
    RS_Vector m_wcsPos;

    double drawIndicator(RS_Painter *painter, const RS_Vector& uiPos) const;

    void drawCrosshairLines(
        RS_Painter *painter, const RS_Vector &guiCoord, double offset, const RS_Vector &p1, const RS_Vector &p2, const RS_Vector &p3,
        const RS_Vector &p4) const;
};


struct SnapIndicatorOptions {
    bool drawLines = false;
    int lines_Type = 0;
    RS_Pen lines_Pen;

    bool drawShape = false;
    int shape_Type = 0;
    RS_Pen shape_Pen;

    int pointType = LC_DEFAULTS_PDMode;
    int pointSize = LC_DEFAULTS_PDSize;
    int shapeSize = 4;

    void loadSettings() {
        using namespace CFG_Snap;
        const int snapIndicatorLineWidth = o_IndicatorScreenLinesLineWidth;
        drawLines = o_IndicatorDrawLines;
        if (drawLines) {
            lines_Type = o_IndicatorLinesType;
            const auto snapIndicatorLineType = o_IndicatorLinesLineType;
            const QColor snapLinesColor = CFG_Colors::o_SnapIndicatorLines;
            lines_Pen = RS_Pen(RS_Color(snapLinesColor), RS2::Width00, snapIndicatorLineType);
            lines_Pen.setScreenWidth(snapIndicatorLineWidth);
        }
        else {
            lines_Type = LC_Crosshair::NoLines;
        }

        drawShape = o_IndicatorDrawShape;
        if (drawShape) {
            shape_Type = o_IndicatorShapeType;
            const QColor snapColor = CFG_Colors::o_SnapIndicator;
            shape_Pen = RS_Pen(RS_Color(snapColor), RS2::Width00, RS2::SolidLine);
            shape_Pen.setScreenWidth(snapIndicatorLineWidth);
        }
        else {
            shape_Type = LC_Crosshair::NoShape;
        }
        shapeSize = o_IndicatorShapeSize;
    }
};

#endif
