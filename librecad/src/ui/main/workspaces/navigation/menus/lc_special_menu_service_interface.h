
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

#ifndef LC_SPECIALMENUSERVICEINTERFACE_H
#define LC_SPECIALMENUSERVICEINTERFACE_H

#include <QMenu>
#include <QToolBar>

class LC_SpecialMenuServiceInterface {
public:
    virtual ~LC_SpecialMenuServiceInterface() = default;
    virtual bool bindMenu(const QString& specialMenuName, QMenu* menu) = 0;
    virtual bool embedToolbarControl(const QString& widgetToken, QToolBar* targetToolbar) {
        return false;
    }
    virtual QAction* getSpecialAction(const QString& actionToken) const {
        return nullptr;
    }
};

#endif
