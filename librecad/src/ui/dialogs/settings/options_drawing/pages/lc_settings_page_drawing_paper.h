
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

#ifndef LC_SETTINGS_PAGE_DRAWING_PAPER_H
#define LC_SETTINGS_PAGE_DRAWING_PAPER_H

#include <memory>
#include <QGraphicsScene>
#include "lc_settings_page_base.h"

namespace Ui { class LC_SettingsPageDrawingPaper; }
class RS_Graphic;

class LC_SettingsPageDrawingPaper : public LC_SettingsPageBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageDrawingPaper(RS_Graphic* graphic, QObject* parent = nullptr);
    ~LC_SettingsPageDrawingPaper() override;
    void loadSettings() override;
    bool saveSettings() override;
    bool isModified() const override;
    void onAboutToShow() override;
    void onAboutToHide() override;
    void onDialogShown() override;
protected:
    void setupUi() override;
    void setupBehavior() override;
private slots:
    void onFormatChanged(int index);
    void onOrientationChanged();
    void updatePreview();
    void doUpdatePreview(bool calcOrientation);
private:
    std::unique_ptr<Ui::LC_SettingsPageDrawingPaper> ui;
    RS_Graphic* m_graphic;
    QGraphicsScene* m_paperScene;
    bool m_pageWasVisible = false;
};

#endif
