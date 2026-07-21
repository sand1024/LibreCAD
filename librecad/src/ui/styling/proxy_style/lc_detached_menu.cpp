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
#include "lc_detached_menu.h"
#include <QMenu>
#include <QEvent>
#include <QHideEvent>
#include <QAction>
#include <QVBoxLayout>
#include <QPainter>

#include "lc_custom_title_bar_widget.h"
#include "lc_proxy_style.h"
#include "lc_proxy_style_shared.h"
#include "lc_dock_title_bar.h"

static constexpr int BORDER_WIDTH = 1;

class LC_DetachedMenuContent : public QMenu {
public:
    explicit LC_DetachedMenuContent(QWidget *parent = nullptr)
        : QMenu(parent) {}

    // Overriding setVisible to robustly intercept and block standard popup closing sweeps
    void setVisible(bool visible) override {
        if (!visible && parentWidget()) {
            return; // Prevent the embedded child menu from being hidden
        }
        QMenu::setVisible(visible);
    }

protected:
    bool event(QEvent *e) override {
        // Suppress the close-on-deactivate behavior QMenu normally has
        if (e->type() == QEvent::WindowDeactivate ||
            e->type() == QEvent::ApplicationDeactivated) {
            return true; // consume without closing
            }
        return QMenu::event(e);
    }

    void hideEvent(QHideEvent *e) override {
        // QMenu hides itself on outside clicks via its grab mechanism.
        // As an embedded child we must ignore these self-hide calls.
        if (parentWidget()) {
            e->ignore();
            return;
        }
        QMenu::hideEvent(e);
    }
};

LC_DetachedMenu::LC_DetachedMenu(const QString &title,
                                 const QList<QAction*> &actions,
                                 const LC_ProxyStyle *style,
                                 QMenu *sourceMenu,
                                 QWidget *parent)
    : QWidget(parent, Qt::Tool | Qt::FramelessWindowHint),
      m_sourceMenu(sourceMenu) {
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);

    setWindowTitle(title); // Sync window title so LC_DockTitleBar queries it correctly
    m_titleBar = new LC_CustomTitleBarWidget(title, title, "", this);

    m_menu = new LC_DetachedMenuContent(this);  // subclass with deactivation suppressed
    m_menu->setWindowFlags(Qt::Widget);
    m_menu->setFocusPolicy(Qt::NoFocus);
    m_menu->setAttribute(Qt::WA_NoMousePropagation, false);
    setFocusPolicy(Qt::StrongFocus);
    m_menu->setStyle(const_cast<LC_ProxyStyle*>(style));
    m_menu->setTearOffEnabled(false);
    m_menu->setWindowFlags(Qt::Widget); // Embeds menu statically inside layout

    populateMenu(m_menu, actions);

    auto *layout = new QVBoxLayout(this);
    // Fix: Add BORDER_WIDTH to the top margin so the layout leaves space for the title bar inside the 1px border
    layout->setContentsMargins(BORDER_WIDTH, BORDER_WIDTH + m_titleBar->sizeHint().height(), BORDER_WIDTH, BORDER_WIDTH);
    layout->setSpacing(0);
    layout->addWidget(m_menu);

    if (sourceMenu) {
        connect(sourceMenu, &QObject::destroyed, this, &QWidget::close);
    }

    adjustSize();
    setFixedSize(sizeHint()); // Locks the torn-off menu palette to its exact structural bounds
}

void LC_DetachedMenu::populateMenu(QMenu *target,
                                   const QList<QAction*> &actions) {
    for (QAction *action : actions) {
        if (action->isSeparator()) {
            target->addSeparator();
            continue;
        }

        if (QMenu *sub = action->menu()) {
            // Create an independent QMenu for the submenu —
            // does not share the original QMenu* pointer
            QMenu *subCopy = new QMenu(action->text(), target);
            subCopy->setIcon(action->icon());
            subCopy->setEnabled(action->isEnabled());
            subCopy->setTearOffEnabled(true); // allow sub-tearoff
            // Recursively populate
            populateMenu(subCopy, sub->actions());
            target->addMenu(subCopy);

            // Forward enable/disable state changes from original
            connect(action, &QAction::changed, subCopy, [action, subCopy]() {
                subCopy->setEnabled(action->isEnabled());
                subCopy->setTitle(action->text());
                subCopy->setIcon(action->icon());
            });
        } else {
            // Leaf action — share the QAction pointer directly.
            // These are safe to share because leaf actions have no
            // associated QMenu that can be hidden under us.
            target->addAction(action);
        }
    }
}

void LC_DetachedMenu::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    if (!m_titleBar || !m_menu) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const bool active = isActiveWindow();
    const QPalette::ColorGroup group = active ? QPalette::Active : QPalette::Inactive;

    // 1. Draw flat card background
    const QColor bg = palette().color(group, QPalette::ToolTipBase);
    painter.setPen(Qt::NoPen);
    painter.setBrush(bg);
    painter.drawRect(rect());

    // 2. Draw 1px borders matching tooltip specifications
    const QStyle *style = m_menu->style();
    if (style) {
        if (const auto *proxyStyle = qobject_cast<const LC_ProxyStyle*>(style)) {
            const SkinColors desc = proxyStyle->getCachedStyleDescriptor(palette(), group);
            painter.setPen(QPen(desc.tooltip.tooltipBorder, 1.0));
            painter.setBrush(Qt::NoBrush);
            painter.drawRect(rect().adjusted(0, 0, -1, -1));
        }
    }
}

void LC_DetachedMenu::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (m_titleBar) {
        // Stretch the title bar horizontally inside the 1px borders
        m_titleBar->setGeometry(BORDER_WIDTH, BORDER_WIDTH,
                                width() - 2 * BORDER_WIDTH,
                                m_titleBar->sizeHint().height());
    }
}
