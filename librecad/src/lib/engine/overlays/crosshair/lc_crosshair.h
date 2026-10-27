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
#include "lc_settings_appearance.h"
#include "lc_settings_colors.h"
#include "lc_settings_snap.h"
#include "rs_pen.h"
#include "rs_vector.h"

struct SnapIndicatorOptions;

class LC_Crosshair:public LC_OverlayDrawable{
public:
    enum IndicatorShape{
        NoShape,
        Circle,
        Point,
        Square,
        Gap,
        BySnapType,
    };

    enum LinesShape{
        NoLines,
        Adaptive,
        Crosshair,
        Spiderweb,
        AdaptiveShort
    };

    enum PickBoxLines {
        NoBoxLines = 0,
        ShortLines = 1,
        AsForSnapIndicator = 2
    };

    LC_Crosshair(const RS_Vector &coord, SnapIndicatorOptions& options, RS2::SnapType snapType);
    LC_Crosshair(const RS_Vector& coord, const RS_Pen& linesPen, const RS_Pen& shapePen, int shapeSize, int showLinesMode, int snapIndicatorLinesType,  int linesPercent);
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
    RS2::SnapType m_snapType = RS2::NO_SNAP;
    int m_crossHairLinesPersent = 100;

    double drawShape(RS_Painter *painter, const RS_Vector& uiPos) const;
    void drawSnapModeAdaptiveShape(RS_Painter* painter, const RS_Vector& uiPos) const;
    void drawLines(RS_Painter* painter, RS_Vector uiCoord, double offset, int width, int height, double halfWidth);
    void drawLines(RS_Painter* painter, RS_Vector uiCoord, double offset);

    void drawCrosshairLines(
        RS_Painter *painter, const RS_Vector &guiCoord, double offset, const RS_Vector &p1, const RS_Vector &p2, const RS_Vector &p3,
        const RS_Vector &p4) const;
};


struct SnapIndicatorOptions {
    bool drawLines = false;
    int lines_Type = 0;
    int crossHairLinesPercent = 100;
    RS_Pen lines_Pen;

    bool drawShape = false;
    int shape_Type = 0;
    RS_Pen shape_Pen;
    RS_Pen pickBox_Pen;
    int pickBoxLinesSize = 0;

    int pointType = LC_DEFAULTS_PDMode;
    int pointSize = LC_DEFAULTS_PDSize;
    int shapeSize = 4;

    void loadSettings() {
        using namespace CFG_Snap;
        const double snapIndicatorScreenLineWidth = o_IndicatorScreenLinesLineWidth;
        drawLines = o_IndicatorDrawLines;
        if (drawLines) {
            lines_Type = o_IndicatorLinesType  + 1; // 0 is for NoLines
            const auto snapIndicatorLineType = o_IndicatorLinesLineType;
            const QColor snapLinesColor = CFG_Colors::o_SnapIndicatorLines;
            lines_Pen = RS_Pen(RS_Color(snapLinesColor), RS2::Width00, snapIndicatorLineType);
            lines_Pen.setScreenWidth(snapIndicatorScreenLineWidth);
        }
        else {
            lines_Type = LC_Crosshair::NoLines;
        }

        drawShape = o_IndicatorDrawShape;
        if (drawShape) {
            shape_Type = o_IndicatorShapeType + 1; // in UI, NO Shape is not shown in combobox
            const QColor snapColor = CFG_Colors::o_SnapIndicator;
            shape_Pen = RS_Pen(RS_Color(snapColor), RS2::Width00, RS2::SolidLine);
            shape_Pen.setScreenWidth(snapIndicatorScreenLineWidth);
        }
        else {
            shape_Type = LC_Crosshair::NoShape;
        }
        crossHairLinesPercent = o_IndicatorCrosshairLinesSizePercents;
        shapeSize = o_IndicatorShapeSize;

        const QColor pickBoxColor = CFG_Colors::o_SelectionPickBoxColor;
        pickBox_Pen = RS_Pen(RS_Color(pickBoxColor), RS2::Width00, RS2::SolidLine);
        pickBox_Pen.setScreenWidth(snapIndicatorScreenLineWidth);

        if (!drawLines) {
            lines_Pen = pickBox_Pen; // needed for AdaptiveShort
        }

        pickBoxLinesSize = CFG_Appearance::o_SelectionPickBoxLinesSizeType;
    }
};

#endif
