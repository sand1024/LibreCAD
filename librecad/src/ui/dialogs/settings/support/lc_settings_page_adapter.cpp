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


#include "lc_settings_page_adapter.h"

#include "lc_settings_page_adapter.h"
#include "lc_highlight_overlay.h"
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>

LC_SettingsPageAdapter::LC_SettingsPageAdapter(const QString& displayName, QWidget* customWidget, const std::function<void()>& loadCallback,
                                               const std::function<bool()>& saveCallback, const std::function<bool()>& modifiedCallback,
                                               QObject* parent)
    : QObject(parent)
    , m_displayName(displayName)
    , m_widget(customWidget)
    , m_loadCallback(loadCallback)
    , m_saveCallback(saveCallback)
    , m_modifiedCallback(modifiedCallback) {}

void LC_SettingsPageAdapter::loadSettings() {
    if (m_loadCallback) {
        m_loadCallback();
    }
}

bool LC_SettingsPageAdapter::saveSettings() {
    if (m_saveCallback) {
        return m_saveCallback();
    }
    return true;
}

void LC_SettingsPageAdapter::resetDefaults() {
    // Left open for procedural subclass defaults resets
}

bool LC_SettingsPageAdapter::isModified() const {
    if (m_modifiedCallback) {
        return m_modifiedCallback();
    }
    return false;
}

bool LC_SettingsPageAdapter::validate(QString& outErrorMessage) {
    Q_UNUSED(outErrorMessage);
    return true;
}

void LC_SettingsPageAdapter::registerSearchTarget(QWidget* target, const QString& text) {
    if (target != nullptr) {
        m_searchTargets.append({text, target});
    }
}

void LC_SettingsPageAdapter::autoIndexLabels() {
    if (m_widget == nullptr) {
        return;
    }
    m_searchTargets.clear();

    // Symmetrical Widget Tree Traversal: introspects and indexes any third-party layout
    auto labels = m_widget->findChildren<QLabel*>();
    for (auto* label : labels) {
        if (!label->text().isEmpty()) {
            QWidget* target = label->buddy() ? label->buddy() : label;
            registerSearchTarget(target, label->text());
        }
    }

    auto checkBoxes = m_widget->findChildren<QCheckBox*>();
    for (auto* cb : checkBoxes) {
        if (!cb->text().isEmpty()) {
            registerSearchTarget(cb, cb->text());
        }
    }

    auto radioButtons = m_widget->findChildren<QRadioButton*>();
    for (auto* rb : radioButtons) {
        if (!rb->text().isEmpty()) {
            registerSearchTarget(rb, rb->text());
        }
    }

    auto groupBoxes = m_widget->findChildren<QGroupBox*>();
    for (auto* gb : groupBoxes) {
        if (!gb->title().isEmpty()) {
            registerSearchTarget(gb, gb->title());
        }
    }
}

void LC_SettingsPageAdapter::highlightSearchPattern(const QString& pattern) {
    clearSearchHighlight();
    if (pattern.isEmpty()) {
        return;
    }

    for (const auto& target : m_searchTargets) {
        if (!target.targetWidget) {
            continue;
        }
        if (target.originalText.contains(pattern, Qt::CaseInsensitive)) {
            new LC_HighlightOverlay(target.targetWidget);
        }
    }
}

void LC_SettingsPageAdapter::clearSearchHighlight() {
    for (const auto& target : m_searchTargets) {
        if (target.targetWidget == nullptr) {
            continue;
        }

        const auto childrenList = target.targetWidget->children();
        for (QObject* child : childrenList) {
            if (auto* overlay = dynamic_cast<LC_HighlightOverlay*>(child)) {
                overlay->deleteLater();
            }
        }
    }
}
