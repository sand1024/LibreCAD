
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

#ifndef LC_POLYGONSELECTION_H
#define LC_POLYGONSELECTION_H

#include <QPolygonF>
#include "rs.h"

class LC_LassoRegion;
class LC_GraphicViewport;
class RS_Document;

namespace LC_PolygonSelection {
    void select(RS_Document* document, LC_GraphicViewport* viewport,
                       const LC_LassoRegion& region, RS2::EntityType typeToSelect = RS2::EntityUnknown,
                       bool crossing = true, bool selecting = true);

    void select(RS_Document* document, LC_GraphicViewport* viewport,
                                 const LC_LassoRegion& region, const QList<RS2::EntityType> &typesToSelect,
                                 const bool crossing, const bool selecting);
};

#endif
