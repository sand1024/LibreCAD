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

#include "lc_settings_banner_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QStyle>

LC_SettingsBannerWidget::LC_SettingsBannerWidget(QWidget* parent)
    : QWidget(parent) {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(10);

    m_lblIcon = new QLabel(this);
    m_lblIcon->setPixmap(style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(22, 22));
    layout->addWidget(m_lblIcon);

    m_lblMessage = new QLabel(this);
    m_lblMessage->setWordWrap(true);
    m_lblMessage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->addWidget(m_lblMessage);

    m_btnAction = new QPushButton(this);
    m_btnAction->setVisible(false);
    layout->addWidget(m_btnAction);

    connect(m_btnAction, &QPushButton::clicked, this, [this]() {
        if (m_actionCallback != nullptr) {
            // Copy locally to keep closure alive if callback clears the banner
            auto callbackCopy = m_actionCallback;
            callbackCopy();
        }
    });
}

void LC_SettingsBannerWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Frame using native palette colors (AlternateBase background and Mid border)
    painter.setPen(palette().color(QPalette::Mid));
    painter.setBrush(palette().color(QPalette::AlternateBase));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 4, 4);
}

void LC_SettingsBannerWidget::setBanner(const QString& message,
                                       const QString& actionText,
                                       std::function<void()> actionCallback) {
    m_lblMessage->setText(message);
    m_actionCallback = std::move(actionCallback);

    if (!actionText.isEmpty() && m_actionCallback) {
        m_btnAction->setText(actionText);
        m_btnAction->setToolTip(actionText);
        m_btnAction->setVisible(true);
    }
    else {
        m_btnAction->setVisible(false);
    }
}

void LC_SettingsBannerWidget::clearAction() {
    m_actionCallback = nullptr;
    m_btnAction->setVisible(false);
}
