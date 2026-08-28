
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

#ifndef LC_INDEX_SETTINGS_PAGE_H
#define LC_INDEX_SETTINGS_PAGE_H

#include <QObject>
#include <QVBoxLayout>
#include <QLabel>
#include "lc_settings_page_interface.h"
#include "lc_settings_list_widget.h"

class LC_SettingsBannerWidget;

class LC_IndexSettingsPage : public QObject, public LC_SettingsPageInterface {
    Q_OBJECT
public:
    LC_IndexSettingsPage(const QString& displayName,
                            const QString& description,
                            QWidget* parentWidget = nullptr);
    ~LC_IndexSettingsPage() override = default;

    QString id() const override { return m_id; }
    void setId(const QString& id) override { m_id = id; }

    QString parentId() const override { return m_parentId; }
    void setParentId(const QString& parentId) override { m_parentId = parentId; }

    QString displayName() const override { return m_displayName; }

    QWidget* getEditingWidget() override { return m_mainWidget; }
    void loadSettings() override {}
    bool saveSettings() override { return true; }
    void resetDefaults() override {}

    bool isModified() const override { return false; }
    bool requiresRestart() const override { return false; }
    void setRestartRequired(bool required) override { Q_UNUSED(required); }
    bool validate(QString& outErrorMessage) override { Q_UNUSED(outErrorMessage); return true; }

    QList<LC_SearchTarget> searchTargets() const override;
    void registerSearchTarget(QWidget* target, const QString& text) override { Q_UNUSED(target); Q_UNUSED(text); }
    void highlightSearchPattern(const QString& pattern) override;
    void clearSearchHighlight() override;
    void setChildPages(const QList<LC_SettingsPageInterface*>& children) override;

    // --- Gating Methods ---
    void setGating(std::function<bool()> isGatedFunc,
                   std::function<QString()> messageFunc,
                   const QString& actionText = QString(),
                   std::function<void()> actionCallback = nullptr);

    bool isPageGated() const override;
    QString gatedMessage() const override;
    QString gatedActionText() const override { return m_gatedActionText; }
    std::function<void()> gatedActionCallback() const override { return m_gatedActionCallback; }

  signals:
        void navigateToPage(const QString& pageId);

private:
    QString m_id;
    QString m_parentId;
    QString m_displayName;
    QString m_description;

    std::function<bool()> m_isGatedFunc;
    std::function<QString()> m_gatedMessageFunc;
    QString m_gatedActionText;
    std::function<void()> m_gatedActionCallback;

    QWidget* m_mainWidget = nullptr;
    QVBoxLayout* m_mainLayout = nullptr;
    QLabel* m_descLabel = nullptr;
    LC_SettingsLinksWidget* m_linksWidget = nullptr;
};

#endif
