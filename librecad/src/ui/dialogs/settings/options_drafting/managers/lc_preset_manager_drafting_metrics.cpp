/*******************************************************************************
 *
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

#include "lc_preset_manager_drafting_metrics.h"

#include <QJsonObject>
#include <QJsonValue>
#include <vector>

#include "lc_drafting_metrics_defaults.h"
#include "lc_drafting_preview_bottom_bar.h"
#include "lc_preset_file_io.h"
#include "lc_settings_appearance.h"
#include "lc_settings_app_styling.h"
#include "lc_settings_grid.h"
#include "lc_settings_info_overlay_cursor.h"
#include "lc_settings_relative_position_assistant.h"
#include "lc_settings_snap.h"
#include "lc_settings_snap_visual.h"
#include "rs_settings.h"

namespace {
    const QString METRICS_HIDPI_KEY = "__hidpi__";

    std::vector<LC_DraftingMetricsMapEntry> g_draftingMetricsMappingTable;

    void mapMetrics(std::initializer_list<LC_DraftingMetricsMapEntry> list) {
        for (const auto& entry : list) {
            g_draftingMetricsMappingTable.push_back(entry);
        }
    }

    void initMetricsMappingTable() {
        if (!g_draftingMetricsMappingTable.empty()) {
            return;
        }

        using namespace CFG_Appearance;
        using namespace CFG_Grid;
        using namespace CFG_Snap;
        using namespace CFG_VisualSnap;
        using namespace CFG_InfoOverlayCursor;
        using namespace CFG_RelativePositionAssistant;

        mapMetrics({
            // Interactive Handles & Markers
            o_EntityHandleSize,
            o_ZeroMarkerSize,
            o_ZeroMarkerFontSize,
            o_ZeroShortAxisMarkSize,
            o_RelZeroMarkerRadius,
            o_AngleSnapMarkerSize,
            o_DraftMarkerFontSize,

            // Snap & Visual Snap Indicators
            o_IndicatorShapeSize,
            o_IndicatorScreenLinesLineWidth,
            o_VSProjectedSnapSize,
            o_VSHighlightedVertexSize,
            o_VSVertexSize,
            o_VSGuidingEntitiesLineWidth,
            o_VSGuidingPointsLineWidth,
            o_VSGuidingEntityLabelFontSize,
            o_VSGuidingLabelOffsetPx,
            o_VSGuidingEntitiesCatchDistance,
            o_AdvSnapEntityCatchRange,

            // Screen Line Widths & Grid Points
            o_OverlaysScreenLineWidth,
            o_IndicatorScreenLinesLineWidth,
            o_MinGridSpacing,
            o_MetaGridPointsLineWidth,
            o_MetaGridLinesLineWidth,
            o_GridLinesLineWidth,
            o_GridPointsPointSize,

            // Overlay Fonts & Offsets
            o_FontSize,
            o_OffsetFromCursor,
            o_AssistantFontSize
        });
    }

    QVariant getJsonValue(const QJsonObject& obj, const QString& path) {
        const QStringList segments = path.split('.');
        QJsonValue current = obj;
        for (const QString& segment : segments) {
            if (current.isObject()) {
                current = current.toObject().value(segment);
            }
            else {
                return QVariant();
            }
        }
        return current.toVariant();
    }

    void setJsonValue(QJsonObject& obj, const QString& path, const QVariant& value) {
        const QStringList segments = path.split('.');
        QList<QJsonObject> objects;
        objects.append(obj);

        for (int i = 0; i < segments.size() - 1; ++i) {
            const QJsonObject child = objects.last().value(segments[i]).toObject();
            objects.append(child);
        }

        objects.last().insert(segments.last(), QJsonValue::fromVariant(value));

        for (int i = segments.size() - 2; i >= 0; --i) {
            QJsonObject parent = objects[i];
            parent.insert(segments[i], objects[i + 1]);
            objects[i] = parent;
        }
        obj = objects.first();
    }
}

LC_PresetManagerDraftingMetrics::LC_PresetManagerDraftingMetrics(LC_RepositoryDraftingMetrics* repository,
                                                               QObject* parent,
                                                               QWidget* previewWidget)
    : LC_PresetManagerConfigBase<LC_DraftingMetricsConfig, LC_RepositoryDraftingMetrics>(
          repository,
          CFG_AppStyling::o_ActiveDraftingMetricsName,
          parent)
    , m_previewWidget(previewWidget) {
    loadPreset(m_activeKey);
}

LC_PresetManagerDraftingMetrics::~LC_PresetManagerDraftingMetrics() = default;

LC_PresetManagerUIStrings LC_PresetManagerDraftingMetrics::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Standard (96–120 DPI)");
    s.labelText = tr("Sizing profile:");
    s.selectToolTip = tr("Select a display sizing and density profile.");
    s.saveToolTip = tr("Save current sizing profile.");
    s.saveAsToolTip = tr("Save current sizes as a new custom profile.");
    s.deleteToolTip = tr("Permanently delete the selected custom sizing profile.");
    s.applyToolTip = tr("Apply the active sizing profile to the viewport.");
    s.revertToolTip = tr("Discard modifications and reload the sizing profile as saved on disk.");
    s.saveAsDialogTitle = tr("Save Sizing Profile As");
    s.saveAsDialogLabel = tr("Enter unique sizing profile name:");
    s.defaultNewPresetName = tr("Custom Sizing");
    s.deleteConfirmTitle = tr("Delete Sizing Profile");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the sizing profile '%1'?");
    s.exportDialogTitle = tr("Export Sizing Profile");
    s.importDialogTitle = tr("Import Sizing Profile");
    s.presetFileFilter = tr("LibreCAD drafting sizing profile files (*%1);;All Files (*.*)")
                             .arg(m_repository != nullptr ? m_repository->getFileExtension() : QString(".metrics.json"));

    s.defaultReadOnlyMessage = tr("Built-in sizing profiles are read-only templates. To customize marker and handle sizes, duplicate it as a custom profile.");
    s.duplicateActionText = tr("Duplicate Profile...");
    s.saveModifiedPromptTitle = tr("Save Sizing Profile");
    s.saveModifiedPromptMessage = tr("You have unsaved changes to sizing profile '%1'.\n\nDo you want to save them before applying?");
    s.discardConfirmTitle = tr("Discard Changes");
    s.discardConfirmMessage = tr("You have unsaved modifications to sizing profile '%1'.\n\nAre you sure you want to discard these changes?");
    return s;
}

bool LC_PresetManagerDraftingMetrics::isDefaultPreset(const QString& key) const {
    return (key.isEmpty() || key == CFG_AppState::DEFAULT_THEME_KEY || key == METRICS_HIDPI_KEY);
}

void LC_PresetManagerDraftingMetrics::prependDefaultPresets(QList<QPair<QString, QString>>& choices) const {
    choices.prepend(qMakePair(tr("Standard (96–120 DPI) - Default"), CFG_AppState::DEFAULT_THEME_KEY));
    choices.prepend(qMakePair(tr("HiDPI / 4K (150–200%) - Default"), QString(METRICS_HIDPI_KEY)));
}

bool LC_PresetManagerDraftingMetrics::resetToDefaults(const QString& key) {
    LC_DraftingMetricsDefaults::applyStandardDefaults();
    setActivePresetKey(key);
    CFG_AppStyling::o_ActiveDraftingMetricsName = key;

    if (m_resetCallback != nullptr) {
        m_resetCallback();
    }

    setDirtyState(false);
    return true;
}

bool LC_PresetManagerDraftingMetrics::resetToHiDpiDefaults(const QString& key) {
    LC_DraftingMetricsDefaults::applyHiDpiDefaults();
    setActivePresetKey(key);
    CFG_AppStyling::o_ActiveDraftingMetricsName = key;

    if (m_resetCallback != nullptr) {
        m_resetCallback();
    }

    setDirtyState(false);
    return true;
}

bool LC_PresetManagerDraftingMetrics::loadPreset(const QString& key) {
    if (key == CFG_AppState::DEFAULT_THEME_KEY || key.isEmpty()) {
        return resetToDefaults(CFG_AppState::DEFAULT_THEME_KEY);
    }
    if (key == METRICS_HIDPI_KEY) {
        return resetToHiDpiDefaults(METRICS_HIDPI_KEY);
    }

    if (m_repository != nullptr && m_repository->loadByKey(key, m_workingConfig)) {
        applyMetricsJson(m_workingConfig.rootObject);
        setActivePresetKey(key);
        m_originalActiveKey = key;
        CFG_AppStyling::o_ActiveDraftingMetricsName.set(key);
        setDirtyState(false);
        return true;
    }

    // Recovery path: file is missing from disk, but values exist in settings!
    m_workingConfig.name = key;
    m_workingConfig.rootObject = serializeCurrentSettings();
    setActivePresetKey(key);
    m_originalActiveKey = key;
    CFG_AppStyling::o_ActiveDraftingMetricsName.set(key);
    setDirtyState(true);
    return true;
}

bool LC_PresetManagerDraftingMetrics::isPresetModified() {
    return m_isDirty;
}

void LC_PresetManagerDraftingMetrics::prepareWorkingConfigForSave() {
    m_workingConfig.rootObject = serializeCurrentSettings();
}

void LC_PresetManagerDraftingMetrics::onPostSavePreset(const QString& outKey) {
    CFG_AppStyling::o_ActiveDraftingMetricsName = outKey;
}

void LC_PresetManagerDraftingMetrics::applyActiveConfigToSystem(const QString& activeKey) {
    CFG_AppStyling::o_ActiveDraftingMetricsName = activeKey;
}

QJsonObject LC_PresetManagerDraftingMetrics::serializeCurrentSettings() const {
    QJsonObject root;
    initMetricsMappingTable();
    for (const auto& entry : g_draftingMetricsMappingTable) {
        if (entry.setting == nullptr) {
            continue;
        }

        QVariant val = entry.setting->getVariant();
        if (!val.isValid()) {
            val = entry.setting->defaultVariant();
        }
        setJsonValue(root, entry.jsonPath, val);
    }
    return root;
}

void LC_PresetManagerDraftingMetrics::applyMetricsJson(const QJsonObject& obj) {
    initMetricsMappingTable();
    for (const auto& entry : g_draftingMetricsMappingTable) {
        if (entry.setting == nullptr) {
            continue;
        }

        const QVariant val = getJsonValue(obj, entry.jsonPath);
        if (val.isValid()) {
            entry.setting->setVariant(val);
        }
    }
    RS_Settings::instance()->emitOptionsChanged();
}

bool LC_PresetManagerDraftingMetrics::importPresetFromFile(const QString& filePath, [[maybe_unused]] QWidget* parent) {
    clearLastError();
    QJsonObject root;
    const LC_PresetError err = LC_PresetFileIO::readJsonFile(filePath, root);
    if (!err.isOk()) {
        setLastError(err);
        return false;
    }

    if (m_repository == nullptr || !m_repository->isCompatible(root)) {
        setLastError(LC_PresetError::fromCode(
            LC_PresetErrorCode::IncompatibleFormat,
            tr("File '%1' is not a valid sizing profile preset.").arg(filePath)
        ));
        return false;
    }

    applyMetricsJson(root);
    setDirtyState(true);
    return true;
}

QWidget* LC_PresetManagerDraftingMetrics::getSharedPreviewWidget()  {
    return m_previewWidget;
}

QWidget* LC_PresetManagerDraftingMetrics::getSharedBottomWidget()  {
    return m_bottomBar;
}

bool LC_PresetManagerDraftingMetrics::isPreviewVisible() const {
    if (m_bottomBar != nullptr) {
        return m_bottomBar->isPreviewVisible();
    }
    return true;
}
