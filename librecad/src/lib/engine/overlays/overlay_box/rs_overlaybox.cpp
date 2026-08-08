/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/
#include "rs_overlaybox.h"

#include <math.h>

#include "lc_settings_appearance.h"
#include "lc_settings_colors.h"
#include "rs_painter.h"
#include "rs_settings.h"

void LC_OverlayBoxOptions::loadSettings() {
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
        if (overlayScreenLineWidth ==1 ) {
            overlayScreenLineWidth = 0;
        }
    }
}

RS_OverlayBox::RS_OverlayBox(const RS_Vector &corner1, const RS_Vector &corner2, LC_OverlayBoxOptions *options)
   :m_corner1(corner1), m_corner2(corner2), m_options(options) {}

void RS_OverlayBox::draw(RS_Painter* painter) {
    double v1x = NAN;
    double v1y;
    double v2x;
    double v2y;

    painter->toGui(m_corner1, v1x, v1y);
    painter->toGui(m_corner2, v2x, v2y);

    QRectF selectRect(v1x,v1y,v2x - v1x,v2y - v1y);

    if (v1x > v2x) {
        RS_Pen p(m_options->colorLineInverted, RS2::Width00, m_options->invertedLineType);
        p.setScreenWidth(m_options->overlayScreenLineWidth);
        painter->setPen(p);
        const RS_Color &fillColor = m_options->colorBoxFillInverted;
        painter->fillRect(selectRect, fillColor);
    }
    else {
        RS_Pen p(m_options->colorBoxLine, RS2::Width00, m_options->lineType);
        p.setScreenWidth(m_options->overlayScreenLineWidth);
        painter->setPen(p);
        const RS_Color &fillColor = m_options->colorBoxFill;
        painter->fillRect(selectRect, fillColor);
    }

    painter->drawRectUI(v1x, v1y, v2x, v2y);
}
