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

#ifndef LC_WinResizeNativeEventFilter_H
#define LC_WinResizeNativeEventFilter_H

#include <QDockWidget>
#include <QDialog>
#include <QtGlobal>
#include <QWidget>

#ifdef Q_OS_WIN
#define USE_WIN_NATIVE_RESIZE_FILTER
#endif

#ifdef USE_WIN_NATIVE_RESIZE_FILTER
class LC_WinResizeNativeEventFilter;
#endif

#ifdef USE_WIN_NATIVE_RESIZE_FILTER
#include <QAbstractNativeEventFilter>
#include <windows.h>
#include <windowsx.h>

#ifdef Q_OS_WIN
typedef HRESULT (WINAPI *DwmSetWindowAttributePtr)(HWND, DWORD, LPCVOID, DWORD);
#endif

class LC_WinResizeNativeEventFilter : public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override {
        if (eventType == "windows_generic_MSG") {
            const MSG *msg = static_cast<MSG*>(message);
            if (msg->message == WM_NCHITTEST) {
                const HWND hwnd = msg->hwnd;
                QWidget *w = QWidget::find(reinterpret_cast<WId>(hwnd));

                bool isResizable = false;
                HWND topHwnd = nullptr; // Track the correct top-level window handle for coordinate checks

                if (w) {
                    QWidget *window = w->window();

                    // Safely locate the parent QDockWidget in both floating and nested hierarchies
                    QDockWidget *dock = qobject_cast<QDockWidget*>(window);
                    if (!dock) {
                        dock = window->findChild<QDockWidget*>();
                        if (!dock) {
                            QWidget *parentW = w->parentWidget();
                            while (parentW) {
                                if (auto *d = qobject_cast<QDockWidget*>(parentW)) {
                                    dock = d;
                                    break;
                                }
                                parentW = parentW->parentWidget();
                            }
                        }
                    }

                    if (dock) {
                        // Only invoke native resize handles if the dock widget is actively floating
                        if (dock->isFloating()) {
                            isResizable = true;
                            topHwnd = reinterpret_cast<HWND>(window->winId()); // Resolve top-level HWND
                        }
                    }
                    else if (auto *dialog = qobject_cast<QDialog*>(window)) {
                        /*// Natively support resizable, frameless dialogs via correct window boundaries
                        if (dialog->windowFlags().testFlag(Qt::FramelessWindowHint) &&
                            dialog->minimumSize() != dialog->maximumSize()) {
                            isResizable = true;
                            topHwnd = reinterpret_cast<HWND>(dialog->winId()); // Resolve top-level HWND [3]
                        }*/
                    }
                }

                if (isResizable && topHwnd) {
                    RECT winRect;
                    // Fix: Query coordinates of the top-level window HWND, not the child's HWND [3]
                    ::GetWindowRect(topHwnd, &winRect);

                    const int x = GET_X_LPARAM(msg->lParam);
                    const int y = GET_Y_LPARAM(msg->lParam);
                    const int border = 8; // Expanded comfortable hit zone

                    const bool left   = (x < winRect.left   + border);
                    const bool right  = (x > winRect.right  - border);
                    const bool top    = (y < winRect.top    + border);
                    const bool bottom = (y > winRect.bottom - border);

                    if (left  && top)    { *result = HTTOPLEFT;     return true; }
                    if (right && top)    { *result = HTTOPRIGHT;    return true; }
                    if (left  && bottom) { *result = HTBOTTOMLEFT;  return true; }
                    if (right && bottom) { *result = HTBOTTOMRIGHT; return true; }
                    if (left)            { *result = HTLEFT;        return true; }
                    if (right)           { *result = HTRIGHT;       return true; }
                    if (bottom)          { *result = HTBOTTOM;      return true; }
                }
            }
        }
        return false;
    }
};
#endif
#endif
