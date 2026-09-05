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

#include "lc_shortcut_search_popup.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "lc_shortcutinfo.h"

LC_ShortcutSearchPopup::LC_ShortcutSearchPopup(QWidget* parent)
    : QFrame(parent, Qt::Popup | Qt::FramelessWindowHint) {
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Raised);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setFocusPolicy(Qt::StrongFocus);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 8, 10, 8);
    mainLayout->setSpacing(6);

    auto* lblPrompt = new QLabel(tr("Press shortcut keys to search:"), this);
    lblPrompt->setStyleSheet("font-weight: bold;");
    mainLayout->addWidget(lblPrompt);

    m_lblSequence = new QLabel(tr("<None>"), this);
    m_lblSequence->setAlignment(Qt::AlignCenter);
    m_lblSequence->setStyleSheet("border: 1px solid palette(mid); border-radius: 3px; padding: 6px; font-size: 13px; background: palette(base);");
    mainLayout->addWidget(m_lblSequence);

    auto* btnLayout = new QHBoxLayout();
    m_btnClear = new QPushButton(tr("Clear Filter"), this);
    btnLayout->addWidget(m_btnClear);

    auto* btnClose = new QPushButton(tr("Close"), this);
    btnLayout->addWidget(btnClose);

    mainLayout->addLayout(btnLayout);

    connect(m_btnClear, &QPushButton::clicked, this, [this]() {
        resetSearch();
        hide();
    });

    connect(btnClose, &QPushButton::clicked, this, &QWidget::hide);
}

void LC_ShortcutSearchPopup::resetSearch() {
    m_lblSequence->setText(tr("<None>"));
    emit shortcutFilterChanged(QKeySequence());
}

void LC_ShortcutSearchPopup::showEvent(QShowEvent* event) {
    QFrame::showEvent(event);
    setFocus();
    qApp->installEventFilter(this);
}

void LC_ShortcutSearchPopup::hideEvent(QHideEvent* event) {
    qApp->removeEventFilter(this);
    QFrame::hideEvent(event);
}

bool LC_ShortcutSearchPopup::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::ShortcutOverride) {
        // Tell Qt this keystroke belongs to our input; prevents Ctrl+S/Ctrl+F from firing actions
        event->accept();
        return true;
    }

    if (event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);
        processKeyEvent(keyEvent);
        return true; // Consume the key completely
    }

    return QFrame::eventFilter(obj, event);
}

void LC_ShortcutSearchPopup::processKeyEvent(QKeyEvent* event) {
    const int key = event->key();

    if (key == Qt::Key_Escape) {
        hide();
        event->accept();
        return;
    }

    // Ignore lone modifier presses
    if (key == Qt::Key_Control || key == Qt::Key_Shift ||
        key == Qt::Key_Meta    || key == Qt::Key_Alt) {
        event->accept();
        return;
    }

    const int combinedKey = key | LC_ShortcutInfo::translateModifiers(event->modifiers(), event->text());
    const QKeySequence seq(combinedKey);

    m_lblSequence->setText(seq.toString(QKeySequence::NativeText));
    emit shortcutFilterChanged(seq);
    event->accept();
}
