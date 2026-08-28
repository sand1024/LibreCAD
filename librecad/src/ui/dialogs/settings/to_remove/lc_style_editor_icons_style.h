
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

#ifndef LC_STYLE_EDITOR_ICONSSTYLE_H
#define LC_STYLE_EDITOR_FUSIONSKIN_H

#include "lc_style_editor_base.h"
#include "lc_palette_editor_shared.h"
#include "lc_icons_style_shared.h"
#include "lc_icon_colors_options.h"

class QComboBox;
class LC_ColorButton;
class QCheckBox;

namespace Ui {
    class LC_StyleEditorIconsStyle;
}

class LC_StyleEditorIconsStyle : public LC_StyleEditorBase {
    Q_OBJECT
public:
    LC_StyleEditorIconsStyle(QWidget* parent, LC_UIStyleManager* styleManager);
    ~LC_StyleEditorIconsStyle() override;

    QString getPresetCategoryName() const override { return tr("Icons Style"); }
    QString getFileExtension() const override { return ICON_STYLE_EXTENSION; }
    QWidget* getWidget() override { return this; }

    bool saveCurrentPreset() override;
    QString getActivePresetKey() const override;
    QList<QPair<QString, QString>> getAvailablePresets() const override;

    bool supportsLivePreview() const override { return false; }
    bool supportsAccessibilityCheck() const override { return true; }

    void applyTransientState(QWidget* previewWindow) const override;
    void applyCurrentPreset() override;
    void rollbackState() override;

private slots:
    void onControlChanged();
    void onVariantTabChanged(bool checked);
    void onAutoCalcStatesToggled(bool checked);
    void onAutoGenerateOppositeSchemePressed();
    void onHarmonizeSeedsPressed();
protected:
    bool doLoadPreset(const QString& key) override;
    bool doSavePresetAs(const QString& name, QString& key) override;
private:
    void setupConnections();
    void setupComboboxes() const;
    void setupStatesTableStructure();
    void setupAccentControls();
    void setupHarmonizationButton();

    void loadConfigToUi();
    void saveUiToConfig();
    void calculateAndApplyAutoStates();

    void loadSeedControl(const QString& colorStr, QCheckBox* chk, LC_ColorButton* btn, QComboBox* cb);
    QString getSeedValueFromUi(const QCheckBox* chk, const LC_ColorButton* btn, const QComboBox* cb) const;

    Ui::LC_StyleEditorIconsStyle* ui;
    IconStyleConfig          m_currentIconStyle;
    LC_IconColorsOptions     m_iconColorsOptions;

    bool m_blockSignals = false;
    bool m_currentVariantDark = false;
};

#endif
