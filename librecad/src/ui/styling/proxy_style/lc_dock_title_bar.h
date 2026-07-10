
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

#ifndef LC_DOCK_TITLE_BAR_H
#define LC_DOCK_TITLE_BAR_H

#include <QWidget>
#include <QPointer>

class QDockWidget;
class LC_ProxyStyle;

#ifndef Q_OS_WIN
#define USE_RESIZE_STRIP
#endif

#define USE_RESIZE_STRIP

#ifdef USE_RESIZE_STRIP
class LC_EdgeResizeStrip : public QWidget {
    Q_OBJECT
public:
    enum Edge { Left, Right, Bottom, BottomLeft, BottomRight };

    LC_EdgeResizeStrip(Edge edge, QWidget *parent);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    static Qt::CursorShape cursorForEdge(Edge e);
    bool isResizeZone(const QPoint &localPos, const QPoint &globalPos) const;

    Edge m_edge;
    bool m_dragging = false;
    QPoint m_startPos;
    QRect m_startGeom;
};
#endif


class LC_DockTitleBar : public QWidget {
    Q_OBJECT
public:
    explicit LC_DockTitleBar(QDockWidget *dock, const LC_ProxyStyle *style, QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    bool isVertical() const;
    bool isClosable() const;

    QPointer<QDockWidget> m_dock;
    const LC_ProxyStyle *m_style;

    QToolButton *m_closeBtn;
    QToolButton *m_floatBtn;

    // Direct Drag mechanics for standalone frameless windows (e.g. detached menus)
    bool m_dragging = false;
    QPoint m_dragOffset;

#ifdef USE_RESIZE_STRIP
    LC_EdgeResizeStrip *m_gripLeft;
    LC_EdgeResizeStrip *m_gripRight;
    LC_EdgeResizeStrip *m_gripBottom;
    LC_EdgeResizeStrip *m_gripBottomLeft;
    LC_EdgeResizeStrip *m_gripBottomRight;
#endif
};

#endif
