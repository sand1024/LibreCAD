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

#include "lc_event_filter_floating_hud.h"

#include <QApplication>
#include <QDockWidget>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionDockWidget>
#include <QTimer>

#include "lc_detached_menu.h"
#include "lc_proxy_style.h"
#include "lc_proxy_style_shared.h"
#include "lc_skin_widgets_layout_resolver.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
typedef HRESULT (WINAPI *DwmSetWindowAttributePtr)(HWND, DWORD, LPCVOID, DWORD);
#endif



// ==================== LC_EventFilterFloatingHUD Implementation ====================

LC_EventFilterFloatingHUD::LC_EventFilterFloatingHUD(QWidget *target, const LC_ProxyStyle *style)
    : LC_ProxyStyleEventFilter(target, style) {
}

bool LC_EventFilterFloatingHUD::eventFilter(QObject* watched, QEvent* event) {
    auto* widget = qobject_cast<QWidget*>(watched);
    if (!widget || !widget->inherits("QTornOffMenu"))
        return false;


    if (event->type() == QEvent::WinIdChange) {
        // Native window just created — cloak it before the OS maps it
        widget->setAttribute(Qt::WA_DontShowOnScreen, true);
#ifdef Q_OS_WIN
        // On Windows, additionally cloak at DWM level to prevent
        // the one-frame flash during window station mapping
        const HWND hwnd = reinterpret_cast<HWND>(widget->winId());
        if (hwnd) {
            const HMODULE dwm = ::LoadLibraryW(L"dwmapi.dll");
            if (dwm) {
                const auto setAttr = reinterpret_cast<DwmSetWindowAttributePtr>(
                    ::GetProcAddress(dwm, "DwmSetWindowAttribute"));
                if (setAttr) {
                    constexpr BOOL cloak = TRUE;
                    constexpr DWORD DWMWA_CLOAK = 13;
                    setAttr(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));
                }
                ::FreeLibrary(dwm);
            }
        }
#endif
        return false; // do not consume — let Qt finish its setup
    }
    if (event->type() == QEvent::Show) {
        // Safe early return: If the custom floating HUD is disabled, let standard Qt render native windows
        if (style() && !style()->m_useFloatingHUD) {
            return false;
        }

        // Extract original menu metadata before relocating the native handle
        const QPoint pos = widget->pos();
        const QString title = widget->windowTitle();
        const QList<QAction*> actions = widget->actions();

        widget->removeEventFilter(this);

        widget->hide();
        widget->setAttribute(Qt::WA_DontShowOnScreen, true); // cleanup

        // Dynamically resolve the actual source menu, stepping up from temporary QTornOffMenu if necessary
        QMenu* sourceMenu = qobject_cast<QMenu*>(parent());
        if (sourceMenu && sourceMenu->inherits("QTornOffMenu")) {
            sourceMenu = qobject_cast<QMenu*>(sourceMenu->parent());
        }

        QTimer::singleShot(5, [widget, title, actions, this, sourceMenu, pos]() {
            // At this point Qt's popup machinery has fully closed all popups — the popup stack is empty

            // Safe Top-Level Window Parentage: avoids nesting conflicts by binding to the top-level main window
            QWidget* parentWin = QApplication::activeWindow() ? QApplication::activeWindow()->window() : nullptr;

            auto *detached = new LC_DetachedMenu(title, actions, style(), sourceMenu, parentWin);
            detached->move(pos);
            detached->show();

            // Fix: Tie the lifetime of the hidden native QTornOffMenu to our custom detached HUD container
            // When the custom panel is closed/destroyed, delete the native handle to release 'tornPopup'
            connect(detached, &QObject::destroyed, widget, &QObject::deleteLater);

            this->deleteLater();
        });
        return true; // Return true to signal that the event has been fully intercepted and handled
    }
    return false;
}
