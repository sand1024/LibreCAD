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

#ifndef LC_SETTINGS_PAGE_ADAPTER_H
#define LC_SETTINGS_PAGE_ADAPTER_H

#include <QObject>
#include <functional>
#include "lc_settings_page_interface.h"

/*
Example:
// Symmetrical Page Integration:
auto skinEditor = std::make_unique<LC_SkinEditor>(); // Implements LC_StyleEditorInterface

auto adapterPage = std::make_unique<LC_SettingsPageAdapter>(
    QObject::tr("UI Themes"),
    skinEditor->getWidget(),
    [editorPtr = skinEditor.get()]() { editorPtr->onInitialize(); },
    [editorPtr = skinEditor.get()]() { return editorPtr->saveCurrentPreset(); },
    [editorPtr = skinEditor.get()]() { return editorPtr->isModified(); }
);

// Register the adapter page directly into our standard registry!
reg->registerPage("application_preferences", "application.startup.ui_theme", "application.startup", [&]() {
    return std::move(adapterPage);
});

*/

class LC_SettingsPageAdapter : public QObject, public LC_SettingsPageInterface {
    Q_OBJECT public:
    // Pluggable Constructor: wraps any QWidget and delegates actions to callbacks
    LC_SettingsPageAdapter(const QString& displayName, QWidget* customWidget, const std::function<void()>& loadCallback = nullptr,
                           const std::function<bool()>& saveCallback = nullptr, const std::function<bool()>& modifiedCallback = nullptr,
                           QObject* parent = nullptr);

    ~LC_SettingsPageAdapter() override = default;

    // --- Page Interface Overrides ---
    QString id() const override {
        return m_id;
    }

    void setId(const QString& id) override {
        m_id = id;
    }

    QString parentId() const override {
        return m_parentId;
    }

    void setParentId(const QString& parentId) override {
        m_parentId = parentId;
    }

    QString displayName() const override {
        return m_displayName;
    }

    QWidget* settingEditingWidget() override {
        return m_widget;
    }

    void loadSettings() override;
    bool saveSettings() override;
    void resetDefaults() override;

    bool isModified() const override;

    bool requiresRestart() const override {
        return m_restartRequired;
    }

    void setRestartRequired(bool required) override {
        m_restartRequired = required;
    }

    bool validate(QString& outErrorMessage) override;

    QList<LC_SearchTarget> searchTargets() const override {
        return m_searchTargets;
    }

    void registerSearchTarget(QWidget* target, const QString& text) override;
    void highlightSearchPattern(const QString& pattern) override;
    void clearSearchHighlight() override;
    void autoIndexLabels() override;

private:
    QString m_id;
    QString m_parentId;
    QString m_displayName;
    QWidget* m_widget = nullptr;
    bool m_restartRequired = false;

    std::function<void()> m_loadCallback;
    std::function<bool()> m_saveCallback;
    std::function<bool()> m_modifiedCallback;

    QList<LC_SearchTarget> m_searchTargets;
};

#endif
