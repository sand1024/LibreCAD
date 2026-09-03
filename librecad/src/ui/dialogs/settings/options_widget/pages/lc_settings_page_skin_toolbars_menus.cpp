#include "lc_settings_page_skin_toolbars_menus.h"
#include "ui_lc_settings_page_skin_toolbars_menus.h"
#include <QCheckBox>
#include <QComboBox>
#include "lc_preset_manager_fusion_skin.h"

LC_SettingsPageSkinToolbarsMenus::LC_SettingsPageSkinToolbarsMenus(QObject* parent)
    : LC_SettingsPageBase(tr("Navigation"), nullptr, parent), ui(std::make_unique<Ui::LC_SettingsPageSkinToolbarsMenus>()) {
}

LC_SettingsPageSkinToolbarsMenus::~LC_SettingsPageSkinToolbarsMenus() = default;

void LC_SettingsPageSkinToolbarsMenus::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerFusionSkin*>(manager);
    if (m_presetManager != nullptr) {
        connect(m_presetManager, &LC_PresetManagerFusionSkin::configLoaded, this, [this](const ControlStyleConfig&) {
            populateUiFromWorkingConfig();
        });
        populateUiFromWorkingConfig();
    }
}

void LC_SettingsPageSkinToolbarsMenus::setupUi() {
    ui->setupUi(m_widget);
    setupComboboxes();
}

void LC_SettingsPageSkinToolbarsMenus::setupComboboxes() {
    ui->cbToolButtonIndicatorStyle->clear();
    ui->cbToolButtonIndicatorStyle->addItem(tr("Context-Aware Stripe (Adaptive)"),
                                            static_cast<int>(ToolButtonIndicatorStyle::ContextStripe));
    ui->cbToolButtonIndicatorStyle->addItem(tr("Centered 4px Accent Dot"), static_cast<int>(ToolButtonIndicatorStyle::AccentDot));
    ui->cbToolButtonIndicatorStyle->addItem(tr("Soft Outline Active Frame"), static_cast<int>(ToolButtonIndicatorStyle::AccentFrame));

    ui->cbSegmentedSeparationStyle->clear();
    ui->cbSegmentedSeparationStyle->addItem(tr("Continuous Card Well (Blender)"),
                                            static_cast<int>(SegmentedSeparationStyle::ContinuousCard));
    ui->cbSegmentedSeparationStyle->addItem(tr("Minimal Outer Border Contour"), static_cast<int>(SegmentedSeparationStyle::MinimalBorder));
    ui->cbSegmentedSeparationStyle->addItem(tr("Transparent Idle (Hover Highlight Only)"),
                                            static_cast<int>(SegmentedSeparationStyle::TransparentIdle));

    ui->cbSegmentedColorPolicy->clear();
    ui->cbSegmentedColorPolicy->addItem(tr("Muted Neutral Alternating"), static_cast<int>(SegmentedColorPolicy::NeutralAlternating));
    ui->cbSegmentedColorPolicy->addItem(tr("Palette-Matched Analogous (Recommended)"),
                                        static_cast<int>(SegmentedColorPolicy::HarmonizedAnalogous));
    ui->cbSegmentedColorPolicy->addItem(tr("Vibrant Full Pastel Spectrum"), static_cast<int>(SegmentedColorPolicy::VibrantSpectrum));
    ui->cbSegmentedColorPolicy->addItem(tr("Anchored Analogous Hue-Shift"), static_cast<int>(SegmentedColorPolicy::AnalogousHueShift));
}

void LC_SettingsPageSkinToolbarsMenus::setupBehavior() {
    connect(ui->chkToolButtonUnderline, &QCheckBox::toggled, this, [this](bool) {
        updateToolButtonUiState();
        onControlChanged();
    });
    connect(ui->cbToolButtonIndicatorStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &LC_SettingsPageSkinToolbarsMenus::onControlChanged);
    connect(ui->chkAutoPopupInstantButtons, &QCheckBox::toggled, this, &LC_SettingsPageSkinToolbarsMenus::onControlChanged);

    connect(ui->chkUseSegmentedButtons, &QCheckBox::toggled, this, [this](bool) {
        updateSegmentedButtonsUiState();
        onControlChanged();
    });
    connect(ui->cbSegmentedSeparationStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        updateSegmentedButtonsUiState();
        onControlChanged();
    });
    connect(ui->cbSegmentedColorPolicy, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LC_SettingsPageSkinToolbarsMenus::onControlChanged);

    connect(ui->chkCustomToolbarOverflow, &QCheckBox::toggled, this, &LC_SettingsPageSkinToolbarsMenus::onControlChanged);
    connect(ui->chkAutoPopupToolbarOverflow, &QCheckBox::toggled, this, &LC_SettingsPageSkinToolbarsMenus::onControlChanged);

    connect(ui->chkAutoPopupMenuBar, &QCheckBox::toggled, this, &LC_SettingsPageSkinToolbarsMenus::onControlChanged);
    connect(ui->chkMenuBarHoverCard, &QCheckBox::toggled, this, &LC_SettingsPageSkinToolbarsMenus::onControlChanged);
    connect(ui->chkShowMenuCommandAliases, &QCheckBox::toggled, this, &LC_SettingsPageSkinToolbarsMenus::onControlChanged);
    connect(ui->chkCustomMenuForTearOff, &QCheckBox::toggled, this, &LC_SettingsPageSkinToolbarsMenus::onControlChanged);
    connect(ui->chkUseFloatingHUDMenus, &QCheckBox::toggled, this, &LC_SettingsPageSkinToolbarsMenus::onControlChanged);
    connect(ui->chkSyncCheckedMenuState, &QCheckBox::toggled, this, &LC_SettingsPageSkinToolbarsMenus::onControlChanged);
}

void LC_SettingsPageSkinToolbarsMenus::loadSettings() {
    populateUiFromWorkingConfig();
}

bool LC_SettingsPageSkinToolbarsMenus::saveSettings() {
    syncUiToWorkingConfig();
    return true;
}

bool LC_SettingsPageSkinToolbarsMenus::isModified() const {
    return m_presetManager ? m_presetManager->isPresetModified() : false;
}

void LC_SettingsPageSkinToolbarsMenus::onControlChanged() {
    if (m_blockSignals) {
        return;
    }
    syncUiToWorkingConfig();
    if (m_presetManager != nullptr) {
        m_presetManager->onSubPageControlChanged();
    }
}

void LC_SettingsPageSkinToolbarsMenus::updateToolButtonUiState() {
    if (m_blockSignals) {
        return;
    }

    const bool isReadOnly = (m_presetManager != nullptr && (m_presetManager->isReadOnlyDefault() || m_presetManager->isGated()));
    const bool customEnabled = !isReadOnly && ui->chkToolButtonUnderline->isChecked();

    ui->cbToolButtonIndicatorStyle->setEnabled(customEnabled);
}

void LC_SettingsPageSkinToolbarsMenus::updateSegmentedButtonsUiState() {
    if (m_blockSignals) {
        return;
    }

    const bool isReadOnly = (m_presetManager != nullptr && (m_presetManager->isReadOnlyDefault() || m_presetManager->isGated()));
    const bool segmentedEnabled = !isReadOnly && ui->chkUseSegmentedButtons->isChecked();

    ui->lblSepStyle->setEnabled(segmentedEnabled);
    ui->cbSegmentedSeparationStyle->setEnabled(segmentedEnabled);

    // Color Policy is only applicable for ContinuousCard style when not read-only
    const auto sepStyle = static_cast<SegmentedSeparationStyle>(ui->cbSegmentedSeparationStyle->currentData().toInt());
    const bool colorPolicyApplicable = segmentedEnabled && (sepStyle == SegmentedSeparationStyle::ContinuousCard);

    ui->lblColorPolicy->setEnabled(colorPolicyApplicable);
    ui->cbSegmentedColorPolicy->setEnabled(colorPolicyApplicable);
}

void LC_SettingsPageSkinToolbarsMenus::populateUiFromWorkingConfig() {
    if (m_presetManager == nullptr || getEditingWidget() == nullptr) {
        return;
    }

    m_blockSignals = true;
    const auto& config = m_presetManager->workingConfig();

    ui->chkToolButtonUnderline->setChecked(config.useToolButtonUnderline);
    ui->cbToolButtonIndicatorStyle->setCurrentIndex(
        ui->cbToolButtonIndicatorStyle->findData(static_cast<int>(config.toolButtonIndicatorStyle)));
    ui->chkAutoPopupInstantButtons->setChecked(config.autoPopupInstantButtons);

    ui->chkUseSegmentedButtons->setChecked(config.useSegmentedToolButtons);
    ui->cbSegmentedSeparationStyle->setCurrentIndex(
        ui->cbSegmentedSeparationStyle->findData(static_cast<int>(config.segmentedSeparationStyle)));
    ui->cbSegmentedColorPolicy->setCurrentIndex(ui->cbSegmentedColorPolicy->findData(static_cast<int>(config.segmentedColorPolicy)));

    ui->chkCustomToolbarOverflow->setChecked(config.customToolbarOverflowGrip);
    ui->chkAutoPopupToolbarOverflow->setChecked(config.autoPopupToolbarOverflow);

    ui->chkAutoPopupMenuBar->setChecked(config.autoPopupMenuBar);
    ui->chkMenuBarHoverCard->setChecked(config.useMenuBarHoverCard);
    ui->chkShowMenuCommandAliases->setChecked(config.showMenuCommandAliases);
    ui->chkCustomMenuForTearOff->setChecked(config.customMenuTearOff);
    ui->chkUseFloatingHUDMenus->setChecked(config.useFloatingHUDMenus);
    ui->chkSyncCheckedMenuState->setChecked(config.syncCheckedMenuState);

    m_blockSignals = false;
    updateToolButtonUiState();
    updateSegmentedButtonsUiState();
    updateArchetypeGating();
}

void LC_SettingsPageSkinToolbarsMenus::syncUiToWorkingConfig() const {
    if (m_presetManager == nullptr) {
        return;
    }

    auto& config = m_presetManager->workingConfig();
    config.useToolButtonUnderline = ui->chkToolButtonUnderline->isChecked();
    config.toolButtonIndicatorStyle = static_cast<ToolButtonIndicatorStyle>(ui->cbToolButtonIndicatorStyle->currentData().toInt());
    config.autoPopupInstantButtons = ui->chkAutoPopupInstantButtons->isChecked();

    config.useSegmentedToolButtons = ui->chkUseSegmentedButtons->isChecked();
    config.segmentedSeparationStyle = static_cast<SegmentedSeparationStyle>(ui->cbSegmentedSeparationStyle->currentData().toInt());
    config.segmentedColorPolicy = static_cast<SegmentedColorPolicy>(ui->cbSegmentedColorPolicy->currentData().toInt());

    config.customToolbarOverflowGrip = ui->chkCustomToolbarOverflow->isChecked();
    config.autoPopupToolbarOverflow = ui->chkAutoPopupToolbarOverflow->isChecked();

    config.autoPopupMenuBar = ui->chkAutoPopupMenuBar->isChecked();
    config.useMenuBarHoverCard = ui->chkMenuBarHoverCard->isChecked();
    config.showMenuCommandAliases = ui->chkShowMenuCommandAliases->isChecked();
    config.customMenuTearOff = ui->chkCustomMenuForTearOff->isChecked();
    config.useFloatingHUDMenus = ui->chkUseFloatingHUDMenus->isChecked();
    config.syncCheckedMenuState = ui->chkSyncCheckedMenuState->isChecked();
}

void LC_SettingsPageSkinToolbarsMenus::updateArchetypeGating() {
    if (m_presetManager == nullptr) {
        return;
    }

    const bool isReadOnly = m_presetManager->isReadOnlyDefault();
    const bool isClassic = m_presetManager->isClassicFusion();

    if (isReadOnly) {
        ui->gbToolButtons->setEnabled(false);
        ui->gbOverflow->setEnabled(false);
        ui->gbMenus->setEnabled(false);
        ui->gbSegmented->setEnabled(false);
    }
    else if (isClassic) {
        ui->gbSegmented->setEnabled(false);
        ui->chkToolButtonUnderline->setEnabled(false);
        ui->cbToolButtonIndicatorStyle->setEnabled(false);
        ui->chkCustomToolbarOverflow->setEnabled(false);
        ui->chkMenuBarHoverCard->setEnabled(false);
        ui->chkShowMenuCommandAliases->setEnabled(false);
        ui->chkCustomMenuForTearOff->setEnabled(false);
        ui->chkSyncCheckedMenuState->setEnabled(false);

        ui->gbToolButtons->setEnabled(true);
        ui->chkAutoPopupInstantButtons->setEnabled(true);
        ui->gbOverflow->setEnabled(true);
        ui->chkAutoPopupToolbarOverflow->setEnabled(true);
        ui->chkUseFloatingHUDMenus->setEnabled(true);
        ui->gbMenus->setEnabled(true);
        ui->chkAutoPopupMenuBar->setEnabled(true);
    }
    else {
        ui->gbToolButtons->setEnabled(true);
        ui->gbOverflow->setEnabled(true);
        ui->gbMenus->setEnabled(true);
        ui->gbSegmented->setEnabled(true);

        ui->chkToolButtonUnderline->setEnabled(true);
        ui->chkAutoPopupInstantButtons->setEnabled(true);
        ui->chkCustomToolbarOverflow->setEnabled(true);
        ui->chkAutoPopupToolbarOverflow->setEnabled(true);
        ui->chkAutoPopupMenuBar->setEnabled(true);
        ui->chkMenuBarHoverCard->setEnabled(true);
        ui->chkShowMenuCommandAliases->setEnabled(true);
        ui->chkCustomMenuForTearOff->setEnabled(true);
        ui->chkSyncCheckedMenuState->setEnabled(true);

        updateToolButtonUiState();
        updateSegmentedButtonsUiState();
    }
}
