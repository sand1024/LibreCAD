/*
 * ********************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2025 LibreCAD.org
 * Copyright (C) 2025 sand1024
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

#include "lc_ucs_mark.h"

#include "lc_settings_appearance.h"
#include "lc_settings_colors.h"
#include "rs_math.h"
#include "rs_painter.h"
#include "rs_pen.h"
#include "rs_settings.h"

void LC_UCSMarkOptions::loadSettings() {
    {
        using namespace CFG_Appearance;
        showUcsZeroMarker = o_ShowUCSZeroMarker;
        showWcsZeroMarker = o_ShowWCSZeroMarker;
        csZeroMarkerSize = o_ZeroMarkerSize;
        csZeroMarkerFontSize = o_ZeroMarkerFontSize;
        csZeroMarkerfontName = o_ZeroMarkerFontName;
        csZeroMarkerFont = QFont(csZeroMarkerfontName, csZeroMarkerFontSize);
        overlayScreenLineWidth = o_OverlaysScreenLineWidth;
        if (overlayScreenLineWidth == 1 ) {
            overlayScreenLineWidth = 0;
        }
    }
    {
        using namespace CFG_Colors;
        colorXAxisExtension = RS_Color(o_XAxisColor);
        colorYAxisExtension = RS_Color(o_YAxisColor);
        colorAngleMark = RS_Color(o_PreviewReferencesColor);
    }
}

LC_OverlayUCSMark::LC_OverlayUCSMark(const RS_Vector& uiOrigin, const double xAxisAngle, const bool forWcs, LC_UCSMarkOptions* options) :
    m_uiOrigin{uiOrigin}, m_xAxisAngle{xAxisAngle}, m_forWcs{forWcs}, m_options{options} {
}

LC_OverlayUCSMark::LC_OverlayUCSMark(LC_UCSMarkOptions *options):m_uiOrigin({0,0,0}), m_xAxisAngle(0), m_options(options) {}

void LC_OverlayUCSMark::update(const RS_Vector& uiPos, const double xAngle, const bool wcs){
    m_uiOrigin = uiPos;
    m_xAxisAngle = xAngle;
    m_forWcs = wcs;
}

void LC_OverlayUCSMark::draw(RS_Painter *painter) {
    const int zr = m_options->csZeroMarkerSize;
    RS_Vector uiXAxisEnd = m_uiOrigin.relative(zr, m_xAxisAngle);

    double yAxisAngle = m_xAxisAngle-M_PI_2; // as we're in UI coordinate space
    RS_Vector uiYAxisEnd = m_uiOrigin.relative(zr,yAxisAngle);

    RS_Pen penXAxis (m_options->colorXAxisExtension, RS2::Width00, RS2::SolidLine);
    penXAxis.setScreenWidth(m_options->overlayScreenLineWidth);

    RS_Pen penYAxis (m_options->colorYAxisExtension, RS2::Width00, RS2::SolidLine);
    penYAxis.setScreenWidth(m_options->overlayScreenLineWidth);

    double anchorFactor = 0.2;
    double anchorAngle = RS_Math::deg2rad(70);
    double anchorSize = zr * anchorFactor;

    double resultingAchorAngle = M_PI_2 + anchorAngle;

    painter->setFont(m_options->csZeroMarkerFont);

    painter->setPen(penXAxis);
    // axis
    painter->drawLineUISimple(m_uiOrigin, uiXAxisEnd);
    // anchor
    painter->drawLineUISimple(uiXAxisEnd, uiXAxisEnd.relative(anchorSize, m_xAxisAngle+resultingAchorAngle));
    painter->drawLineUISimple(uiXAxisEnd, uiXAxisEnd.relative(anchorSize, m_xAxisAngle-resultingAchorAngle));

    const char* xString = m_forWcs ? "wX" : "X";

    const QSize &xSize = QFontMetrics(m_options->csZeroMarkerFont).size(Qt::TextSingleLine, xString);

    auto offset = RS_Vector(20,10);

    RS_Vector xTextPosition = uiXAxisEnd.relative(offset.x, yAxisAngle);

    auto xRect = QRect(QPoint(xTextPosition.x, xTextPosition.y), xSize);
    QRect xBoundingRect;
    painter->drawText(xRect,  Qt::AlignTop | Qt::AlignRight | Qt::TextDontClip, xString, &xBoundingRect);

    painter->setPen(penYAxis);
    // axis
    painter->drawLineUISimple(m_uiOrigin, uiYAxisEnd);
    // anchor
    painter->drawLineUISimple(uiYAxisEnd, uiYAxisEnd.relative(anchorSize, yAxisAngle+resultingAchorAngle));
    painter->drawLineUISimple(uiYAxisEnd, uiYAxisEnd.relative(anchorSize, yAxisAngle-resultingAchorAngle));

    const char* yString = m_forWcs ? "wY" : "Y";
    const QSize &ySize = QFontMetrics(m_options->csZeroMarkerFont).size(Qt::TextSingleLine, yString);

    RS_Vector yTextPosition = uiYAxisEnd.relative(offset.x, m_xAxisAngle);

    auto yRect = QRect(QPoint(yTextPosition.x, yTextPosition.y), ySize);
    QRect yBoundingRect;
    painter->drawText(yRect, Qt::AlignTop | Qt::AlignRight | Qt::TextDontClip, yString, &yBoundingRect);

    // square
    double angleFactor = 0.2;
    double angleLen = zr*angleFactor;

    RS_Vector angleX0 = m_uiOrigin.relative(angleLen, m_xAxisAngle);
    RS_Vector angleX1 = angleX0.relative(angleLen, yAxisAngle);
    RS_Vector anchorY0 = m_uiOrigin.relative(angleLen, yAxisAngle);

    RS_Pen anglePen (m_options->colorAngleMark, RS2::Width00, RS2::SolidLine);
    anglePen.setScreenWidth(m_options->overlayScreenLineWidth);

    painter->setPen(anglePen);

    painter->drawLineUISimple(anchorY0, angleX1);
    painter->drawLineUISimple(angleX0, angleX1);

    painter->drawGridPoint(m_uiOrigin);
}
