
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

#ifndef LC_STYLE_EDITOR_BASE_H
#define LC_STYLE_EDITOR_BASE_H

#include <QWidget>
#include <functional>

#include "lc_palette_editor_shared.h"
#include "lc_style_editor_interface.h"

class LC_UIStyleManager;
class LC_RepositoryBase;

class LC_StyleEditorBase : public QWidget, public LC_StyleEditorInterface {
    Q_OBJECT
    Q_INTERFACES(LC_StyleEditorInterface)
public:
    LC_StyleEditorBase(QWidget* parent,
                       LC_UIStyleManager* styleManager,
                       LC_RepositoryBase* repository)
        : QWidget(parent)
        , m_styleManager(styleManager)
        , m_repository(repository) {}

    ~LC_StyleEditorBase() override = default;

    bool deletePreset(const QString& key) override {
        return m_repository ? m_repository->removeByKey(key) : false;
    }

    bool loadPreset(const QString& key) override;
    bool savePresetAs(const QString& name, QString& outKey) override;

    QList<QPair<QString, QString>> getAvailablePresets() const override {
        return m_repository ? m_repository->getPresetChoices() : QList<QPair<QString, QString>>();
    }

    void setChangedCallback(std::function<void(bool isDirty)> callback) override {
        m_changedCallback = callback;
    }

protected:
    virtual bool doLoadPreset(const QString& key) = 0;
    virtual bool doSavePresetAs(const QString& name, QString& key) = 0;

    void notifyChanged(bool isDirty) {
        if (m_changedCallback) {
            m_changedCallback(isDirty);
        }
    }

    LC_UIStyleManager* m_styleManager;
    QString m_currentPresetKey;
    LC_RepositoryBase* m_repository;
    std::function<void(bool)> m_changedCallback;
};

#endif
