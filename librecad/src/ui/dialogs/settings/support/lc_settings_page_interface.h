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

#ifndef LC_SETTINGS_PAGE_INTERFACE_H
#define LC_SETTINGS_PAGE_INTERFACE_H

#include <QList>
#include <QString>
#include <QWidget>
#include "lc_settings_types.h"

class LC_LivePreview {
public:
    virtual void updateLivePreview() = 0;
    virtual void updatePreviewForContentCategory(const QString& tag){};
    virtual void cleanupPreviewForContentCategory(const QString& tag){};
};

class LC_SettingsPageInterface {
public:
    virtual ~LC_SettingsPageInterface() = default;

    // --- Dynamic Routing Setters (Injected by Dialog/Registry) ---
    virtual QString id() const = 0;
    virtual void setId(const QString& id) = 0;

    virtual QString parentId() const = 0;
    virtual void setParentId(const QString& parentId) = 0;

    // --- Localized Identity ---
    virtual QString displayName() const = 0; // e.g. tr("Program Defaults")

    int sortWeight() const { return m_sortWeight;}
    void setSortWeight(int weight) {m_sortWeight = weight;}

    virtual QWidget* getEditingWidget() = 0; // fixme - sand - settings - rename to editingWidget
    virtual QWidget* getHeaderWidget() { return nullptr; }
    virtual QWidget* getPreviewWidget() { return nullptr; }
    virtual QWidget* getBottomWidget() { return nullptr; }
    virtual void updateLivePreview() {}
    virtual void onPageResized() {}
    virtual void onDialogShown() {}

    virtual void loadSettings() = 0;
    virtual bool saveSettings() = 0;
    virtual void resetDefaults() = 0;

    virtual bool isModified() const = 0;
    virtual bool requiresRestart() const = 0;
    virtual void setRestartRequired(bool required) = 0;
    virtual bool validate(QString& outErrorMessage) = 0;

    virtual void onAboutToShow() {}
    virtual void onAboutToHide() {}

    virtual QList<LC_SearchTarget> searchTargets() const = 0;
    virtual void registerSearchTarget(QWidget* target, const QString& text) = 0;
    virtual void highlightSearchPattern(const QString& pattern) = 0;
    virtual void clearSearchHighlight() = 0;
    virtual void autoIndexLabels() {}
    virtual bool acceptsSharedPreview(){return true;}

    virtual void setChildPages([[maybe_unused]]const QList<LC_SettingsPageInterface*>& children) {}
private:
    int m_sortWeight = 100;
};

#endif
