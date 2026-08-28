
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

/*******************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2026 LibreCAD.org
 ******************************************************************************/

#ifndef LC_SETTINGS_PAGE_DRAWING_VARIABLES_H
#define LC_SETTINGS_PAGE_DRAWING_VARIABLES_H

#include "lc_settings_page_base.h"

namespace Ui { class LC_SettingsPageDrawingVariables; }
class RS_Graphic;

class LC_SettingsPageDrawingVariables : public LC_SettingsPageBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageDrawingVariables(RS_Graphic* graphic, QObject* parent = nullptr);
    ~LC_SettingsPageDrawingVariables() override;

    void setupUi() override;
    void loadSettings() override;
private:
    std::unique_ptr<Ui::LC_SettingsPageDrawingVariables> ui;
    RS_Graphic* m_graphic;
};

#endif // LC_SETTINGS_PAGE_DRAWING_VARIABLES_H
