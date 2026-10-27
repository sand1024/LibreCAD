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

#include "lc_preset_manager_drafting_theme.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include "lc_drafting_preview_bottom_bar.h"
#include "lc_preset_file_io.h"
#include "lc_settings_appearance.h"
#include "lc_settings_app_styling.h"
#include "lc_settings_colors.h"
#include "lc_settings_grid.h"
#include "lc_settings_info_overlay_cursor.h"
#include "lc_settings_relative_position_assistant.h"
#include "lc_settings_snap.h"
#include "lc_settings_snap_visual.h"
#include "rs_settings.h"

namespace {
    std::vector<LC_ViewportThemeMapEntry> g_draftingThemeMappingTable;

    void mapSettings(std::initializer_list<LC_ViewportThemeMapEntry> list) {
        for (const auto& s : list) {
            g_draftingThemeMappingTable.push_back(s);
        }
    }

    void initMappingTable() {
        if (!g_draftingThemeMappingTable.empty()) {
            return;
        }

        using namespace CFG_Colors;
        using namespace CFG_Appearance;
        using namespace CFG_Grid;
        using namespace CFG_Snap;
        using namespace CFG_VisualSnap;
        using namespace CFG_InfoOverlayCursor;
        using namespace CFG_RelativePositionAssistant;

        mapSettings({
            // Canvas & Entities
            o_BackgroundColor,
            o_ForegroundBWColor,
            o_SelectedEntityColor,
            o_HighlightedEntityColor,
            o_DraftModeMarkerColor,
            // Grid Display
            o_GridPointsColor,
            o_MetaGridColor,
            o_GridLinesColor,
            o_MetaGridLinesColor,
            o_GridLinesLineType,
            o_metaGridLinesLineType,
            o_MetaGridPointsLineType,
            o_GridType, o_MetaGridEvery,
            // Coordinate System & Markers
            o_RelativeZeroColor,
            o_XAxisColor,
            o_YAxisColor,
            o_ZeroMarkerFontName,
            o_DraftMarkerFontName,
            o_AnglesBasisDirectionIndicatorColor,
            o_AnglesBasisAngleRayColor,
            o_ExtendAxisLines,
            o_ExtendModeXAxis,
            o_ExtendModeYAxis,
            // Snapping Indicators
            o_SnapIndicator,
            o_SnapIndicatorLines,
            o_IndicatorDrawLines,
            o_IndicatorDrawShape,
            o_IndicatorLinesType,
            o_IndicatorShapeType,
            o_IndicatorLinesLineType,
            // Visual Snap Guides
            o_VisualSnapGuideEntitiesColor,
            o_VisualSnapVertexesColor,
            o_VisualSnapProjectedSnapColor,
            o_VisualSnapDocumentEntitiesColor,
            o_VSGuidingEntitiesLineType,
            o_VSActiveGuidingEntityLineType,
            o_VSDocGuidingEntitiesLineType,
            o_VSGuidingEntitiesShowLabels,
            // Selection & Overlays
            o_OverlayBoxLineColor,
            o_OverlayBoxFillColor,
            o_OverlayBoxLineInvertedColor,
            o_OverlayBoxFillInvertedColor,
            o_OverlayBoxTransparency,
            o_SelectionOverlayLineType,
            o_SelectionOverlayInvertedLineType,
            o_SelectionPickBoxColor,
            o_SelectionPickBoxLinesSizeType,

            // Cursor & Assistant
            o_InfoOverlayAbsoluteCoordinatesColor,
            o_InfoOverlayRelativeColor,
            o_InfoOverlayPromptColor,
            o_InfoOverlaySnapInfoColor,
            o_RelativePositionAssistantBackgroundColor,
            o_RelativePositionAssistantTextColor,
            o_FontName,
            o_AssistantFontName,
            // selection and highlight
            o_SelectedEntityLineType,
            o_HighlightedEntityLineType,
            // Handles & Preview
            o_EntityHandleStartColor,
            o_EntityHandleInternalColor,
            o_EntityHandleEndColor,
            o_PreviewEntitiesColor,
            o_PreviewReferencesColor,
            o_PreviewReferencesHighlightColor,
            o_PreviewRefPointType,
            o_PreviewRefPointSize,
            o_MaxPreview,
            o_HighlightedEntityColor
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

LC_PresetManagerDraftingTheme::LC_PresetManagerDraftingTheme(LC_RepositoryDraftingTheme* repository,
                                                           QObject* parent,
                                                           QWidget* previewWidget)
    : LC_PresetManagerConfigBase<LC_ViewportThemeConfig, LC_RepositoryDraftingTheme>(
          repository,
          CFG_AppStyling::o_ActiveDraftingThemeName,
          parent)
    , m_previewWidget(previewWidget) {
    loadPreset(m_activeKey);
}

LC_PresetManagerDraftingTheme::~LC_PresetManagerDraftingTheme()  = default;

LC_PresetManagerUIStrings LC_PresetManagerDraftingTheme::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Default Theme");
    s.labelText = tr("Visual theme:");
    s.selectToolTip = tr("Select a visual drafting theme.");
    s.saveToolTip = tr("Save current visual theme.");
    s.saveAsToolTip = tr("Save current visual theme as a new custom theme.");
    s.deleteToolTip = tr("Permanently delete the selected custom visual theme.");
    s.applyToolTip = tr("Apply the active visual theme to the drawing canvas.");
    s.revertToolTip = tr("Discard modifications and reload the visual theme as saved on disk.");
    s.saveAsDialogTitle = tr("Save Visual Theme As");
    s.saveAsDialogLabel = tr("Enter unique visual theme name:");
    s.defaultNewPresetName = tr("Custom Theme");
    s.deleteConfirmTitle = tr("Delete Theme");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the theme '%1'?");
    s.exportDialogTitle = tr("Export Visual Theme");
    s.importDialogTitle = tr("Import Visual Theme");
    s.presetFileFilter = tr("LibreCAD drafting visual theme files (*%1);;All Files (*.*)")
                             .arg(m_repository != nullptr ? m_repository->getFileExtension() : QString(".lcdv"));

    s.defaultReadOnlyMessage = tr("The Default visual theme is a read-only template. To customize drawing area colors and styles, duplicate it as a custom theme.");
    s.duplicateActionText = tr("Duplicate Theme...");
    s.saveModifiedPromptTitle = tr("Save Visual Theme");
    s.saveModifiedPromptMessage = tr("You have unsaved changes to visual theme '%1'.\n\nDo you want to save them before applying?");
    s.discardConfirmTitle = tr("Discard Changes");
    s.discardConfirmMessage = tr("You have unsaved modifications to visual theme '%1'.\n\nAre you sure you want to discard these changes?");
    return s;
}

bool LC_PresetManagerDraftingTheme::resetToDefaults(const QString& key) {
    initMappingTable();

    for (const auto& entry : g_draftingThemeMappingTable) {
        if (entry.setting != nullptr) {
            entry.setting->setVariant(entry.setting->defaultVariant());
        }
    }

    setActivePresetKey(key);
    CFG_AppStyling::o_ActiveDraftingThemeName.set(key);

    if (m_resetCallback != nullptr) {
        m_resetCallback();
    }

    setDirtyState(false);
    return true;
}

bool LC_PresetManagerDraftingTheme::loadPreset(const QString& key) {
    if (isDefaultPreset(key)) {
        return resetToDefaults(key);
    }

    if (m_repository != nullptr && m_repository->loadByKey(key, m_workingConfig)) {
        applyThemeJson(m_workingConfig.rootObject);
        setActivePresetKey(key);
        m_originalActiveKey = key;
        CFG_AppStyling::o_ActiveDraftingThemeName.set(key);
        setDirtyState(false);
        return true;
    }

    // Recovery path: file is missing from disk, but values exist in settings!
    // Do not erase settings with defaults; reconstruct working config and allow saving.
    m_workingConfig.name = key;
    m_workingConfig.rootObject = serializeCurrentSettings();
    setActivePresetKey(key);
    m_originalActiveKey = key;
    CFG_AppStyling::o_ActiveDraftingThemeName.set(key);
    setDirtyState(true);
    return true;
}

bool LC_PresetManagerDraftingTheme::isPresetModified() {
    return m_isDirty;
}

void LC_PresetManagerDraftingTheme::prepareWorkingConfigForSave() {
    m_workingConfig.rootObject = serializeCurrentSettings();
}

void LC_PresetManagerDraftingTheme::onPostSavePreset(const QString& outKey) {
    CFG_AppStyling::o_ActiveDraftingThemeName.set(outKey);
}

void LC_PresetManagerDraftingTheme::applyActiveConfigToSystem(const QString& activeKey) {
    CFG_AppStyling::o_ActiveDraftingThemeName.set(activeKey);
}

QJsonObject LC_PresetManagerDraftingTheme::serializeCurrentSettings() const {
    QJsonObject root;
    initMappingTable();
    for (const auto& entry : g_draftingThemeMappingTable) {
        if (entry.setting == nullptr) {
            continue;
        }

        QVariant val = entry.setting->getVariant();
        if (!val.isValid() || (val.typeId() == QMetaType::QString && val.toString().isEmpty())) {
            val = entry.setting->defaultVariant();
        }
        setJsonValue(root, entry.jsonPath, val);
    }
    return root;
}

void LC_PresetManagerDraftingTheme::applyThemeJson(const QJsonObject& obj) {
    initMappingTable();
    for (const auto& entry : g_draftingThemeMappingTable) {
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

bool LC_PresetManagerDraftingTheme::importPresetFromFile(const QString& filePath, [[maybe_unused]] QWidget* parent) {
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
            tr("File '%1' is not a valid visual theme preset.").arg(filePath)
        ));
        return false;
    }

    applyThemeJson(root);
    setDirtyState(true);
    return true;
}

QWidget* LC_PresetManagerDraftingTheme::getSharedPreviewWidget() {
    return m_previewWidget;
}

QWidget* LC_PresetManagerDraftingTheme::getSharedBottomWidget()  {
    return m_bottomBar;
}

bool LC_PresetManagerDraftingTheme::isPreviewVisible() const {
    if (m_bottomBar != nullptr) {
        return m_bottomBar->isPreviewVisible();
    }
    return true;
}
