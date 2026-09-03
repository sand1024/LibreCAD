#include "lc_preset_manager_metrics.h"
#include "lc_settings_manager_styling.h"
#include "lc_style_metrics_utils.h"

LC_PresetManagerMetrics::LC_PresetManagerMetrics(QObject* parent)
    : LC_PresetManagerBase(
          QC_ApplicationWindow::getAppWindow()->getUiStyleManager(),
          QC_ApplicationWindow::getAppWindow()->getUiStyleManager()->getMetricsRepository(),
          QC_ApplicationWindow::getAppWindow()->getUiStyleManager()->getActiveMetrics(),
          parent) {
    m_headerBar = std::make_unique<LC_MetricsDensityHeaderBar>();
    connect(m_headerBar.get(), &LC_MetricsDensityHeaderBar::densityPresetSelected, this, &LC_PresetManagerMetrics::onDensityPresetSelected);

    loadPreset(m_activeKey);
}

LC_PresetManagerUIStrings LC_PresetManagerMetrics::presetStrings() const {
    LC_PresetManagerUIStrings s;
    s.defaultPresetName = tr("Default Fusion Metrics");
    s.labelText = tr("Metrics preset:");
    s.selectToolTip = tr("Select a saved metrics density configuration or load system defaults.");
    s.saveToolTip = tr("Save changes directly to active metrics preset.");
    s.saveAsToolTip = tr("Save current metrics configuration as a new preset.");
    s.deleteToolTip = tr("Permanently delete the selected custom metrics preset from disk.");
    s.applyToolTip = tr("Apply the active metrics density configuration globally to the workspace.");
    s.revertToolTip = tr("Discard modifications and reload the metrics preset as saved on disk.");
    s.saveAsDialogTitle = tr("Save Metrics Preset As");
    s.saveAsDialogLabel = tr("Enter unique metrics preset name:");
    s.defaultNewPresetName = tr("Custom Metrics");
    s.deleteConfirmTitle = tr("Delete Metrics Preset");
    s.deleteConfirmLabel = tr("Are you sure you want to delete the metrics preset '%1'?");
    s.presetFileFilter = tr("Metrics Files (*.lcms)");

    s.defaultReadOnlyMessage = tr("The Default metrics preset is a read-only template. To customize layout metrics, duplicate it as a custom preset.");
    s.duplicateActionText = tr("Duplicate Metrics...");
    return s;
}

bool LC_PresetManagerMetrics::loadPreset(const QString& key) {
    const bool isDefault = (key == DEFAULT_THEME_KEY || key.isEmpty() || key == DEFAULT_THEME_NAME ||
                            (m_repository != nullptr && !m_repository->exists(key)));

    if (isDefault) {
        resetToDefaults(m_workingConfig);
        m_workingConfig.name = defaultPresetDisplayName();
        m_activeKey = DEFAULT_THEME_KEY;
        if (m_headerBar != nullptr) {
            m_headerBar->setDensityPreset("standard");
        }
    } else if (m_repository != nullptr) {
        if (!m_repository->loadByKey(key, m_workingConfig)) {
            resetToDefaults(m_workingConfig);
            m_workingConfig.name = defaultPresetDisplayName();
            m_activeKey = DEFAULT_THEME_KEY;
        } else {
            m_activeKey = key;
        }
        if (m_headerBar != nullptr) {
            m_headerBar->setDensityToCustom();
        }
    }

    m_isDirty = false;
    if (m_changedCallback != nullptr) {
        m_changedCallback(false);
    }

    emit configLoaded(m_workingConfig);
    updatePreview();
    return true;
}

QString LC_PresetManagerMetrics::getAppliedPresetKey() const {
    return (m_styleManager != nullptr) ? m_styleManager->getActiveMetrics() : m_originalActiveKey;
}

void LC_PresetManagerMetrics::applyCurrentPreset() {
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveMetrics(m_activeKey);
        m_styleManager->applyActiveStyleAndTheme();
        m_originalActiveKey = m_activeKey;
        m_isDirty = false;
        if (m_changedCallback != nullptr) {
            m_changedCallback(false);
        }
    }
}

QWidget* LC_PresetManagerMetrics::getSharedHeaderWidget() {
    return m_headerBar.get();
}

void LC_PresetManagerMetrics::onDensityPresetSelected(const QString& presetName) {
    if (presetName == "custom") {
        return;
    }
    LC_StyleMetricsUtils::applyDensityPreset(m_workingConfig, presetName);
    notifyWorkingConfigChanged();
    emit configLoaded(m_workingConfig);
}

void LC_PresetManagerMetrics::updatePreview() {
    if (m_previewController != nullptr) {
        m_previewController->updatePreviewMetrics(m_workingConfig);
    }
}

void LC_PresetManagerMetrics::resetToDefaults(StyleMetricsConfig& config) {
    LC_StyleMetricsUtils::initializeDefault(config);
}

void LC_PresetManagerMetrics::applyActiveConfigToSystem(const QString& activeKey) {
    if (m_styleManager != nullptr) {
        m_styleManager->setActiveMetrics(activeKey);
    }
}

bool LC_PresetManagerMetrics::isGated() const {
    return LC_SettingsManagerStyling::isFusionGated() || isReadOnlyDefault();
}

QString LC_PresetManagerMetrics::gatedMessage() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return LC_SettingsManagerStyling::fusionGatedMessage(tr("metrics"));
    }
    if (isReadOnlyDefault()) {
        return presetStrings().defaultReadOnlyMessage;
    }
    return QString();
}

QString LC_PresetManagerMetrics::gatedActionText() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return LC_SettingsManagerStyling::fusionGatedActionText();
    }
    if (isReadOnlyDefault()) {
        return presetStrings().duplicateActionText;
    }
    return QString();
}

std::function<void()> LC_PresetManagerMetrics::gatedActionCallback() const {
    if (LC_SettingsManagerStyling::isFusionGated()) {
        return [this]() {
            LC_SettingsManagerStyling::enableFusionStyling(m_styleManager);
        };
    }
    return nullptr;
}
