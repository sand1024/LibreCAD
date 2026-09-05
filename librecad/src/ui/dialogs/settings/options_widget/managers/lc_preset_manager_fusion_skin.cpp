#include "lc_preset_manager_fusion_skin.h"

#include "lc_fusion_skins_repository.h"
#include "lc_palette_color_utils.h"
#include "lc_settings_manager_styling.h"
#include "lc_skin_archetype_header_bar.h"

LC_PresetManagerFusionSkin::LC_PresetManagerFusionSkin(QObject* parent)
    : LC_PresetManagerBase(QC_ApplicationWindow::getAppWindow()->getUiStyleManager(),
                      QC_ApplicationWindow::getAppWindow()->getUiStyleManager()->getSkinsRepository(),
                   QC_ApplicationWindow::getAppWindow()->getUiStyleManager()->getActiveSkin(), parent) {
    m_headerBar = std::make_unique<LC_SkinArchetypeHeaderBar>();
    auto bar = m_headerBar.get();
    connect(bar, &LC_SkinArchetypeHeaderBar::archetypeChanged, this, &LC_PresetManagerFusionSkin::onArchetypeChanged);
    connect(bar, &LC_SkinArchetypeHeaderBar::decorationChanged, this, &LC_PresetManagerFusionSkin::onDecorationChanged);

    loadPreset(m_activeKey);
}

LC_PresetManagerUIStrings LC_PresetManagerFusionSkin::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Default Fusion Style");
    s.labelText = tr("Controls style preset:");
    s.selectToolTip = tr("Select a saved controls style configuration or load standard defaults.");
    s.saveToolTip = tr("Save changes directly to active controls style preset.");
    s.saveAsToolTip = tr("Save current controls style configuration as a new preset.");
    s.deleteToolTip = tr("Permanently delete the selected custom style preset from disk.");
    s.applyToolTip = tr("Apply the active controls style configuration globally to the workspace.");
    s.revertToolTip = tr("Discard modifications and reload the controls style preset as saved on disk.");
    s.saveAsDialogTitle = tr("Save Controls Style Preset As");
    s.saveAsDialogLabel = tr("Enter unique controls style preset name:");
    s.defaultNewPresetName = tr("Custom Controls Style");
    s.deleteConfirmTitle = tr("Delete Controls Style Preset");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the controls style preset '%1'?");
    s.presetFileFilter = tr("Controls Style Files (*.lcsk)");

    s.defaultReadOnlyMessage = tr("The Default controls style is a read-only template. To customize widget decorators and styling, duplicate it as a custom preset.");
    s.duplicateActionText = tr("Duplicate Style...");
    return s;
}

bool LC_PresetManagerFusionSkin::loadPreset(const QString& key) {
    const bool isDefault = (key == DEFAULT_THEME_KEY || key.isEmpty() || key == DEFAULT_THEME_NAME ||
                            (m_repository != nullptr && !m_repository->exists(key)));

    if (isDefault) {
        resetToDefaults(m_workingConfig);
        m_workingConfig.name = defaultPresetDisplayName();
        m_activeKey = DEFAULT_THEME_KEY;
    } else if (m_repository != nullptr) {
        if (!m_repository->loadByKey(key, m_workingConfig)) {
            resetToDefaults(m_workingConfig);
            m_workingConfig.name = defaultPresetDisplayName();
            m_activeKey = DEFAULT_THEME_KEY;
        } else {
            m_activeKey = key;
        }
    }

    if (m_headerBar != nullptr) {
        m_headerBar->populateFromConfig(m_workingConfig);
    }

    m_isDirty = false;
    if (m_changedCallback != nullptr) {
        m_changedCallback(false);
    }

    emit configLoaded(m_workingConfig);
    updatePreview();
    return true;
}

QString LC_PresetManagerFusionSkin::getAppliedPresetKey() const {
    return (m_styleManager != nullptr) ? m_styleManager->getActiveSkin() : m_originalActiveKey;
}

void LC_PresetManagerFusionSkin::applyCurrentPreset() {
    applyActiveConfigToSystem(m_activeKey);
    if (m_styleManager != nullptr) {
        m_styleManager->applyActiveStyleAndTheme();
    }
    m_originalActiveKey = m_activeKey;
    setDirtyState(false);
}

QWidget* LC_PresetManagerFusionSkin::getSharedHeaderWidget() {
    return m_headerBar.get();
}

void LC_PresetManagerFusionSkin::onArchetypeChanged(StyleArchetype archetype) {
    m_workingConfig.styleArchetype = archetype;
    emit configLoaded(m_workingConfig);
    notifyWorkingConfigChanged();
}

void LC_PresetManagerFusionSkin::onDecorationChanged(BoxDecoration decoration) {
    m_workingConfig.boxDecoration = decoration;
    notifyWorkingConfigChanged();
}

void LC_PresetManagerFusionSkin::updatePreview() {
    if (m_previewController != nullptr) {
        m_previewController->updatePreviewSkin(m_workingConfig);
    }
}

void LC_PresetManagerFusionSkin::resetToDefaults(ControlStyleConfig& config) {
    LC_PaletteColorUtils::initializeDefaultControlStyle(config);
}

void LC_PresetManagerFusionSkin::applyActiveConfigToSystem(const QString& activeKey) {
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveSkin(activeKey);
    }
}

bool LC_PresetManagerFusionSkin::isGated() const {
    return LC_SettingsManagerStyling::isFusionGated() || isReadOnlyDefault();
}

QString LC_PresetManagerFusionSkin::gatedMessage() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return LC_SettingsManagerStyling::fusionGatedMessage(tr("controls styling"));
    }
    if (isReadOnlyDefault()) {
        return presetStrings().defaultReadOnlyMessage;
    }
    if (isClassicFusion()) {
        return tr("The Classic Fusion archetype uses native Qt widget drawing. Sub-controls for custom grips, outlines, and item view hover are not applicable.");
    }
    return QString();
}

QString LC_PresetManagerFusionSkin::gatedActionText() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return LC_SettingsManagerStyling::fusionGatedActionText();
    }
    if (isReadOnlyDefault()) {
        return presetStrings().duplicateActionText;
    }
    return QString();
}

std::function<void()> LC_PresetManagerFusionSkin::gatedActionCallback() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return [this]() {
            LC_SettingsManagerStyling::enableFusionStyling(m_styleManager);
        };
    }
    return nullptr;
}
