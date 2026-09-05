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

#include "lc_preset_manager_viewport.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>

#include "lc_abstract_preset_manager.h"
#include "lc_dialog.h"
#include "lc_settings_appearance.h"
#include "lc_settings_app_styling.h"
#include "lc_settings_colors.h"
#include "lc_settings_defaults.h"
#include "lc_settings_grid.h"
#include "lc_settings_info_overlay_cursor.h"
#include "lc_settings_relative_position_assistant.h"
#include "lc_settings_render.h"
#include "lc_settings_snap.h"
#include "lc_settings_snap_visual.h"

namespace {
    class LC_SelectiveImportDialog : public LC_Dialog {
    public:
        explicit LC_SelectiveImportDialog(QWidget* parent) : LC_Dialog(parent, "ViewportStyleSelectiveImport") {
            setWindowTitle(tr("Import Option Schemes"));
            auto* layout = new QVBoxLayout(this);
            layout->setSpacing(10);

            m_cbStyle = new QCheckBox(tr("Viewport Colors (Style)"), this);
            m_cbStyle->setChecked(true);
            layout->addWidget(m_cbStyle);

            m_cbBehavior = new QCheckBox(tr("Drafting & Snap Settings (Behavior)"), this);
            layout->addWidget(m_cbBehavior);

            m_cbSizing = new QCheckBox(tr("Sizes & Fonts (Sizing)"), this);
            layout->addWidget(m_cbSizing);

            auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
            connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
            connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
            layout->addWidget(buttons);
        }

        bool importStyle() const {
            return m_cbStyle->isChecked();
        }

        bool importBehavior() const {
            return m_cbBehavior->isChecked();
        }

        bool importSizing() const {
            return m_cbSizing->isChecked();
        }

    private:
        QCheckBox* m_cbStyle;
        QCheckBox* m_cbBehavior;
        QCheckBox* m_cbSizing;
    };

    std::vector<LC_ViewportThemeMapEntry> g_viewportThemeMappingTable;


    void mapSettings(std::initializer_list<LC_ViewportThemeMapEntry> list, LC_ViewportThemeComponent component) {
        for (auto s: list) {
            s.component = component;
            g_viewportThemeMappingTable.push_back(s);
        }
    }

    void initMappingTable() {
        if (!g_viewportThemeMappingTable.empty()) {
            return;
        }
        using namespace CFG_Colors;
        using namespace CFG_Appearance;
        using namespace CFG_Grid;
        using namespace CFG_Snap;
        using namespace CFG_VisualSnap;
        using namespace CFG_Render;
        using namespace CFG_InfoOverlayCursor;
        using namespace CFG_Defaults;
        using namespace CFG_RelativePositionAssistant;

        // =========================================================================
        // STYLE (Aesthetic Look & Feel - The "Scheme")
        // =========================================================================
        mapSettings({
        // Colors: Core
        o_BackgroundColor, o_ForegroundBWColor, o_SelectedEntityColor, o_HighlightedEntityColor, o_DraftModeMarkerColor,

        // Colors: Grid (Points & Lines)
        o_GridPointsColor, o_MetaGridColor, o_GridLinesColor, o_MetaGridLinesColor,

        // Colors: Coordinate System & Markers
        o_RelativeZeroColor, o_XAxisColor, o_YAxisColor,
        o_AnglesBasisDirectionIndicatorColor, o_AnglesBasisAngleRayColor,

        // Colors: Snapping & Previews
        o_SnapIndicator, o_SnapIndicatorLines,
        o_VisualSnapGuideEntitiesColor, o_VisualSnapVertexesColor,
        o_VisualSnapProjectedSnapColor, o_VisualSnapDocumentEntitiesColor,
        o_PreviewEntitiesColor, o_PreviewReferencesColor, o_PreviewReferencesHighlightColor,

        // Colors: Selection & Assistant
        o_OverlayBoxLineColor, o_OverlayBoxFillColor, o_OverlayBoxLineInvertedColor, o_OverlayBoxFillInvertedColor,
        o_RelativePositionAssistantBackgroundColor, o_RelativePositionAssistantTextColor,
        o_InfoOverlayAbsoluteCoordinatesColor, o_InfoOverlayRelativeColor, o_InfoOverlayPromptColor, o_InfoOverlaySnapInfoColor,

        // Colors: Handles
        o_EntityHandleStartColor, o_EntityHandleInternalColor, o_EntityHandleEndColor,

        // Visual Logic (Gathers the 'Look' of elements)
        o_GridType, o_GridDraw, o_metaGridDraw, o_GridDrawIsoVerticalForTop,
        o_ShowUCSZeroMarker, o_ShowWCSZeroMarker, o_HideRelativeZero,
        o_ShowDraftModeMarker, o_AnglesBasisMarkEnabled,
        o_IndicatorDrawLines, o_IndicatorDrawShape,
        o_IndicatorLinesType, o_IndicatorShapeType,

        // Line Types (Aesthetic patterns)
        o_GridLinesLineType, o_metaGridLinesLineType, o_metaGridPointsLineType,
        o_SelectionOverlayLineType, o_SelectionOverlayInvertedLineType,
        o_IndicatorLinesLineType, o_VSGuidingEntitiesLineType,
        o_VSActiveGuidingEntityLineType, o_VSDocGuidingEntitiesLineType,

        // Typography (The chosen fonts)
        o_ZeroMarkerFontName, o_DraftMarkerFontName, o_FontName, o_AssistantFontName,

        // Overlays
        o_OverlayBoxTransparency
    }, LC_ViewportThemeComponent::Style);

    // =========================================================================
    // SIZING (Physical Metrics & High-DPI Adaptation)
    // =========================================================================
    mapSettings({
        // Marker & Handle Sizes
        o_EntityHandleSize, o_RelZeroMarkerRadius, o_ZeroMarkerSize,
        o_AngleSnapMarkerSize, o_IndicatorShapeSize, o_ZeroShortAxisMarkSize,
        o_VSProjectedSnapSize, o_VSHighlightedVertexSize, o_VSVertexSize,

        // Line & Point Weights (Screen Pixels)
        o_OverlaysScreenLineWidth, o_IndicatorScreenLinesLineWidth,
        o_metaGridPointsLineWidth, o_metaGridLinesLineWidth,
        o_GridLinesLineWidth, o_GridPointsPointSize,
        o_VSGuidingEntitiesLineWidth, o_VSGuidingPointsLineWidth,

        // Text Metrics
        o_DraftMarkerFontSize, o_ZeroMarkerFontSize, o_FontSize,
        o_AssistantFontSize, o_VSGuidingEntityLabelFontSize, o_VSGuidingLabelOffsetPx,
        o_OffsetFromCursor,

        // Logic-based Sizes
        o_RefPointSize, // Stored as QString in UI but represents a metric

        // Thresholds
        o_MinRenderableTextHeightPx, o_MinLineLen, o_MinArcRadius,
        o_MinCircleRadius, o_MinEllipseMajor, o_MinEllipseMinor
    }, LC_ViewportThemeComponent::Sizing);

    // =========================================================================
    // BEHAVIOR (Drafting Workflows & View Logic)
    // =========================================================================
    mapSettings({
        // View & Navigation
        o_ScrollBars, o_Antialiasing, o_ClassicRenderer, o_Autopanning,
        o_FirstTimeNoZoom, o_PanOnZoom, o_ScrollZoomFactor,
        o_WheelScrollInvertH, o_WheelScrollInvertV, o_InvertZoomDirection,
        o_ModifyOnViewChange, // Found on draw.renderer, though belongs to defaults

        // Grid & Snap Calculations
        o_ScaleGrid, o_UnitlessGrid, o_MetaGridEvery, o_MinGridSpacing,
        o_CursorHidingWhenSnapping, o_showSnapOptionsInSnapToolbar,
        o_SnapGridIgnoreIfNoGrid, o_AdvSnapEntityCatchRange,
        o_AdvSnapOnEntitySwitchToFreeDistance, o_AdvSnapGridCellSnapFactor,
        o_AngleSnapToLinesIfGrid, o_AngleSnapStep,

        // Visual Snap Logic
        o_VSGuidingEntitiesCatchDistance, o_VSAngleSnapStepRaysVertexes,
        o_VSAngleSnapStepRaysRelative, o_VSSnapManualAddingWithCTRL,
        o_VSVertexVertexDistanceCircles, o_VSVertexVertexDistanceTangents,
        o_VSShowNotSnappableGuides, o_VSSnapAutoAddSnapPoint,
        o_VSSnapAutoAddLastSnapPointOnly, o_VSClearSolutionByRMB,
        o_VSSnapPointAddingDelay, o_VSVertexAddingDelay, o_VSDocEntityAddingDelay,

        // Renderer Logic
        o_GridRenderSimple, o_GridDisableWithinPan, o_RefPointType,
        o_ExtendAxisLines, o_ExtendModeXAxis, o_ExtendModeYAxis,
        o_AnglesBasisMarkPolicy, o_VisualizeHovering, o_VisualizeHoveringRefPoints,
        o_VisualizePreviewRefPoints, o_IgnoreDraftForHighlight,
        o_DrawTextsAsDraftInPanning, o_DrawTextsAsDraftInPreview,
        o_CircleRenderAsArcs, o_ArcRenderInterpolate,
        o_ArcRenderInterpolateSegmentFixed, o_ArcRenderInterpolateSegmentAngle,
        o_ArcRenderInterpolateSegmentSagitta, o_FontLettersColumnsCount,
        o_MaxPreview,

        // Context Overlay Display State
        o_InfoCursorEnabled, o_ShowAbsolute, o_ShowAbsoluteWCS, o_ShowRelativeDA,
        o_ShowRelativeDD, o_ShowSnapInfo, o_ShowPrompt, o_ShowActionName,
        o_ShowLabels, o_SingleLine, o_ShowPropertiesCatched,
        o_ShowPropertiesCreating, o_ShowPropertiesEdit,
        o_RememberCoordinatesMode, o_StartInOffsetMode
    }, LC_ViewportThemeComponent::Behavior);
    }

    QVariant getJsonValue(const QJsonObject& obj, const QString& path) {
        QStringList segments = path.split('.');
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
        QStringList segments = path.split('.');
        QList<QJsonObject> objects;
        objects.append(obj);

        for (int i = 0; i < segments.size() - 1; ++i) {
            QJsonObject child = objects.last().value(segments[i]).toObject();
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

LC_PresetManagerViewport::LC_PresetManagerViewport(QObject* parent, QWidget* previewWidget)
    :LC_AbstractPresetManager(CFG_AppStyling::o_ActiveGraphicViewScheme.get(), parent)
    , m_previewWidget(previewWidget) {
    m_repository.initializeIndex();
}

LC_PresetManagerUIStrings LC_PresetManagerViewport::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Default Theme");
    s.labelText = tr("Drawing visual scheme:");
    s.selectToolTip = tr("Select a drawing area visual scheme.");
    s.saveToolTip = tr("Save current visual scheme.");
    s.saveAsToolTip = tr("Save current visual settings configuration as a new scheme.");
    s.deleteToolTip = tr("Delete the selected custom visual scheme.");
    s.applyToolTip = tr("Apply the active drawing scheme to the viewport.");
    s.revertToolTip = tr("Discard modifications and reload the scheme as saved on disk.");
    s.saveAsDialogTitle = tr("Save Drawing Visual Scheme As");
    s.saveAsDialogLabel = tr("Enter unique scheme name:");
    s.defaultNewPresetName = tr("Custom Scheme");
    s.deleteConfirmTitle = tr("Delete Scheme");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the scheme '%1'?");
    s.exportDialogTitle = tr("Export Drawing Visual Scheme");
    s.importDialogTitle = tr("Import Drawing Visual Scheme");
    s.presetFileFilter = tr("Drawing Visual Scheme Files (*.lsvs)");

    s.defaultReadOnlyMessage = tr("The Default visual scheme is a read-only template. To customize drawing area colors, duplicate it as a custom scheme.");
    s.duplicateActionText = tr("Duplicate Scheme...");

    s.saveModifiedPromptTitle = tr("Save Viewport Visual Scheme");
    s.saveModifiedPromptMessage = tr("You have unsaved changes to visual scheme '%1'.\n\nDo you want to save them before applying?");
    s.discardConfirmTitle = tr("Discard Changes");
    s.discardConfirmMessage = tr("You have unsaved modifications to visual scheme '%1'.\n\nAre you sure you want to discard these changes?");
    return s;
}

bool LC_PresetManagerViewport::resetToDefaults(const QString& key) {
    initMappingTable();

    for (const auto& entry : g_viewportThemeMappingTable) {
        if (entry.component == LC_ViewportThemeComponent::Style) {
            entry.setting->setVariant(entry.setting->defaultVariant());
        }
    }

    m_activeKey = key;
    m_registryDiverged = false;
    CFG_AppStyling::o_ActiveGraphicViewScheme.set(key);

    if (m_resetCallback) {
        m_resetCallback();
    }

    setDirtyState(false);
    return true;
}

bool LC_PresetManagerViewport::loadPreset(const QString& key) {
    bool success = false;
    if (key == DEFAULT_THEME_KEY) {
        return resetToDefaults(key);
    } else {
        LC_ViewportThemeConfig config;
        if (m_repository.loadByKey(key, config)) {
            applyThemeJson(config.rootObject);
            success = true;
        }
    }

    if (success) {
        m_activeKey = key;
        CFG_AppStyling::o_ActiveGraphicViewScheme = key;
        m_registryDiverged = false;
        setDirtyState(false);
    }
    return success;
}

bool LC_PresetManagerViewport::isPresetModified() {
    return m_isDirty || m_registryDiverged;
}

void LC_PresetManagerViewport::applyActiveConfigToSystem(const QString& activeKey) {
    CFG_AppStyling::o_ActiveGraphicViewScheme.set(activeKey);
}

void LC_PresetManagerViewport::applyCurrentPreset() {
    if (m_saveCommitCallback != nullptr) {
        m_saveCommitCallback(); // Commits active page widgets into RS_Settings
    }

    applyActiveConfigToSystem(m_activeKey);
    m_originalActiveKey = m_activeKey;
    m_registryDiverged = calculateDivergence();
    setDirtyState(false);
}


bool LC_PresetManagerViewport::saveCurrentPreset() {
    if (isReadOnlyDefault()) {
        return false;
    }
    LC_ViewportThemeConfig config;

    QString name;
    for (const auto& choice : m_repository.getPresetChoices()) {
        if (choice.second == m_activeKey) {
            name = choice.first;
            break;
        }
    }
    if (name.isEmpty()) {
        name = tr("Custom");
    }
    config.name = name;

    if (m_saveCommitCallback) {
        m_saveCommitCallback();
    }

    config.rootObject = serializeCurrentSettings();
    QString outKey;
    if (!m_repository.save(config.name, config, outKey)) {
        return false;
    }
    m_activeKey = outKey;
    CFG_AppStyling::o_ActiveGraphicViewScheme.set(outKey);
    m_registryDiverged = false;
    setDirtyState(false);
    return true;
}

bool LC_PresetManagerViewport::savePresetAs(const QString& name, QString& outKey) {
    LC_ViewportThemeConfig config;
    config.name = name;

    if (m_saveCommitCallback) {
        m_saveCommitCallback();
    }

    config.rootObject = serializeCurrentSettings();
    if (m_repository.save(name, config, outKey)) {
        m_activeKey = outKey;
        CFG_AppStyling::o_ActiveGraphicViewScheme.set(outKey);
        m_registryDiverged = false;
        setDirtyState(false);
        return true;
    }
    return false;
}

bool LC_PresetManagerViewport::deletePreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY || key.isEmpty()) {
        return false;
    }
    return m_repository.deleteByKey(key);
}

bool LC_PresetManagerViewport::calculateDivergence() const {
    if (isReadOnlyDefault()) {
        return false;
    }

    initMappingTable();
    LC_ViewportThemeConfig config;
    if (!m_repository.loadByKey(m_activeKey, config)) {
        return false;
    }

    for (const auto& entry : g_viewportThemeMappingTable) {
        if (entry.component != LC_ViewportThemeComponent::Style) {
            continue;
        }
        if (entry.setting->getVariant().toString() != getJsonValue(config.rootObject, entry.jsonPath).toString()) {
            return true;
        }
    }
    return false;
}

QList<QPair<QString, QString>> LC_PresetManagerViewport::getAvailablePresets() const {
    auto choices = m_repository.getPresetChoices();
    choices.prepend(qMakePair(defaultPresetDisplayName(), DEFAULT_THEME_KEY));
    return choices;
}


QJsonObject LC_PresetManagerViewport::serializeCurrentSettings() const {
    QJsonObject root;
    initMappingTable();
    for (const auto& entry : g_viewportThemeMappingTable) {
        if (entry.component != LC_ViewportThemeComponent::Style)
            continue;

        QVariant val = entry.setting->getVariant();
        if (!val.isValid() || (val.typeId() == QMetaType::QString && val.toString().isEmpty())) {
            val = entry.setting->defaultVariant();
        }
        setJsonValue(root, entry.jsonPath, val);
    }
    return root;
}

void LC_PresetManagerViewport::applyThemeJson(const QJsonObject& obj) {
    initMappingTable();
    for (const auto& entry : g_viewportThemeMappingTable) {
        if (entry.component != LC_ViewportThemeComponent::Style) {
            continue;
        }

        QVariant val = getJsonValue(obj, entry.jsonPath);
        if (val.isValid()) {
            entry.setting->setVariant(val);
        }
    }
    RS_Settings::instance()->emitOptionsChanged();
}

bool LC_PresetManagerViewport::importPresetFromFile(const QString& filePath, QWidget* parent) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonObject root = doc.object();

    if (!m_repository.isCompatible(root)) {
        return false;
    }

    LC_SelectiveImportDialog dlg(parent);
    if (dlg.exec() == QDialog::Accepted) {
        QSet<LC_ViewportThemeComponent> components;
        if (dlg.importStyle()) components.insert(LC_ViewportThemeComponent::Style);
        if (dlg.importBehavior()) components.insert(LC_ViewportThemeComponent::Behavior);
        if (dlg.importSizing()) components.insert(LC_ViewportThemeComponent::Sizing);

        for (const auto& entry : g_viewportThemeMappingTable) {
            initMappingTable();
            if (!components.contains(entry.component)) {
                continue;
            }

            QVariant val = getJsonValue(root, entry.jsonPath);
            if (val.isValid()) {
                entry.setting->setVariant(val);
            }
        }

        RS_Settings::instance()->emitOptionsChanged();
        setDirtyState(true);
        return true;
    }
    return false;
}

bool LC_PresetManagerViewport::exportPresetToFile(const QString&, const QString& filePath, QWidget*) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    QJsonObject root = serializeCurrentSettings();
    root["file_identifier"] = m_repository.fileIdentifier();

    file.write(QJsonDocument(root).toJson());
    return true;
}
