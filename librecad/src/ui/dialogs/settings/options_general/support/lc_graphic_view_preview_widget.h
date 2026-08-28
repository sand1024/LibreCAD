
/*******************************************************************************
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

#ifndef LC_GraphicViewPreviewWidget_H
#define LC_GraphicViewPreviewWidget_H

#include "lc_cursoroverlayinfo.h"
#include "lc_settings_page_interface.h"
#include "rs_entity.h"

class LC_VisualSnapManager;
struct LC_InfoCursorData;
class LC_VisualSnapSolution;
struct LC_OverlayBoxOptions;
struct LC_VisualSnapOptions;
class LC_Highlight;
class LC_DimStylePreviewGraphicView;
class RS_Preview;

class LC_GraphicViewPreviewWidget : public QWidget, public LC_LivePreview {
public:
    LC_GraphicViewPreviewWidget(QWidget* parent);
    ~LC_GraphicViewPreviewWidget() override;
    void updateLivePreview() override;
    void updatePreviewForContentCategory(const QString& tag) override;
    void cleanupPreviewForContentCategory(const QString& tag) override;
    void updateGrid(int forcedType);
private:
    void previewSnapIndicator(bool showAngleSnap) const;
    void preparePreviewPreview();
    void previewSnapVisual();
    void addDocumentEntityToVisualSnap(RS_Entity* entity1);
    void addEntityPointToVisualSnap(RS_Entity* entity1);
    void prepareVisualSnapSolution();
    void previewInfoCursor();
    void prepareInfoCursor(LC_InfoCursorOverlayPrefs* prefs);
    void previewRelativePointInputAssistant();
    LC_DimStylePreviewGraphicView* m_previewView = nullptr;
    std::unique_ptr<RS_Preview> m_preview;
    std::unique_ptr<LC_Highlight> m_highlight;
    std::unique_ptr<LC_VisualSnapOptions> m_visualSnapOptions;
    std::unique_ptr<LC_OverlayBoxOptions> m_overlayBoxOptions;
    std::unique_ptr<LC_VisualSnapSolution> m_solution;
    std::unique_ptr<LC_InfoCursorData> m_infoCursorOverlayData;
    QString m_currentPageTag;
};

#endif
