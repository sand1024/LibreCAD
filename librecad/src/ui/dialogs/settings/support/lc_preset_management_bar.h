
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

#ifndef LC_PRESET_MANAGEMENT_BAR_H
#define LC_PRESET_MANAGEMENT_BAR_H

#include <QWidget>
#include <QComboBox>
#include "lc_preset_manager_interface.h"

namespace Ui {
    class LC_PresetManagementBar;
}

class LC_PresetManagementBar : public QWidget {
    Q_OBJECT
public:
    explicit LC_PresetManagementBar(QWidget* parent = nullptr);
    ~LC_PresetManagementBar() override;

    void bindToManager(LC_PresetManagerInterface* manager);

    void populatePresets(const QList<QPair<QString, QString>>& presets,
                         const QString& activeKey,
                         const QString& savedActiveKeyOnDisk);

    QString currentPresetKey() const;
    QString currentPresetName() const;
    void setCurrentPresetKey(const QString& key);
    void setDirty(bool isDirty);

private slots:
    void onComboIndexChanged(int index);

signals:
    void presetSelected(const QString& name);

private:
    void updateComboFonts(const QString& activeItemKey);
    void updateButtons();

    Ui::LC_PresetManagementBar* ui;
    LC_PresetManagerInterface* m_manager = nullptr;
    int m_previousIndex = -1;
    bool m_blockSignals = false;
    bool m_isDirty = false;
};


#endif
