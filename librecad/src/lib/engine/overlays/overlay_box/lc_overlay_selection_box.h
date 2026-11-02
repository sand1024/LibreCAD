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

#ifndef RS_OVERLAYBOX_H
#define RS_OVERLAYBOX_H

#include "lc_overlayentity.h"
#include "rs_color.h"
#include "rs_vector.h"

/**
 * Holds the data that defines a line.
 */

struct LC_SelectionBoxOptions{
    LC_SelectionBoxOptions() = default;
    ~LC_SelectionBoxOptions() = default;
    RS_Color colorBoxLine;
    RS_Color colorBoxFill;
    RS_Color colorLineInverted;
    RS_Color colorBoxFillInverted;
    RS2::LineType lineType = RS2::SolidLine;
    RS2::LineType invertedLineType = RS2::DashLine;
    double overlayScreenLineWidth = 0;
    void loadSettings();
};

/**
 * Class for a line entity.
 *
 * @author R. van Twisk
 */
class LC_OverlaySelectionBox : public LC_OverlayDrawable {
public:
    LC_OverlaySelectionBox(const RS_Vector &corner1, const RS_Vector &corner2,
                          LC_SelectionBoxOptions *options, bool crossing = false, bool isFence = false);
    void draw(RS_Painter* painter) override;
    void setCrossing(const bool crossing) {
        m_crossing = crossing;
    }

    bool isCrossing() const {
        return m_crossing;
    }
protected:
    RS_Vector m_corner1;
    RS_Vector m_corner2;
    bool m_crossing = false;
    bool m_fence = false;
    LC_SelectionBoxOptions* m_options = nullptr;
};
#endif
