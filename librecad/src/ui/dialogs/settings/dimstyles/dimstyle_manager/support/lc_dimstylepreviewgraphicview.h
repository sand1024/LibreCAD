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

#ifndef LC_DIMSTYLEPREVIEWGRAPHICVIEW_H
#define LC_DIMSTYLEPREVIEWGRAPHICVIEW_H

#include "lc_dimstyleitem.h"
#include "lc_graphicviewrenderer.h"
#include "lc_preview_graphic_view.h"

class LC_DimStylePreviewGraphic;
class LC_DimStyle;

class LC_DimStylePreviewGraphicView: public LC_PreviewGraphicView {
public:
    void updateDims();
    void refresh() override;
    void setDimStyle(LC_DimStyle *dimStyle) const;
    void setEntityDimStyle(const LC_DimStyle* dimStyle, bool override, const QString& baseName) const;
    void setEntityPen(const RS_Pen& pen) const;

    void addDimStyle(LC_DimStyle* dimStyle) const;
    void setEntityArrowsFlipMode(bool flipArrow1, bool flipArrow2) const;
    static LC_DimStylePreviewGraphicView* init(QWidget* parent,RS_Graphic* originalGraphic, RS2::EntityType dimensionType);
    static LC_DimStylePreviewGraphicView* init(QWidget* parent,RS_Graphic* originalGraphic, const RS_Dimension* dimension);
    void hideNonZeroLayers() const;
    void hideNonRelevantLayers(RS2::EntityType dimType) const;
protected:
    LC_DimStylePreviewGraphicView(QWidget* parent,LC_ActionContext* actionContext);
    ~LC_DimStylePreviewGraphicView() override = default;
    static LC_DimStylePreviewGraphicView* createAndSetupView(QWidget* parent,
            RS_Graphic* graphic, const RS_Graphic* originalGraphic, bool showInWCS);
    static void copyBlocks(RS_Graphic* originalGraphic, RS_Graphic* graphic);
};
#endif
