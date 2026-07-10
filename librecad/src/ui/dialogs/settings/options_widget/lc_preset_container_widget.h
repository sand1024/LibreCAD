
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

#ifndef LC_PRESET_CONTAINER_WIDGET_H
#define LC_PRESET_CONTAINER_WIDGET_H

#include <QWidget>

#include "lc_palette_color_utils.h"
#include "lc_palette_editor_shared.h"

class LC_PresetManagerBar;
class LC_StyleEditorInterface;
class LC_UIStyleManager;
class LC_SkinPreviewWindow;

namespace Ui {
    class LC_PresetContainerWidget;
}

class LC_PresetContainerWidget : public QWidget {
    Q_OBJECT
public:
    LC_PresetContainerWidget(QWidget* parent,
                             LC_UIStyleManager* styleManager,
                             LC_StyleEditorInterface* editor);
    ~LC_PresetContainerWidget() override;

    bool isDirty() const { return m_isDirty; }
    QString currentPresetKey() const { return m_currentKey; }

    // Pluggable lifecycle triggers
    void initialize(const QString& initialKey);
    void populatePresetBar();
    void updatePresetBarStates();
    void applyCurrentPreset();
    void rollback();

    // Isolated preview controller
    void setPreviewVisible(bool visible);
    bool isPreviewVisible() const;
    LC_PaletteColorUtils::CVDType activeCvdType() const;

signals:
    void dirtinessChanged(bool isDirty);
    void activePresetChanged(const QString& displayName);

private slots:
    void onPresetSelected(const QString& key);
    void onSaveRequested();
    void onSaveAsRequested();
    void onDeleteRequested();
    void onApplyRequested();
    void onRevertClicked();
    void onResetClicked();
    void onShowPreviewToggled(bool checked);
    void onDisablePreviewToggled(bool checked);
    void onCvdSimulatorChanged(int index);

private:
    void updatePresetBar();
    void setDirty(bool dirty);
    void updateLivePreview() const;

    LC_UIStyleManager*        m_styleManager;
    LC_StyleEditorInterface*  m_editor;
    LC_PresetManagerBar*      m_presetBar;
    QWidget*                  m_editorContainer;

    mutable LC_SkinPreviewWindow* m_previewWindow = nullptr;

    bool m_isDirty = false;
    bool m_blockSignals = false;
    QString m_currentKey;
    QString m_originalActiveKey;

    Ui::LC_PresetContainerWidget* ui;
};

#endif
