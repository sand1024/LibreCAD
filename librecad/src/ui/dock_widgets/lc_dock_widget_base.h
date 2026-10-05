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

#ifndef LC_DOCKWIDGET_BASE_H
#define LC_DOCKWIDGET_BASE_H

#include <QDockWidget>
#include <QPointer>

class LC_DockWidgetBase : public QDockWidget {
    Q_OBJECT

public:
    explicit LC_DockWidgetBase(QWidget* parent = nullptr,
                               const QString& title = QString(),
                               const QString& verticalTitle = QString(),
                               bool isCadDock = false,
                               const Qt::WindowFlags& flags = Qt::WindowFlags());
    void showEvent(QShowEvent* event);
    ~LC_DockWidgetBase() override = default;

    QString realTitle() const;
    void setRealTitle(const QString& title);

    QString verticalTitle() const;
    void setVerticalTitle(const QString& title);

    void setWindowTitle(const QString& title);

    bool isCadDock() const;
    void setIsCadDock(bool isCad);

    void setIconOnlyTabMode(bool iconOnly);
    bool isIconOnlyTabMode() const;

    bool isTabActive() const;
    bool activateDockTab();
    void floatWithOffset();

    void setFocusTargetWidget(QWidget* target);
    QWidget* focusTargetWidget() const;

    static const bool isCADDockWidget(QDockWidget* dw);

public slots:
    void toggleDockVisibility();

protected slots:
    void onTopLevelChanged(bool floating);

private:
    QString m_realTitle;
    QString m_verticalTitle;
    bool m_isCadDock{false};
    bool m_iconOnlyTabMode{false};
    Qt::DockWidgetArea m_lastDockArea{Qt::NoDockWidgetArea};
    QPointer<QWidget> m_focusTargetWidget{nullptr};
};

#endif
