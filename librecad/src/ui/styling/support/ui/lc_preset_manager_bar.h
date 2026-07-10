
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

#ifndef LC_PRESET_MANAGER_BAR_H
#define LC_PRESET_MANAGER_BAR_H

#include <QWidget>

namespace Ui {
    class LC_PresetManagerBar;
}

class LC_PresetManagerBar : public QWidget {
    Q_OBJECT
public:
    explicit LC_PresetManagerBar(QWidget* parent = nullptr);
    ~LC_PresetManagerBar() override;

    void populatePresets(const QList<QPair<QString, QString>>& presets,
                         const QString& activeKey,
                         const QString& savedActiveKeyOnDisk);


    QString currentPresetKey() const;
    QString currentPresetName() const;
    void setCurrentPresetKey(const QString& key);

    void setLabelText(const QString& text);
    void setSaveButtonToolTip(const QString& tooltip);
    void setSaveAsButtonToolTip(const QString& tooltip);
    void setDeleteButtonToolTip(const QString& tooltip);
    void setApplyButtonToolTip(const QString& tooltip);

    void setSaveButtonEnabled(bool enabled);
    void setDeleteButtonEnabled(bool enabled);
    void setApplyButtonEnabled(bool enabled);

    void updateComboFonts(const QString& activeItemKey) const;
    void fillNamesList(QStringList& namesList) const;

    void updateComboFonts(const QString& activeItemKey);
signals:
        void presetSelected(const QString& name);
        void saveRequested();
        void saveAsRequested();
        void deleteRequested();
        void applyRequested();

private slots:
    void onComboIndexChanged(int index);

private:
    Ui::LC_PresetManagerBar* ui;
    int m_previousIndex = -1;
    bool m_blockSignals = false;
};

#endif
