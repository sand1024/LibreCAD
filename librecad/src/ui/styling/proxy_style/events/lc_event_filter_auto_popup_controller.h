
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

#ifndef LC_EVENTFILTERAUTOPOPUPCONTROLLER_H
#define LC_EVENTFILTERAUTOPOPUPCONTROLLER_H

#include <QPointer>

#include "lc_proxy_style_event_filter.h"

class QAction;

class LC_EventFilterAutoPopupController : public LC_ProxyStyleEventFilter {
    Q_OBJECT
public:
    explicit LC_EventFilterAutoPopupController(const LC_ProxyStyle *style);
    ~LC_EventFilterAutoPopupController() override;

    bool eventFilter(QObject *watched, QEvent *event) override;

    QPointer<QWidget> hoveredPopupTrigger() const { return m_hoveredPopupTrigger; }
    QPointer<QAction> hoveredMenuBarAction() const { return m_hoveredMenuBarAction; }

private slots:
    void handlePopupTimeout();
    void handleLeaveCloseTimeout();
    void onMenuAboutToHide();

private:

    void handleSegmentedButtonEnter(QObject *watched);
    void handleToolbarOverflowEnter(QObject *watched);
    void handleInstantButtonEnter(QObject *watched);
    void handleReEntryCancellation(QObject *watched);

    // Dynamic Leave Event Handlers
    void handleSegmentedButtonLeave(QObject *watched);
    void handleLabelLinkLeave(QObject *watched);
    void handleHoverLeaveCleanup(QObject *watched);

    // Dynamic Mouse/Hover Move Event Handlers
    bool handleActiveGrabNavigation(QObject *watched);
    void handleMenuBarHover(QObject *watched);

    QTimer *m_toolbarPopupTimer = nullptr;
    QTimer *m_leaveCloseTimer = nullptr;

    QPointer<QWidget> m_hoveredPopupTrigger;
    QPointer<QAction> m_hoveredMenuBarAction;
};

#endif
