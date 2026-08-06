
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

#ifndef LC_SETTINGS_PAGE_GRAPHIC_VIEW_SELECTION_H
#define LC_SETTINGS_PAGE_GRAPHIC_VIEW_SELECTION_H

#include "lc_settings_page_base.h"

namespace Ui { class LC_SettingsPageGraphicViewSelection; }

class LC_SettingsPageGraphicViewSelection : public LC_SettingsPageBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageGraphicViewSelection(QObject* parent = nullptr);
    ~LC_SettingsPageGraphicViewSelection() override;

protected:
    void setupUi() override;
    void setupBindings() override;

private:
    std::unique_ptr<Ui::LC_SettingsPageGraphicViewSelection> ui;
};
#endif
