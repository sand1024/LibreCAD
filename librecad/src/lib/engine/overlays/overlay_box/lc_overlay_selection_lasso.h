
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

#ifndef LC_OVERLAYSELECTIONLASSO_H
#define LC_OVERLAYSELECTIONLASSO_H
#include <qtransform.h>
#include <vector>

#include "lc_overlayentity.h"

class RS_Vector;
struct LC_SelectionBoxOptions;

class LC_OverlaySelectionLasso: public LC_OverlayDrawable {
public:
    LC_OverlaySelectionLasso(LC_SelectionBoxOptions* m_options);
    void update(const std::vector<RS_Vector>& points, bool crossing, bool fence);
    void draw(RS_Painter* painter) override;
private:
    void updateScreenPolygon(const QTransform& transform, const std::vector<RS_Vector>* worldPoints);
    LC_SelectionBoxOptions* m_options = nullptr;
    const std::vector<RS_Vector>* m_worldPoints = nullptr;
    QPolygonF m_screenPoints;
    bool m_inverting = false;
    bool m_fence = false;
};

#endif
