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

#include "lc_dock_title_bar.h"

#include "lc_dock_title_bar.h"
#include <QDockWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QStyleOptionDockWidget>
#include "lc_proxy_style.h"
#include "lc_proxy_style_shared.h"
#include "lc_skin_widgets_layout_resolver.h"

#ifdef USE_RESIZE_STRIP
#include <QScrollBar>

LC_EdgeResizeStrip::LC_EdgeResizeStrip(Edge edge, QWidget *parent)
    : QWidget(parent), m_edge(edge) {
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setMouseTracking(true);
    setCursor(cursorForEdge(edge));
}

bool LC_EdgeResizeStrip::isResizeZone(const QPoint &localPos, const QPoint &globalPos) const {
    if (!parentWidget()) return true;

    // 1. Locate any active visible scrollbar directly under the cursor
    QList<QScrollBar*> scrollBars = parentWidget()->findChildren<QScrollBar*>();
    QScrollBar *underCursorScrollBar = nullptr;
    for (QScrollBar *sb : scrollBars) {
        if (sb->isVisible() && sb->rect().contains(sb->mapFromGlobal(globalPos))) {
            underCursorScrollBar = sb;
            break;
        }
    }

    if (!underCursorScrollBar) {
        return true; // No scrollbar collision: the entire 8px width is a resize zone
    }

    // 2. Query scrollbar width/height at runtime to handle custom widths and High-DPI scaling
    const int scrollbarExtent = (m_edge == Left || m_edge == Right)
                                ? underCursorScrollBar->width()
                                : underCursorScrollBar->height();

    // Allocate the outer 30% of the custom scrollbar extent as the guaranteed resize margin
    const int guaranteedResizeWidth = qMax(3, qRound(scrollbarExtent * 0.3));

    // 3. Perform edge distance validations based on target border layout rules
    switch (m_edge) {
        case Left:        return localPos.x() < guaranteedResizeWidth;
        case Right:       return (width() - localPos.x()) < guaranteedResizeWidth;
        case Bottom:      return (height() - localPos.y()) < guaranteedResizeWidth;
        case BottomLeft:  return localPos.x() < guaranteedResizeWidth || (height() - localPos.y()) < guaranteedResizeWidth;
        case BottomRight: return (width() - localPos.x()) < guaranteedResizeWidth || (height() - localPos.y()) < guaranteedResizeWidth;
    }
    return true;
}

void LC_EdgeResizeStrip::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const QPoint globalPos = e->globalPosition().toPoint();
#else
        const QPoint globalPos = e->globalPos();
#endif

        if (isResizeZone(e->pos(), globalPos)) {
            m_dragging = true;
            m_startPos  = globalPos;
            m_startGeom = window()->geometry();
            e->accept();
        } else {
            e->ignore(); // Fallback propagation lets user interact with vertical/horizontal scrollbars
        }
    }
}

void LC_EdgeResizeStrip::mouseMoveEvent(QMouseEvent *e) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPoint globalPos = e->globalPosition().toPoint();
#else
    const QPoint globalPos = e->globalPos();
#endif

    if (m_dragging) {
        const QPoint delta = globalPos - m_startPos;
        QRect g = m_startGeom;

        switch (m_edge) {
            case Left:        g.setLeft(g.left()   + delta.x()); break;
            case Right:       g.setRight(g.right() + delta.x()); break;
            case Bottom:      g.setBottom(g.bottom() + delta.y()); break;
            case BottomLeft:  g.setLeft(g.left()   + delta.x());
                              g.setBottom(g.bottom() + delta.y()); break;
            case BottomRight: g.setRight(g.right() + delta.x());
                              g.setBottom(g.bottom() + delta.y()); break;
        }

        const QSize minS = window()->minimumSizeHint();
        if (g.width() >= minS.width() && g.height() >= minS.height()) {
            window()->setGeometry(g);
        }
        e->accept();
    } else {
        // Not dragging yet: dynamically swap cursor shape based on pixel-level hit test
        if (isResizeZone(e->pos(), globalPos)) {
            setCursor(cursorForEdge(m_edge));
        } else {
            setCursor(Qt::ArrowCursor); // Shows standard scroll cursor when over prioritized scroll zone
        }
        e->ignore(); // Propagate hover moves so scrollbar track highlights function natively
    }
}

void LC_EdgeResizeStrip::mouseReleaseEvent(QMouseEvent *e) {
    m_dragging = false;
    e->accept();
}

Qt::CursorShape LC_EdgeResizeStrip::cursorForEdge(Edge e) {
    switch (e) {
        case Left:
        case Right:       return Qt::SizeHorCursor;
        case Bottom:      return Qt::SizeVerCursor;
        case BottomLeft:  return Qt::SizeBDiagCursor;
        case BottomRight: return Qt::SizeFDiagCursor;
    }
    return Qt::ArrowCursor;
}
#endif

LC_DockTitleBar::LC_DockTitleBar(QDockWidget *dock, const LC_ProxyStyle *style, QWidget *parent)
    : QWidget(parent), m_dock(dock), m_style(style) {
    setMouseTracking(true);

    m_closeBtn = new QToolButton(this);
    m_closeBtn->setAutoRaise(true);
    m_closeBtn->setCursor(Qt::ArrowCursor); // Overrides the OpenHandCursor of the container
    m_closeBtn->setProperty(PROP_IS_DOCK_TITLE_BUTTON, true);
    if (m_style) {
        m_closeBtn->setIcon(m_style->standardIcon(QStyle::SP_DockWidgetCloseButton, nullptr, m_closeBtn));
    }
    connect(m_closeBtn, &QToolButton::clicked, this, [this, dock]() {
        if (dock) {
            dock->close();
        } else if (window()) {
            window()->close(); // Direct window close fallback for LC_DetachedMenu
        }
    });

    m_floatBtn = new QToolButton(this);
    m_floatBtn->setAutoRaise(true);
    m_floatBtn->setCursor(Qt::ArrowCursor); // Overrides the OpenHandCursor of the container
    m_floatBtn->setProperty(PROP_IS_DOCK_TITLE_BUTTON, true);
    if (m_style) {
        m_floatBtn->setIcon(m_style->standardIcon(
            (dock && dock->isFloating()) ? QStyle::SP_TitleBarNormalButton
                                 : QStyle::SP_TitleBarMaxButton, nullptr, m_floatBtn));
    }
    connect(m_floatBtn, &QToolButton::clicked, this, [dock]() {
        if (dock) {
            dock->setFloating(!dock->isFloating());
        }
    });

    if (dock) {
        connect(dock, &QDockWidget::windowTitleChanged, this, qOverload<>(&QWidget::update));
        connect(dock, &QDockWidget::featuresChanged, this, [this]() {
            updateGeometry();
            update();
        });
        connect(dock, &QDockWidget::topLevelChanged, this, [this, dock](bool floating) {
            if (m_style) {
                m_floatBtn->setIcon(m_style->standardIcon(
                    floating ? QStyle::SP_TitleBarNormalButton
                             : QStyle::SP_TitleBarMaxButton, nullptr, m_floatBtn));
            }
            // Invalidate layout cache on both the title bar and parent dock widget
            updateGeometry();
            dock->updateGeometry();
            update();
        });
    } else {
        // Detached menu fallback: subscribe to parent window title changes
        if (parent) {
            connect(parent, &QWidget::windowTitleChanged, this, qOverload<>(&QWidget::update));
        }
    }

#ifdef USE_RESIZE_STRIP
    if (dock) {
        m_gripLeft        = new LC_EdgeResizeStrip(LC_EdgeResizeStrip::Left,        dock);
        m_gripRight       = new LC_EdgeResizeStrip(LC_EdgeResizeStrip::Right,       dock);
        m_gripBottom      = new LC_EdgeResizeStrip(LC_EdgeResizeStrip::Bottom,      dock);
        m_gripBottomLeft  = new LC_EdgeResizeStrip(LC_EdgeResizeStrip::BottomLeft,  dock);
        m_gripBottomRight = new LC_EdgeResizeStrip(LC_EdgeResizeStrip::BottomRight, dock);
    }
#endif
}

bool LC_DockTitleBar::isVertical() const {
    if (!m_dock) return false;
    if (m_dock->isFloating()) return false;

    // Physical geometry check once widget layout has occurred
    if (width() > 0 && height() > 0) {
        return width() < height();
    }

    // Initial size negotiation fallback
    return m_dock->features().testFlag(QDockWidget::DockWidgetVerticalTitleBar);
}


bool LC_DockTitleBar::isClosable() const {
    if (!m_dock) return true;
    return m_dock && m_dock->features().testFlag(QDockWidget::DockWidgetClosable);
}

QSize LC_DockTitleBar::sizeHint() const {
    if (!m_style) return QSize(0, 0);
    int height = m_style->pixelMetric(QStyle::PM_TitleBarHeight, nullptr, this);
    if (isVertical()) {
        return QSize(height, QWIDGETSIZE_MAX);
    }
    return QSize(100, height);
}

QSize LC_DockTitleBar::minimumSizeHint() const {
    if (!m_style) return QSize(0, 0);
    int height = m_style->pixelMetric(QStyle::PM_TitleBarHeight, nullptr, this);
    if (isVertical()) {
        return QSize(height, 50);
    }
    return QSize(50, height);
}

void LC_DockTitleBar::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (!m_style) return;

    QDockWidget::DockWidgetFeatures features = QDockWidget::NoDockWidgetFeatures;
    if (m_dock) {
        features = m_dock->features();
    } else {
        features = QDockWidget::DockWidgetClosable;
    }

    const SkinScaledGeometries &geoms = m_style->getGeometries(this);
    const auto buttonLayout = LC_SkinWidgetsLayoutResolver::resolveTitleBarButtonLayout(rect(), geoms, features, isVertical());

    m_closeBtn->setGeometry(buttonLayout.closeRect);
    m_closeBtn->setVisible(isClosable());

    m_floatBtn->setGeometry(buttonLayout.floatRect);
    m_floatBtn->setVisible(m_dock && m_dock->features().testFlag(QDockWidget::DockWidgetFloatable));

#ifdef USE_RESIZE_STRIP
    if (m_dock != nullptr) {
        const bool floating = m_dock->isFloating();
        m_gripLeft->setVisible(floating);
        m_gripRight->setVisible(floating);
        m_gripBottom->setVisible(floating);
        m_gripBottomLeft->setVisible(floating);
        m_gripBottomRight->setVisible(floating);

        if (floating) {
            const int b = 8; // Width matching standard OS borders
            const QRect r = m_dock->rect();
            m_gripLeft->setGeometry(0, b, b, r.height() - 2 * b);
            m_gripRight->setGeometry(r.right() - b, b, b, r.height() - 2 * b);
            m_gripBottom->setGeometry(b, r.bottom() - b, r.width() - 2 * b, b);
            m_gripBottomLeft->setGeometry(0, r.bottom() - b, b, b);
            m_gripBottomRight->setGeometry(r.right() - b, r.bottom() - b, b, b);
        }
    }
#endif
}

void LC_DockTitleBar::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    if (!m_style) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const bool active = m_dock ? m_dock->isActiveWindow() : (window() && window()->isActiveWindow());
    const bool vertical = isVertical();

    QStyleOptionDockWidget titleOpt;
    titleOpt.rect = rect();
    titleOpt.title = m_dock ? m_dock->windowTitle() : (window() ? window()->windowTitle() : QString());
    titleOpt.verticalTitleBar = vertical;
    if (active) {
        titleOpt.state |= QStyle::State_Active;
    } else {
        titleOpt.state &= ~QStyle::State_Active;
    }

    m_style->drawCustomDockTitleBar(&titleOpt, &painter, m_dock ? static_cast<QWidget*>(m_dock.data()) : parentWidget());
}

void LC_DockTitleBar::mousePressEvent(QMouseEvent *event) {
     if (event->button() == Qt::LeftButton) {
        // Move standalone frameless menus manually
        if (!m_dock) {
            m_dragging = true;
            m_dragOffset = event->globalPosition().toPoint() - window()->frameGeometry().topLeft();
            event->accept();
            return;
        }
    }
    event->ignore(); // Propagates up to m_dock to let Qt handle native drag snaps
}

void LC_DockTitleBar::mouseReleaseEvent(QMouseEvent *event) {
    if (!m_dock) {
        m_dragging = false;
        event->accept();
        return;
    }
    event->ignore();
}

void LC_DockTitleBar::mouseDoubleClickEvent(QMouseEvent *event) {
    // Propagate double click to native dock to trigger native float-dock transitions
    event->ignore();
}

void LC_DockTitleBar::mouseMoveEvent(QMouseEvent *event) {
    const QPoint globalPos = event->globalPosition().toPoint();
    if (!m_dock && m_dragging) {
        window()->move(globalPos - m_dragOffset);
        event->accept();
        return;
    }

    if (!m_dock) {
        setCursor(Qt::OpenHandCursor);
    }
    event->ignore(); // Propagates moves to allow standard dock mouse tracking
}

void LC_DockTitleBar::leaveEvent(QEvent *event) {
    Q_UNUSED(event);
    setCursor(Qt::ArrowCursor);
}

void LC_DockTitleBar::changeEvent(QEvent *event) {
    if (event->type() == QEvent::ActivationChange || event->type() == QEvent::WindowTitleChange) {
        update();
    }
    QWidget::changeEvent(event);
}
