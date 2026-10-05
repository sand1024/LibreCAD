/*
* **************************************************************************
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
 * *********************************************************************
 *
 */
#ifndef LC_DOCKWIDGET_H
#define LC_DOCKWIDGET_H

#include <QDockWidget>

#include "lc_dock_widget_base.h"

class LC_DockWidget : public LC_DockWidgetBase {
    Q_OBJECT
public:
    LC_DockWidget(QWidget* parent, const QString& title, const QString& verticalTitle, const Qt::WindowFlags& flags = Qt::WindowFlags());
    void paintEvent(QPaintEvent* event);
    void setVisible(bool visible) override;

    void updateTitleOld();
protected:
    void showEvent(QShowEvent* event) override { LC_DockWidgetBase::showEvent(event); m_ownVisible = true; }
    void hideEvent(QHideEvent* event) override { LC_DockWidgetBase::hideEvent(event); m_ownVisible = false; }

private:
    bool m_ownVisible = false;
};

#endif
