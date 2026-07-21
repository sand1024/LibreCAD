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

#include "lc_event_filter_dialog.h"

#include <QDateTime>
#include <QDialog>
#include <QEvent>
#include <QLayout>
#include <QMouseEvent>
#include <QWindow>

#include "lc_proxy_style.h"
#include "rs_debug.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <windowsx.h>
typedef HRESULT (WINAPI *DwmSetWindowAttributePtr)(HWND, DWORD, LPCVOID, DWORD);
#endif

bool LC_EventFilterDialog::eventFilter(QObject* watched, QEvent* event) {
    auto* widget = qobject_cast<QWidget*>(watched);
    if (!widget) {
        return false;
    }

    // Resolve the top-level window context so child widget hover events are processed [3]
    auto* dialog = qobject_cast<QDialog*>(widget->window());
    if (!dialog) {
        return false;
    }

    const QEvent::Type type = event->type();

    if (type == QEvent::WinIdChange) {
        // Only run for the top-level dialog itself during handle creation
        if (watched == dialog && m_style && m_style->customDialogTitleBarEnabled()) {
            dialog->setAttribute(Qt::WA_DontShowOnScreen, true);
#ifdef Q_OS_WIN
            const HWND hwnd = reinterpret_cast<HWND>(dialog->winId());
            if (hwnd) {
                const HMODULE dwm = ::LoadLibraryW(L"dwmapi.dll");
                if (dwm) {
                    const auto setAttr = reinterpret_cast<DwmSetWindowAttributePtr>(
                        ::GetProcAddress(dwm, "DwmSetWindowAttribute"));
                    if (setAttr) {
                        constexpr BOOL cloak = TRUE;
                        constexpr DWORD DWMWA_CLOAK = 13;
                        setAttr(hwnd, DWMWA_CLOAK, &cloak, sizeof(cloak));
                        dialog->setProperty(PROP_STYLE_PRE_CLOAKED, true);
                    }
                    ::FreeLibrary(dwm);
                }
            }
#endif
        }
        return false;
    }

    if (type == QEvent::Show) {
        if (watched != dialog) {
            return false;
        }
        if (m_style && !m_style->customDialogTitleBarEnabled()) {
            return false;
        }
        if (dialog->property("lcfs_dialogTitleBarInstalled").toBool()) {
            return false;
        }

        dialog->hide();
        dialog->setAttribute(Qt::WA_DontShowOnScreen, true);

        // Strip native borders
        dialog->setWindowFlags(dialog->windowFlags() | Qt::FramelessWindowHint);

        // Instantiate and inject our custom title bar widget [1]
        m_style->setupCustomDialogTitleBar(dialog);

        // Apply a 1px margin around the dialog content to reveal the custom border [2]
        dialog->setContentsMargins(1, 1, 1, 1);

        // Enable mouse and hover tracking on the container for seamless border resizes
        dialog->setMouseTracking(true);
        dialog->setAttribute(Qt::WA_Hover, true);

        // Mark as successfully initialized
        dialog->setProperty("lcfs_dialogTitleBarInstalled", true);

        // Restore screen mapping and show
        dialog->setAttribute(Qt::WA_DontShowOnScreen, false);
        dialog->show();

        return true;
    }

    // Determine layout-based size constraints to filter out fixed-size dialogs
    bool isResizable = dialog->minimumSize() != dialog->maximumSize();
    if (isResizable && dialog->layout()) {
        if (dialog->layout()->sizeConstraint() == QLayout::SetFixedSize) {
            isResizable = false;
        }
    }

    const bool isResizing = dialog->property("lcfs_isResizing").toBool();

    // 1. Drag Resizing State Machine (Manual Geometry Scaling)
    if (isResizing) {
        if (type == QEvent::MouseMove) {
            const QPoint startPos = dialog->property("lcfs_resizeDragStartPos").toPoint();
            const QRect startGeom = dialog->property("lcfs_resizeStartGeometry").toRect();
            const int edges = dialog->property("lcfs_resizeEdges").toInt();

            const QPoint delta = QCursor::pos() - startPos;

            int minW = qMax(dialog->minimumSize().width(), dialog->minimumSizeHint().width());
            int minH = qMax(dialog->minimumSize().height(), dialog->minimumSizeHint().height());
            if (dialog->layout()) {
                minW = qMax(minW, dialog->layout()->minimumSize().width());
                minH = qMax(minH, dialog->layout()->minimumSize().height());
            }
            if (minW <= 0) minW = 100; // Safe fallback
            if (minH <= 0) minH = 100;
            int maxW = dialog->maximumSize().width();
            int maxH = dialog->maximumSize().height();

            QRect g = startGeom;
            if (edges & Qt::LeftEdge) {
                int newWidth = qBound(minW, startGeom.width() - delta.x(), maxW);
                g.setLeft(startGeom.right() - newWidth + 1);
            }
            if (edges & Qt::RightEdge) {
                int newWidth = qBound(minW, startGeom.width() + delta.x(), maxW);
                g.setWidth(newWidth);
            }
            if (edges & Qt::TopEdge) {
                int newHeight = qBound(minH, startGeom.height() - delta.y(), maxH);
                g.setTop(startGeom.bottom() - newHeight + 1);
            }
            if (edges & Qt::BottomEdge) {
                int newHeight = qBound(minH, startGeom.height() + delta.y(), maxH);
                g.setHeight(newHeight);
            }

            // 60 FPS update throttling to eliminate visual glitches on resize
            const qint64 lastUpdate = dialog->property("lcfs_resizeLastUpdateTime").toLongLong();
            const qint64 now = QDateTime::currentMSecsSinceEpoch();

            if (g != dialog->geometry() && (now - lastUpdate >= 16)) {
                dialog->setGeometry(g);
                dialog->setProperty("lcfs_resizeLastUpdateTime", now);
            }

            event->accept();
            return true;
        }
        if (type == QEvent::MouseButtonRelease) {
            dialog->setProperty("lcfs_isResizing", false);
            dialog->unsetCursor();
            event->accept();
            return true;
        }
    }

    // 2. Hover Edge Detection and Interaction Grab
    if (isResizable && m_style && m_style->customDialogTitleBarEnabled() && dialog->property("lcfs_dialogTitleBarInstalled").toBool()) {
        if (type == QEvent::MouseMove || type == QEvent::HoverMove || type == QEvent::MouseButtonPress) {
            // Symmetrical global coordinate mapping prevents scaling and layout-margin shift bugs
            const QPoint localPos = dialog->mapFromGlobal(QCursor::pos());
            const QRect rect = dialog->rect();
            const qreal dpr = dialog->devicePixelRatioF();
            const int border = qMax(11, qRound(8.0 * dpr));

            int edges = 0;
            if (localPos.x() < border)                 edges |= Qt::LeftEdge;
            if (localPos.x() > rect.width() - border)  edges |= Qt::RightEdge;
            if (localPos.y() < border)                 edges |= Qt::TopEdge;
            if (localPos.y() > rect.height() - border) edges |= Qt::BottomEdge;

            if (edges != 0) {
                // Determine and set the correct native cursor shape based on hovered edges
                Qt::CursorShape cursor = Qt::ArrowCursor;
                if (edges == (Qt::LeftEdge | Qt::TopEdge) || edges == (Qt::RightEdge | Qt::BottomEdge)) {
                    cursor = Qt::SizeFDiagCursor;
                } else if (edges == (Qt::RightEdge | Qt::TopEdge) || edges == (Qt::LeftEdge | Qt::BottomEdge)) {
                    cursor = Qt::SizeBDiagCursor;
                } else if (edges & (Qt::LeftEdge | Qt::RightEdge)) {
                    cursor = Qt::SizeHorCursor;
                } else if (edges & (Qt::TopEdge | Qt::BottomEdge)) {
                    cursor = Qt::SizeVerCursor;
                }

                dialog->setCursor(cursor);

                // Initiate manual dragging and capture focus on click inside zone
                if (type == QEvent::MouseButtonPress) {
                    auto* mouseEvent = static_cast<QMouseEvent*>(event);
                    if (mouseEvent->button() == Qt::LeftButton) {
                        dialog->setProperty("lcfs_isResizing", true);
                        dialog->setProperty("lcfs_resizeDragStartPos", QCursor::pos());
                        dialog->setProperty("lcfs_resizeStartGeometry", dialog->geometry());
                        dialog->setProperty("lcfs_resizeEdges", edges);
                        dialog->setProperty("lcfs_resizeLastUpdateTime", 0); // Force immediate first redraw
                        event->accept();
                        return true;
                    }
                }
            } else {
                // Restore standard cursor shape when outside the resize boundaries
                dialog->unsetCursor();
            }
        }
    }

    return false;
}


     // LC_ERR << "[Dialog Resize Debug]"
     //           << " EventType:" << type
     //           << " WatchedWidget:" << watched->metaObject()->className()
     //           << " isResizable:" << isResizable
     //           << " hasWindowHandle:" << (dialog->windowHandle() != nullptr)
     //           << " MousePos:" << localPos.x() << ", " << localPos.y()
     //           << " DialogRect:" << rect.x() << "," << rect.y() << "," << rect.width() << "," << rect.height();
