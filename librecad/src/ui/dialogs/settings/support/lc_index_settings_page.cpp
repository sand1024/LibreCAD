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
#include "lc_index_settings_page.h"
#include <QRegularExpression>

#include "lc_highlight_overlay.h"
#include "lc_settings_list_widget.h"


LC_IndexSettingsPage::LC_IndexSettingsPage(const QString& displayName,
                                           const QString& description,
                                           QWidget* parentWidget)
    : QObject(parentWidget)
    , m_displayName(displayName)
    , m_description(description) {

    m_mainWidget = new QWidget(parentWidget);
    m_mainLayout = new QVBoxLayout(m_mainWidget);
    m_mainLayout->setContentsMargins(16, 16, 16, 16); // fixme - sand check this, how it's related do metrics?
    m_mainLayout->setSpacing(12);

    m_descLabel = new QLabel(m_description, m_mainWidget);
    m_descLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_descLabel->setWordWrap(true);
    m_mainLayout->addWidget(m_descLabel);
}

// context around LC_IndexSettingsPage::setChildPages inside lc_index_settings_page.cpp:
void LC_IndexSettingsPage::setChildPages(const QList<LC_SettingsPageInterface*>& children) {
    if (m_linksWidget != nullptr) {
        m_mainLayout->removeWidget(m_linksWidget);
        m_linksWidget->deleteLater();
        m_linksWidget = nullptr;
    }

    if (children.isEmpty()) {
        return;
    }

    QList<QPair<QString, QString>> links;
    for (const auto* child : children) {
        // Symmetrical Update: Use child->displayName() to register category index targets [4.2]
        links.append({child->id(), child->displayName()});
    }

    m_linksWidget = new LC_SettingsLinksWidget(links, m_mainWidget);
    m_mainLayout->addWidget(m_linksWidget);
    m_mainLayout->addStretch();

    connect(m_linksWidget, &LC_SettingsLinksWidget::pageSelected,
            this, &LC_IndexSettingsPage::navigateToPage);
}

QList<LC_SearchTarget> LC_IndexSettingsPage::searchTargets() const {
    QList<LC_SearchTarget> targets;
    if (m_descLabel != nullptr) {
        targets.append({m_description, m_descLabel});
    }
    if (m_linksWidget != nullptr) {
        auto buttons = m_linksWidget->findChildren<LC_HyperlinkButton*>();
        for (auto* btn : buttons) {
            const QString original = btn->property("originalText").toString();
            targets.append({original, btn});
        }
    }
    return targets;
}


void LC_IndexSettingsPage::highlightSearchPattern(const QString& pattern) {
    clearSearchHighlight();
    if (pattern.isEmpty()) {
        return;
    }

    if (m_descLabel && m_description.contains(pattern, Qt::CaseInsensitive)) {
        new LC_HighlightOverlay(m_descLabel);
    }

    if (m_linksWidget != nullptr) {
        auto buttons = m_linksWidget->findChildren<LC_HyperlinkButton*>();
        for (auto* btn : buttons) {
            QString original = btn->property("originalText").toString();
            if (original.contains(pattern, Qt::CaseInsensitive)) {
                new LC_HighlightOverlay(btn);
            }
        }
    }
}

void LC_IndexSettingsPage::clearSearchHighlight() {
    if (m_descLabel != nullptr) {
        const auto childrenList = m_descLabel->children();
        for (QObject* child : childrenList) {
            if (auto* o = dynamic_cast<LC_HighlightOverlay*>(child)) {
                o->deleteLater();
            }
        }
    }

    if (m_linksWidget != nullptr) {
        auto buttons = m_linksWidget->findChildren<LC_HyperlinkButton*>();
        for (const auto* btn : buttons) {
            const auto childrenList = btn->children();
            for (QObject* child : childrenList) {
                if (auto* o = dynamic_cast<LC_HighlightOverlay*>(child)) {
                    o->deleteLater();
                }
            }
        }
    }
}
