
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

#ifndef LC_STYLE_EDITOR_FUSIONSKIN_H
#define LC_STYLE_EDITOR_FUSIONSKIN_H

#include "lc_style_editor_base.h"
#include "lc_palette_editor_shared.h"

namespace Ui {
    class LC_StyleEditorFusionSkin;
}

class LC_StyleEditorFusionSkin : public LC_StyleEditorBase {
    Q_OBJECT
public:
    LC_StyleEditorFusionSkin(QWidget* parent, LC_UIStyleManager* styleManager);
    ~LC_StyleEditorFusionSkin() override;

    // --- LC_StyleEditorInterface Overrides ---
    QString getPresetCategoryName() const override { return tr("Skins"); }
    QString getFileExtension() const override { return SKIN_EXTENSION; }
    QWidget* getWidget() override { return this; }

    bool saveCurrentPreset() override;

    QString getActivePresetKey() const override;
    QList<QPair<QString, QString>> getAvailablePresets() const override;

    void applyTransientState(QWidget* previewWindow) const override;
    void applyCurrentPreset() override;
    void rollbackState() override;
    bool supportsAccessibilityCheck() const override { return true; }

private slots:
    void onControlChanged();
    void onVariantTabChanged(bool checked);
    void onAutoCalc3DToggled(bool checked);
    void onBevelSeedChanged(int index);
    void onUseThemeIconsToggled(bool checked);
    void onArchetypeIndexChanged(int index);
    void onDecorationIndexChanged(int index);
    void onGenerateHarmonizedTheme();
    void onGenerateTwoColorTheme();
    void onGenerateHighContrastTheme();
    void updateGroupBoxUiState();
    void updateSplitterUiState();
    void updateToolButtonUiState();
    void updateSegmentedButtonsUiState();

protected:
    bool doLoadPreset(const QString& key) override;
    bool doSavePresetAs(const QString& name, QString& key) override;
private:
    void setupConnections();
    void setupComboboxes();
    void setupTablesStructure();
    void setupGeneratorMenu();

    void loadConfigToUi(bool isDarkMode);
    void saveUiToConfig(bool isDarkMode);
    void calculateProceduralBevels(bool isDarkMode);

    QString getArchetypeDescription(StyleArchetype archetype) const;
    QString getDecorationDescription(BoxDecoration dec) const;
    QString getRoleTooltip(QPalette::ColorRole role) const;


    Ui::LC_StyleEditorFusionSkin* ui;
    SkinConfig m_currentConfig;

    bool m_blockSignals = false;
    bool m_currentVariantDark = false;
};

#endif
