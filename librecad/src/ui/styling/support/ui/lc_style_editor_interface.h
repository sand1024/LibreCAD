
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

#ifndef LC_STYLE_EDITOR_INTERFACE_H
#define LC_STYLE_EDITOR_INTERFACE_H

#include <QString>
#include <QList>
#include <QList>
#include <QWidget>
#include <functional>

class LC_StyleEditorInterface {
public:
    virtual ~LC_StyleEditorInterface() = default;

    // --- Part 1: Metadata Hooks ---
    virtual QString getPresetCategoryName() const = 0;
    virtual QString getFileExtension() const = 0;
    virtual QWidget* getWidget() = 0;


    virtual bool loadPreset(const QString& key) = 0;
    virtual bool saveCurrentPreset() = 0;
    virtual bool savePresetAs(const QString& name, QString& outKey) = 0;
    virtual bool deletePreset(const QString& key) = 0;
    virtual QList<QPair<QString, QString>> getAvailablePresets() const = 0;
    virtual QString getActivePresetKey() const = 0;

    virtual bool supportsLivePreview() const { return true; }
    virtual bool supportsAccessibilityCheck() const { return false; }
    virtual void applyTransientState(QWidget* previewWindow) const = 0;


    virtual void onInitialize() {}
    virtual void onAboutToShow() {}
    virtual void onAboutToHide() {}
    virtual bool validateInput(QString& outErrorMessage) { Q_UNUSED(outErrorMessage); return true; }
    virtual void applyCurrentPreset() = 0;
    virtual void rollbackState() = 0;


    virtual void setChangedCallback(std::function<void(bool isDirty)> callback) = 0;
};

Q_DECLARE_INTERFACE(LC_StyleEditorInterface, "org.librecad.LC_StyleEditorInterface")
#endif
