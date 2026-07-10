
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
#ifndef LC_STYLE_EDITOR_METRICS_H
#define LC_STYLE_EDITOR_METRICS_H

#include "lc_style_editor_base.h"
#include "lc_palette_editor_shared.h"

namespace Ui {
    class LC_StyleEditorMetrics;
}

class LC_StyleEditorMetrics : public LC_StyleEditorBase {
    Q_OBJECT
public:
    LC_StyleEditorMetrics(QWidget* parent, LC_UIStyleManager* styleManager);
    ~LC_StyleEditorMetrics() override;

    QString getPresetCategoryName() const override { return tr("Metrics"); }
    QString getFileExtension() const override { return METRICS_EXTENSION; }
    QWidget* getWidget() override { return this; }


    bool saveCurrentPreset() override;
    QString getActivePresetKey() const override;
    QList<QPair<QString, QString>> getAvailablePresets() const override;

    void applyTransientState(QWidget* previewWindow) const override;
    void applyCurrentPreset() override;
    void rollbackState() override;

private slots:
    void onControlChanged();
    void onDensityPresetChanged(int index);
protected:
    bool doLoadPreset(const QString& key) override;
    bool doSavePresetAs(const QString& name, QString& key) override;

private:
    void setupConnections();
    void loadConfigToUi(const StyleMetricsConfig& config) const;
    StyleMetricsConfig getConfigFromUi() const;

    Ui::LC_StyleEditorMetrics* ui;
    StyleMetricsConfig m_currentConfig;
    bool m_blockSignals = false;
};

#endif
