
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

#ifndef LC_DETACHEDMENU_H
#define LC_DETACHEDMENU_H

#include <QPointer>
#include <QWidget>

class LC_CustomTitleBarWidget;
class LC_DockTitleBar;
class LC_ProxyStyle;

class LC_DetachedMenu : public QWidget {
    Q_OBJECT
public:
    LC_DetachedMenu(const QString &title,
                    const QList<QAction*> &actions,
                    const LC_ProxyStyle *style,
                    QMenu *sourceMenu = nullptr,
                    QWidget *parent = nullptr);

    static void populateMenu(QMenu* target, const QList<QAction*>& actions);
protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPointer<QMenu> m_sourceMenu;
    QMenu *m_menu;
    LC_CustomTitleBarWidget *m_titleBar;
};

#endif
