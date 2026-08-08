
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

#ifndef LC_SETTINGS_PAGE_DRAWING_DIMENSIONS_H
#define LC_SETTINGS_PAGE_DRAWING_DIMENSIONS_H

#include "lc_settings_page_base.h"

namespace Ui { class LC_SettingsPageDrawingDimensions; }
class RS_Graphic;
class LC_DimStyleTreeModel;
class LC_DimStyleItem;
class LC_DimStyle;
class LC_DimStylePreviewGraphicView;

class LC_SettingsPageDrawingDimensions : public LC_SettingsPageBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageDrawingDimensions(RS_Graphic* graphic, QObject* parent = nullptr);
    ~LC_SettingsPageDrawingDimensions() override;

    void loadSettings() override;
    bool saveSettings() override;
    void onPageResized() override;
    void onDialogShown() override;
    bool isModified() const override;
    void onAboutToShow() override;
    void onAboutToHide() override;
private slots:
    void onCurrentStyleChanged(const QModelIndex& current, const QModelIndex& previous);
    void onStyleNew();
    void onStyleEdit();
    void onStyleRename();
    void onStyleRemove();
    void onStyleSetDefault();
    void onStyleExport();
    void onStyleImport();
    void onStyleDoubleClick(const QModelIndex&);
    void onContextMenu(const QPoint& pos);
protected:
    void setupUi() override;
    void updateActiveStyleLabel(LC_DimStyleTreeModel* model);
    void collectStylesUsage(QMap<QString, int>& map) const;
    void setupBehavior() override;
private:
    void updatePreview(LC_DimStyle* style);
    void refreshActionButtons(const LC_DimStyleItem* item);
    QString getUniqueName(const QString& title, const QString& label, const QString& base);



    std::unique_ptr<Ui::LC_SettingsPageDrawingDimensions> ui;
    RS_Graphic* m_graphic;
    LC_DimStylePreviewGraphicView* m_previewView = nullptr;
    bool m_manualModified = false;
    bool m_pageWasVisible = false;
};

#endif
