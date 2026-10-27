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

#include "lc_coordinate_widget_base.h"

#include <algorithm>
#include <QAction>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QGuiApplication>
#include <QLabel>
#include <QMenu>
#include <QToolButton>
#include "rs_debug.h"
#include "rs_dialogfactory.h"
#include "rs_dialogfactoryinterface.h"

LC_CoordinateWidgetBase::LC_CoordinateWidgetBase(QWidget* parent, const Qt::WindowFlags fl)
    : QWidget(parent, fl), m_iconLocked(QStringLiteral(":/icons/locked.lci")), m_iconUnlocked(QStringLiteral(":/icons/unlocked.lci")) {
    createActions();
}

void LC_CoordinateWidgetBase::createActions() {
    m_actionLock = new QAction(m_iconLocked, tr("Lock"), this);
    m_actionLock->setToolTip(tr("Lock coordinates"));
    connect(m_actionLock, &QAction::triggered, this, [this]() {
        setLocked(true);
    });

    m_actionUnlock = new QAction(m_iconUnlocked, tr("Unlock"), this);
    m_actionUnlock->setToolTip(tr("Unlock coordinates"));
    connect(m_actionUnlock, &QAction::triggered, this, [this]() {
        setLocked(false);
    });
}

bool LC_CoordinateWidgetBase::isLocked() const {
    return m_isLocked;
}

void LC_CoordinateWidgetBase::setLocked(const bool locked) {
    if (m_isLocked != locked) {
        m_isLocked = locked;
        updateLockVisuals();
        emit lockToggled(m_isLocked);
    }
}

QAction* LC_CoordinateWidgetBase::actionLock() const {
    return m_actionLock;
}

QAction* LC_CoordinateWidgetBase::actionUnlock() const {
    return m_actionUnlock;
}

void LC_CoordinateWidgetBase::initLockControl(QToolButton* button) {
    m_lockButton = button;
    if (m_lockButton != nullptr) {
        m_lockButton->setAutoRaise(true);
        m_lockButton->setFocusPolicy(Qt::NoFocus);
        connect(m_lockButton, &QToolButton::clicked, this, [this]() {
            setLocked(!m_isLocked);
        });
    }
    updateLockVisuals();
}

void LC_CoordinateWidgetBase::registerCoordinateField(QLabel* label, const QString& description, std::function<RS_Vector()> getter) {
    if (label != nullptr) {
        m_coordinateFields.append({label, description, std::move(getter)});
        label->installEventFilter(this);
        label->setEnabled(!m_isLocked);
    }
}

void LC_CoordinateWidgetBase::updateLockVisuals() {
    if (m_lockButton != nullptr) {
        if (m_isLocked) {
            m_lockButton->setIcon(m_iconLocked);
            m_lockButton->setToolTip(tr("Unlock coordinates"));
        }
        else {
            m_lockButton->setIcon(m_iconUnlocked);
            m_lockButton->setToolTip(tr("Lock coordinates"));
        }
    }

    for (const LC_CoordinateField& field : m_coordinateFields) {
        if (field.m_label != nullptr) {
            field.m_label->setEnabled(!m_isLocked);
        }
    }
}

void LC_CoordinateWidgetBase::copyToClipboard(const QString& text) const {
    QClipboard* clipboard = QGuiApplication::clipboard();
    if (clipboard != nullptr) {
        clipboard->setText(text);
    }
}

void LC_CoordinateWidgetBase::onInsertToCmd(const RS_Vector& coord, const QString& text) {
    // LC_ERR << "LC_CoordinateWidgetBase::onInsertToCmd: text=" << text << " (" << coord.x << ", " << coord.y << ")";
    RS_DIALOGFACTORY->command(text, false);// fixme - sand  - review later how to pass data to input, as cmd will be reworked
    emit insertToCmdRequested(coord, text);
}

void LC_CoordinateWidgetBase::contextMenuEvent(QContextMenuEvent* event) {
    if (event != nullptr) {
        // LC_ERR << "LC_CoordinateWidgetBase::contextMenuEvent caught at pos=" << event->pos().x() << ", " << event->pos().y();
        showContextMenu(event->globalPos(), childAt(event->pos()));
    }
}

bool LC_CoordinateWidgetBase::eventFilter(QObject* watched, QEvent* event) {
    if (event != nullptr && watched != nullptr) {
        if (event->type() == QEvent::ContextMenu) {
            auto* menuEvent = static_cast<QContextMenuEvent*>(event);
            // LC_ERR << "LC_CoordinateWidgetBase::eventFilter: ContextMenu event caught on " << watched;
            showContextMenu(menuEvent->globalPos(), qobject_cast<QWidget*>(watched));
            return true;
        }

        if (event->type() == QEvent::MouseButtonRelease) {
            auto* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::RightButton) {
                // LC_ERR << "LC_CoordinateWidgetBase::eventFilter: Right MouseButtonRelease caught on " << watched;
                showContextMenu(mouseEvent->globalPosition().toPoint(), qobject_cast<QWidget*>(watched));
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}

void LC_CoordinateWidgetBase::showContextMenu(const QPoint& globalPos, QWidget* sourceWidget) {
    // LC_ERR << "LC_CoordinateWidgetBase::showContextMenu: isLocked=" << m_isLocked
    // << " sourceWidget=" << sourceWidget;

    QMenu menu(this);

    // Unlocked state: only the Lock action is shown
    if (!m_isLocked) {
        if (m_actionLock != nullptr) {
            menu.addAction(m_actionLock);
        }
        menu.exec(globalPos);
        return;
    }

    // Locked state: Unlock action + copy options + insert to cmd
    if (m_actionUnlock != nullptr) {
        menu.addAction(m_actionUnlock);
    }

    menu.addSeparator();

    // Identify target coordinate field by widget pointer
    const LC_CoordinateField* clickedField = nullptr;
    if (sourceWidget != nullptr) {
        for (const LC_CoordinateField& field : m_coordinateFields) {
            if (field.m_label != nullptr && (field.m_label == sourceWidget || field.m_label->isAncestorOf(sourceWidget))) {
                clickedField = &field;
                break;
            }
        }
    }

    // Fallback: hit-test label geometries using local coordinates
    if (clickedField == nullptr) {
        const QPoint localPos = mapFromGlobal(globalPos);
        for (const LC_CoordinateField& field : m_coordinateFields) {
            if (field.m_label != nullptr && field.m_label->geometry().contains(localPos)) {
                clickedField = &field;
                break;
            }
        }
    }

    // LC_ERR << "LC_CoordinateWidgetBase::showContextMenu: targetField="
    // << (clickedField != nullptr ? clickedField->m_description : QStringLiteral("none"));

    if (clickedField != nullptr) {
        const QString labelText = clickedField->m_label->text();
        const RS_Vector vectorVal = (clickedField->m_getter != nullptr) ? clickedField->m_getter() : RS_Vector();

        QAction* copyAction = menu.addAction(tr("Copy %1").arg(clickedField->m_description));
        connect(copyAction, &QAction::triggered, this, [this, labelText]() {
            copyToClipboard(labelText);
        });

        QAction* insertAction = menu.addAction(tr("Insert %1 to Cmd").arg(clickedField->m_description));
        connect(insertAction, &QAction::triggered, this, [this, vectorVal, labelText]() {
            onInsertToCmd(vectorVal, labelText);
        });
    }
    else {
        for (const LC_CoordinateField& field : m_coordinateFields) {
            if (field.m_label != nullptr) {
                const QString labelText = field.m_label->text();
                QAction* copyAction = menu.addAction(tr("Copy %1").arg(field.m_description));
                connect(copyAction, &QAction::triggered, this, [this, labelText]() {
                    copyToClipboard(labelText);
                });
            }
        }

        if (!m_coordinateFields.isEmpty()) {
            menu.addSeparator();
            const LC_CoordinateField& primaryField = m_coordinateFields.first();
            if (primaryField.m_label != nullptr) {
                const QString primaryText = primaryField.m_label->text();
                const RS_Vector primaryVector = (primaryField.m_getter != nullptr) ? primaryField.m_getter() : RS_Vector();

                QAction* insertAction = menu.addAction(tr("Insert %1 to Cmd").arg(primaryField.m_description));
                connect(insertAction, &QAction::triggered, this, [this, primaryVector, primaryText]() {
                    onInsertToCmd(primaryVector, primaryText);
                });
            }
        }
    }

    menu.exec(globalPos);
}
