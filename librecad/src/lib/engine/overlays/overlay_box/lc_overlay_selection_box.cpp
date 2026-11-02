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
#include "lc_overlay_selection_box.h"

#include <math.h>

#include "lc_settings_appearance.h"
#include "lc_settings_colors.h"
#include "rs_painter.h"
#include "rs_settings.h"

void LC_SelectionBoxOptions::loadSettings() {
    {
        using namespace CFG_Colors;
        int overlayTransparency = o_OverlayBoxTransparency;;
        colorBoxLine = RS_Color(o_OverlayBoxLineColor);
        auto tmp = RS_Color(o_OverlayBoxFillColor);
        const RS_Color fillColor(tmp.red(), tmp.green(), tmp.blue(), overlayTransparency);
        colorBoxFill = fillColor;
        colorLineInverted = RS_Color(o_OverlayBoxLineInvertedColor);
        tmp = RS_Color(o_OverlayBoxFillInvertedColor);
        RS_Color fillColorInverted(tmp.red(), tmp.green(), tmp.blue(), overlayTransparency);
        colorBoxFillInverted = fillColorInverted;
    }

    {
        using namespace CFG_Appearance;
        lineType = o_SelectionOverlayLineType;
        invertedLineType = o_SelectionOverlayInvertedLineType;
        overlayScreenLineWidth = o_OverlaysScreenLineWidth;
        if (overlayScreenLineWidth == 1) {
            overlayScreenLineWidth = 0;
        }
    }
}

LC_OverlaySelectionBox::LC_OverlaySelectionBox(const RS_Vector& corner1, const RS_Vector& corner2, LC_SelectionBoxOptions* options, bool crossing, bool fence)
    : m_corner1(corner1), m_corner2(corner2), m_options(options), m_crossing(crossing), m_fence(fence) {}

void LC_OverlaySelectionBox::draw(RS_Painter* painter) {
    double v1x = NAN;
    double v1y;
    double v2x;
    double v2y;

    painter->toGui(m_corner1, v1x, v1y);
    painter->toGui(m_corner2, v2x, v2y);

    QRectF selectRect(v1x, v1y, v2x - v1x, v2y - v1y);

    if (m_fence) {
        RS_Pen p(m_options->colorLineInverted, RS2::Width00, m_options->invertedLineType);
        p.setScreenWidth(m_options->overlayScreenLineWidth);
        painter->setPen(p, RS_Painter::DashSpace::Screen);
        // Zero fill for Fence mode
    }
    else if (m_crossing) {
        RS_Pen p(m_options->colorLineInverted, RS2::Width00, m_options->invertedLineType);
        p.setScreenWidth(m_options->overlayScreenLineWidth);
        painter->setPen(p, RS_Painter::DashSpace::Screen);
        const RS_Color& fillColor = m_options->colorBoxFillInverted;
        painter->fillRect(selectRect, fillColor);
    }
    else {
        RS_Pen p(m_options->colorBoxLine, RS2::Width00, m_options->lineType);
        p.setScreenWidth(m_options->overlayScreenLineWidth);
        painter->setPen(p, RS_Painter::DashSpace::Screen);
        const RS_Color& fillColor = m_options->colorBoxFill;
        painter->fillRect(selectRect, fillColor);
    }

    painter->drawRectUI(v1x, v1y, v2x, v2y);
}
