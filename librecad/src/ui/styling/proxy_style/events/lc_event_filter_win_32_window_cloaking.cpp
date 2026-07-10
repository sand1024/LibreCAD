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


#include "lc_event_filter_win_32_window_cloaking.h"


#ifdef Q_OS_WIN
#include <QApplication>
#include <QEvent>
#include <QTimer>
#include <windows.h>

#include "lc_proxy_style_shared.h"
typedef HRESULT (WINAPI *DwmSetWindowAttributePtr)(HWND, DWORD, LPCVOID, DWORD);

bool LC_EventFilterWin32WindowCloaking::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::Show) {
        if (auto *window = qobject_cast<QWidget*>(watched)) {
            if (window->property(PROP_STYLE_PRE_CLOAKED).toBool()) {
                const HWND hwnd = reinterpret_cast<HWND>(window->winId());
                if (hwnd) {
                    const HMODULE dwmDll = ::LoadLibraryW(L"dwmapi.dll");
                    if (dwmDll) {
                        const auto setWindowAttr = reinterpret_cast<DwmSetWindowAttributePtr>(
                            ::GetProcAddress(dwmDll, "DwmSetWindowAttribute"));

                        if (setWindowAttr) {
                            QTimer::singleShot(30, window, [hwnd, setWindowAttr, window]() {
                                constexpr BOOL uncloak = FALSE;
                                constexpr DWORD DWMWA_CLOAK = 13;
                                setWindowAttr(hwnd, DWMWA_CLOAK, &uncloak, sizeof(uncloak));

                                window->setProperty(PROP_STYLE_PRE_CLOAKED, false);
                                ::RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
                                QApplication::restoreOverrideCursor();
                            });
                        }
                        ::FreeLibrary(dwmDll);
                    }
                }
            }
        }
    }
    return false;
}
#endif
