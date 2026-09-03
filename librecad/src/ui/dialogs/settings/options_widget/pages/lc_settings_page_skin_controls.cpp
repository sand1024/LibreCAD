#include "lc_settings_page_skin_controls.h"
#include "ui_lc_settings_page_skin_controls.h"
#include <QCheckBox>
#include <QComboBox>
#include "lc_preset_manager_fusion_skin.h"

LC_SettingsPageSkinControls::LC_SettingsPageSkinControls(QObject* parent)
    : LC_SettingsPageBase(tr("Views and Controls"), nullptr, parent)
    , ui(std::make_unique<Ui::LC_SettingsPageSkinControls>()) {
}

LC_SettingsPageSkinControls::~LC_SettingsPageSkinControls() = default;

void LC_SettingsPageSkinControls::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerFusionSkin*>(manager);
    if (m_presetManager != nullptr) {
        connect(m_presetManager, &LC_PresetManagerFusionSkin::configLoaded, this, [this](const ControlStyleConfig&) {
            populateUiFromWorkingConfig();
        });
        populateUiFromWorkingConfig();
    }
}

void LC_SettingsPageSkinControls::setupUi() {
    ui->setupUi(m_widget);
    setupComboboxes();
}

void LC_SettingsPageSkinControls::setupComboboxes() {
    ui->cbBranchIndicatorStyle->clear();
    ui->cbBranchIndicatorStyle->addItem(tr("Sleek Chevrons (Modern IDE)"), static_cast<int>(BranchIndicatorStyle::MutedChevrons));
    ui->cbBranchIndicatorStyle->addItem(tr("Plus/Minus Square Boxes"), static_cast<int>(BranchIndicatorStyle::PlusMinusBoxes));
    ui->cbBranchIndicatorStyle->addItem(tr("Tactile Circle Badges"), static_cast<int>(BranchIndicatorStyle::TactileCircles));
    ui->cbBranchIndicatorStyle->addItem(tr("Guidelines Only (No Icons)"), static_cast<int>(BranchIndicatorStyle::ClassicLinesOnly));
    ui->cbBranchIndicatorStyle->addItem(tr("None (Open Indented Layout)"), static_cast<int>(BranchIndicatorStyle::None));
}

void LC_SettingsPageSkinControls::setupBehavior() {
    const QList<QCheckBox*> checkBoxes = m_widget->findChildren<QCheckBox*>();
    for (QCheckBox* cb : checkBoxes) {
        connect(cb, &QCheckBox::toggled, this, &LC_SettingsPageSkinControls::onControlChanged);
    }

    const QList<QComboBox*> comboBoxes = m_widget->findChildren<QComboBox*>();
    for (QComboBox* cb : comboBoxes) {
        connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageSkinControls::onControlChanged);
    }
}

void LC_SettingsPageSkinControls::loadSettings() {
    populateUiFromWorkingConfig();
}

bool LC_SettingsPageSkinControls::saveSettings() {
    syncUiToWorkingConfig();
    return true;
}

bool LC_SettingsPageSkinControls::isModified() const {
    return m_presetManager ? m_presetManager->isPresetModified() : false;
}

void LC_SettingsPageSkinControls::onControlChanged() {
    if (m_blockSignals) return;
    syncUiToWorkingConfig();
    if (m_presetManager != nullptr) {
        m_presetManager->onSubPageControlChanged();
    }
}

void LC_SettingsPageSkinControls::populateUiFromWorkingConfig() {
    if (m_presetManager == nullptr || getEditingWidget() == nullptr) {
        return;
    }

    m_blockSignals = true;
    const auto& config = m_presetManager->workingConfig();

    ui->chkActiveRowSpotlight->setChecked(config.showActiveRowSpotlight);
    ui->chkShowItemViewHover->setChecked(config.showItemViewHover);
    ui->chkShowTreeConnectingLines->setChecked(config.showTreeConnectingLines);
    ui->cbBranchIndicatorStyle->setCurrentIndex(ui->cbBranchIndicatorStyle->findData(static_cast<int>(config.branchIndicatorStyle)));

    ui->chkFocusedInputGlow->setChecked(config.useFocusedInputGlow);
    ui->chkUseSpinBoxProgressBar->setChecked(config.useSpinBoxProgressBar);

    ui->chkTabStripeAtBottom->setChecked(config.tabStripeAtBottom);
    ui->chkCustomToolTipCard->setChecked(config.customToolTipCard);
    ui->chkCustomVectorIcons->setChecked(config.customVectorIconsInButtons);

    ui->chkHighContrastScrollbars->setChecked(config.accentedScrollbars);
    ui->chkTransparentScrollbars->setChecked(config.transparentScrollbars);
    ui->chkUseStatusPillChips->setChecked(config.useStatusPillChips);

    m_blockSignals = false;
    updateArchetypeGating();
}

void LC_SettingsPageSkinControls::syncUiToWorkingConfig() {
    if (m_presetManager == nullptr) return;

    auto& config = m_presetManager->workingConfig();
    config.showActiveRowSpotlight = ui->chkActiveRowSpotlight->isChecked();
    config.showItemViewHover = ui->chkShowItemViewHover->isChecked();
    config.showTreeConnectingLines = ui->chkShowTreeConnectingLines->isChecked();
    config.branchIndicatorStyle = static_cast<BranchIndicatorStyle>(ui->cbBranchIndicatorStyle->currentData().toInt());

    config.useFocusedInputGlow = ui->chkFocusedInputGlow->isChecked();
    config.useSpinBoxProgressBar = ui->chkUseSpinBoxProgressBar->isChecked();

    config.tabStripeAtBottom = ui->chkTabStripeAtBottom->isChecked();
    config.customToolTipCard = ui->chkCustomToolTipCard->isChecked();
    config.customVectorIconsInButtons = ui->chkCustomVectorIcons->isChecked();

    config.accentedScrollbars = ui->chkHighContrastScrollbars->isChecked();
    config.transparentScrollbars = ui->chkTransparentScrollbars->isChecked();
    config.useStatusPillChips = ui->chkUseStatusPillChips->isChecked();
}

void LC_SettingsPageSkinControls::updateArchetypeGating() {
    if (m_presetManager == nullptr) {
        return;
    }

    const bool isReadOnly = m_presetManager->isReadOnlyDefault();
    const bool isClassic = m_presetManager->isClassicFusion();

    if (isReadOnly) {
        ui->gbViews->setEnabled(false);
        ui->gbTabsToolTips->setEnabled(false);
        ui->gbScrollbarsStatus->setEnabled(false);
        ui->gbInputs->setEnabled(false);
    }
    else if (isClassic) {
        ui->gbViews->setEnabled(false);
        ui->gbTabsToolTips->setEnabled(false);
        ui->gbInputs->setEnabled(false);

        ui->gbScrollbarsStatus->setEnabled(true);
        ui->chkHighContrastScrollbars->setEnabled(true);
        ui->chkTransparentScrollbars->setEnabled(true);
        ui->chkUseStatusPillChips->setEnabled(false);
    }
    else {
        ui->gbViews->setEnabled(true);
        ui->gbTabsToolTips->setEnabled(true);
        ui->gbScrollbarsStatus->setEnabled(true);
        ui->gbInputs->setEnabled(true);

        ui->chkHighContrastScrollbars->setEnabled(true);
        ui->chkTransparentScrollbars->setEnabled(true);
        ui->chkUseStatusPillChips->setEnabled(true);
    }
}
