#include "lc_settings_page_skin_containers.h"
#include "ui_lc_settings_page_skin_containers.h"
#include <QCheckBox>
#include <QComboBox>
#include "lc_preset_manager_fusion_skin.h"

LC_SettingsPageSkinContainers::LC_SettingsPageSkinContainers(QObject* parent)
    : LC_SettingsPageBase(tr("Windows, Docks & Containers"), nullptr, parent)
    , ui(std::make_unique<Ui::LC_SettingsPageSkinContainers>()) {
    setSortWeight(20);
}

LC_SettingsPageSkinContainers::~LC_SettingsPageSkinContainers() = default;

void LC_SettingsPageSkinContainers::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerFusionSkin*>(manager);
    if (m_presetManager != nullptr) {
        connect(m_presetManager, &LC_PresetManagerFusionSkin::configLoaded, this, [this](const SkinConfig&) {
            populateUiFromWorkingConfig();
        });
        populateUiFromWorkingConfig();
    }
}

void LC_SettingsPageSkinContainers::setupUi() {
    ui->setupUi(m_widget);
    setupComboboxes();
}

void LC_SettingsPageSkinContainers::setupComboboxes() {
    ui->cbGroupBoxHeaderStyle->clear();
    ui->cbGroupBoxHeaderStyle->addItem(tr("Plain (Minimal text)"), static_cast<int>(GroupBoxHeaderStyle::Plain));
    ui->cbGroupBoxHeaderStyle->addItem(tr("Overlapping"), static_cast<int>(GroupBoxHeaderStyle::Overlapping));
    ui->cbGroupBoxHeaderStyle->addItem(tr("Clean Break"), static_cast<int>(GroupBoxHeaderStyle::BreakBorder));
    ui->cbGroupBoxHeaderStyle->addItem(tr("Header Underline Divider"), static_cast<int>(GroupBoxHeaderStyle::HeaderUnderline));
    ui->cbGroupBoxHeaderStyle->addItem(tr("Header Banner Card block"), static_cast<int>(GroupBoxHeaderStyle::HeaderBanner));

    ui->cbDockTitleStyle->clear();
    ui->cbDockTitleStyle->addItem(tr("Native (Fusion Default)"), static_cast<int>(DockTitleBarStyle::Native));
    ui->cbDockTitleStyle->addItem(tr("Solid Well"), static_cast<int>(DockTitleBarStyle::CustomSolid));
    ui->cbDockTitleStyle->addItem(tr("Soft Satin"), static_cast<int>(DockTitleBarStyle::CustomSatin));
    ui->cbDockTitleStyle->addItem(tr("Glassy Gloss"), static_cast<int>(DockTitleBarStyle::CustomGlassy));
    ui->cbDockTitleStyle->addItem(tr("Accent Outline"), static_cast<int>(DockTitleBarStyle::CustomAccentOutline));
    ui->cbDockTitleStyle->addItem(tr("Accent Line (Sidebar Style)"), static_cast<int>(DockTitleBarStyle::CustomAccentLine));

    ui->cbSplitterGripStyle->clear();
    ui->cbSplitterGripStyle->addItem(tr("Sleek 3-Dot Grip"), static_cast<int>(SplitterGripStyle::MutedDots));
    ui->cbSplitterGripStyle->addItem(tr("Rounded Capsule Pill"), static_cast<int>(SplitterGripStyle::RoundedPill));
    ui->cbSplitterGripStyle->addItem(tr("Short 3-Ridge Ribbed Grip"), static_cast<int>(SplitterGripStyle::RibbedGrip));
    ui->cbSplitterGripStyle->addItem(tr("Double Full-Length Ridges"), static_cast<int>(SplitterGripStyle::DoubleRidges));
    ui->cbSplitterGripStyle->addItem(tr("Razor Hairline Divider"), static_cast<int>(SplitterGripStyle::BorderHairline));

    ui->cbCloseColorPolicy->clear();
    ui->cbCloseColorPolicy->addItem(tr("Active Accent Highlight"), static_cast<int>(CloseButtonColorPolicy::AccentColor));
    ui->cbCloseColorPolicy->addItem(tr("Muted Red Warning"), static_cast<int>(CloseButtonColorPolicy::MutedRed));
    ui->cbCloseColorPolicy->addItem(tr("Vibrant Pure Red"), static_cast<int>(CloseButtonColorPolicy::VibrantRed));
    ui->cbCloseColorPolicy->addItem(tr("Muted Neutral Border"), static_cast<int>(CloseButtonColorPolicy::MutedNeutral));
}

void LC_SettingsPageSkinContainers::setupBehavior() {
    connect(ui->chkCustomGroupBox, &QCheckBox::toggled, this, [this](bool) {
        updateGroupBoxUiState();
        onControlChanged();
    });
    connect(ui->cbGroupBoxHeaderStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        updateGroupBoxUiState();
        onControlChanged();
    });
    connect(ui->cbGroupBoxBoundaryStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageSkinContainers::onControlChanged);
    connect(ui->chkGroupBoxUseAccent, &QCheckBox::toggled, this, &LC_SettingsPageSkinContainers::onControlChanged);

    connect(ui->chkCustomDockTitle, &QCheckBox::toggled, this, [this](bool checked) {
        ui->cbDockTitleStyle->setEnabled(checked);
        onControlChanged();
    });
    connect(ui->cbDockTitleStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageSkinContainers::onControlChanged);
    connect(ui->chkPersistentDockSplitter, &QCheckBox::toggled, this, &LC_SettingsPageSkinContainers::onControlChanged);
    connect(ui->chkShowGenericDockIcons, &QCheckBox::toggled, this, &LC_SettingsPageSkinContainers::onControlChanged);
    connect(ui->chkShowSpecialDockIcons, &QCheckBox::toggled, this, &LC_SettingsPageSkinContainers::onControlChanged);
    connect(ui->chkCustomDialogTitleBar, &QCheckBox::toggled, this, &LC_SettingsPageSkinContainers::onControlChanged);

    connect(ui->chkCustomSplitterGrip, &QCheckBox::toggled, this, [this](bool) {
        updateSplitterUiState();
        onControlChanged();
    });
    connect(ui->cbSplitterGripStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageSkinContainers::onControlChanged);
    connect(ui->chkHighlightSplitterOnDrag, &QCheckBox::toggled, this, &LC_SettingsPageSkinContainers::onControlChanged);
    connect(ui->chkShowGripBackgroundWell, &QCheckBox::toggled, this, &LC_SettingsPageSkinContainers::onControlChanged);
    connect(ui->chkAccentGrips, &QCheckBox::toggled, this, &LC_SettingsPageSkinContainers::onControlChanged);

    connect(ui->chkUseFloatingHUD, &QCheckBox::toggled, this, [this](bool checked) {
        ui->cbCloseColorPolicy->setEnabled(checked);
        onControlChanged();
    });
    connect(ui->cbCloseColorPolicy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageSkinContainers::onControlChanged);
}

void LC_SettingsPageSkinContainers::loadSettings() {
    populateUiFromWorkingConfig();
}

bool LC_SettingsPageSkinContainers::saveSettings() {
    syncUiToWorkingConfig();
    return true;
}

bool LC_SettingsPageSkinContainers::isModified() const {
    return m_presetManager ? m_presetManager->isPresetModified() : false;
}

void LC_SettingsPageSkinContainers::onControlChanged() {
    if (m_blockSignals) return;
    syncUiToWorkingConfig();
    if (m_presetManager != nullptr) {
        m_presetManager->onSubPageControlChanged();
    }
}

void LC_SettingsPageSkinContainers::updateGroupBoxUiState() {
    if (m_blockSignals) return;

    const bool customEnabled = ui->chkCustomGroupBox->isChecked();
    ui->cbGroupBoxHeaderStyle->setEnabled(customEnabled);
    ui->cbGroupBoxBoundaryStyle->setEnabled(customEnabled);
    ui->chkGroupBoxUseAccent->setEnabled(customEnabled);

    if (!customEnabled) return;

    m_blockSignals = true;
    const auto headerStyle = static_cast<GroupBoxHeaderStyle>(ui->cbGroupBoxHeaderStyle->currentData().toInt());
    const auto prevBoundary = static_cast<GroupBoxBoundaryStyle>(ui->cbGroupBoxBoundaryStyle->currentData().toInt());

    ui->cbGroupBoxBoundaryStyle->clear();

    if (headerStyle == GroupBoxHeaderStyle::Overlapping || headerStyle == GroupBoxHeaderStyle::BreakBorder) {
        ui->cbGroupBoxBoundaryStyle->addItem(tr("Box Outline Frame (All sides)"), static_cast<int>(GroupBoxBoundaryStyle::Full));
        ui->cbGroupBoxBoundaryStyle->addItem(tr("Top Line Only"), static_cast<int>(GroupBoxBoundaryStyle::TopLine));
        const int idx = ui->cbGroupBoxBoundaryStyle->findData(static_cast<int>(prevBoundary));
        ui->cbGroupBoxBoundaryStyle->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    else if (headerStyle == GroupBoxHeaderStyle::HeaderUnderline || headerStyle == GroupBoxHeaderStyle::HeaderBanner) {
        ui->cbGroupBoxBoundaryStyle->addItem(tr("None (Frameless)"), static_cast<int>(GroupBoxBoundaryStyle::None));
        ui->cbGroupBoxBoundaryStyle->addItem(tr("Box Outline Frame (All sides)"), static_cast<int>(GroupBoxBoundaryStyle::Full));
        ui->cbGroupBoxBoundaryStyle->addItem(tr("Active Left Sidebar (IDE Style)"), static_cast<int>(GroupBoxBoundaryStyle::LeftStripe));
        const int idx = ui->cbGroupBoxBoundaryStyle->findData(static_cast<int>(prevBoundary));
        ui->cbGroupBoxBoundaryStyle->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    else {
        ui->cbGroupBoxBoundaryStyle->addItem(tr("None (Frameless)"), static_cast<int>(GroupBoxBoundaryStyle::None));
        ui->cbGroupBoxBoundaryStyle->addItem(tr("Box Outline Frame (All sides)"), static_cast<int>(GroupBoxBoundaryStyle::Full));
        ui->cbGroupBoxBoundaryStyle->addItem(tr("Active Left Sidebar (IDE Style)"), static_cast<int>(GroupBoxBoundaryStyle::LeftStripe));
        ui->cbGroupBoxBoundaryStyle->addItem(tr("Top Line Only"), static_cast<int>(GroupBoxBoundaryStyle::TopLine));
        const int idx = ui->cbGroupBoxBoundaryStyle->findData(static_cast<int>(prevBoundary));
        ui->cbGroupBoxBoundaryStyle->setCurrentIndex(idx >= 0 ? idx : 0);
    }
    m_blockSignals = false;
}

void LC_SettingsPageSkinContainers::updateSplitterUiState() {
    if (m_blockSignals) return;
    const bool customEnabled = ui->chkCustomSplitterGrip->isChecked();
    ui->cbSplitterGripStyle->setEnabled(customEnabled);
    ui->chkHighlightSplitterOnDrag->setEnabled(customEnabled);
    ui->chkShowGripBackgroundWell->setEnabled(customEnabled);
    ui->chkAccentGrips->setEnabled(customEnabled);
}

void LC_SettingsPageSkinContainers::populateUiFromWorkingConfig() {
    if (m_presetManager == nullptr || getEditingWidget() == nullptr) return;

    m_blockSignals = true;
    const auto& config = m_presetManager->workingConfig();

    ui->chkCustomGroupBox->setChecked(config.customGroupBoxBar);
    ui->cbGroupBoxHeaderStyle->setCurrentIndex(ui->cbGroupBoxHeaderStyle->findData(static_cast<int>(config.groupBoxHeaderStyle)));
    ui->chkGroupBoxUseAccent->setChecked(config.groupBoxUseAccent);

    ui->chkCustomDockTitle->setChecked(config.customDockTitleBar);
    ui->cbDockTitleStyle->setCurrentIndex(ui->cbDockTitleStyle->findData(static_cast<int>(config.dockTitleBarStyle)));
    ui->cbDockTitleStyle->setEnabled(config.customDockTitleBar);

    ui->chkPersistentDockSplitter->setChecked(config.persistentDockSplitter);
    ui->chkShowGenericDockIcons->setChecked(config.showGenericDockIcons);
    ui->chkShowSpecialDockIcons->setChecked(config.showSpecialDockIcons);
    ui->chkCustomDialogTitleBar->setChecked(config.customDialogTitleBar);

    ui->chkCustomSplitterGrip->setChecked(config.customSplitterGrip);
    ui->cbSplitterGripStyle->setCurrentIndex(ui->cbSplitterGripStyle->findData(static_cast<int>(config.splitterGripStyle)));
    ui->chkHighlightSplitterOnDrag->setChecked(config.highlightSplitterOnDrag);
    ui->chkShowGripBackgroundWell->setChecked(config.showGripBackgroundWell);
    ui->chkAccentGrips->setChecked(config.accentGrips);

    ui->chkUseFloatingHUD->setChecked(config.useFloatingHUD);
    ui->cbCloseColorPolicy->setCurrentIndex(ui->cbCloseColorPolicy->findData(static_cast<int>(config.closeButtonColorPolicy)));
    ui->cbCloseColorPolicy->setEnabled(config.useFloatingHUD);

    m_blockSignals = false;
    updateGroupBoxUiState();
    updateSplitterUiState();
    m_blockSignals = true;
    ui->cbGroupBoxBoundaryStyle->setCurrentIndex(ui->cbGroupBoxBoundaryStyle->findData(static_cast<int>(config.groupBoxBoundaryStyle)));
    m_blockSignals = false;
}

void LC_SettingsPageSkinContainers::syncUiToWorkingConfig() {
    if (m_presetManager == nullptr) return;

    auto& config = m_presetManager->workingConfig();
    config.customGroupBoxBar = ui->chkCustomGroupBox->isChecked();
    config.groupBoxHeaderStyle = static_cast<GroupBoxHeaderStyle>(ui->cbGroupBoxHeaderStyle->currentData().toInt());
    config.groupBoxBoundaryStyle = static_cast<GroupBoxBoundaryStyle>(ui->cbGroupBoxBoundaryStyle->currentData().toInt());
    config.groupBoxUseAccent = ui->chkGroupBoxUseAccent->isChecked();

    config.customDockTitleBar = ui->chkCustomDockTitle->isChecked();
    config.dockTitleBarStyle = static_cast<DockTitleBarStyle>(ui->cbDockTitleStyle->currentData().toInt());
    config.persistentDockSplitter = ui->chkPersistentDockSplitter->isChecked();
    config.showGenericDockIcons = ui->chkShowGenericDockIcons->isChecked();
    config.showSpecialDockIcons = ui->chkShowSpecialDockIcons->isChecked();
    config.customDialogTitleBar = ui->chkCustomDialogTitleBar->isChecked();

    config.customSplitterGrip = ui->chkCustomSplitterGrip->isChecked();
    config.splitterGripStyle = static_cast<SplitterGripStyle>(ui->cbSplitterGripStyle->currentData().toInt());
    config.highlightSplitterOnDrag = ui->chkHighlightSplitterOnDrag->isChecked();
    config.showGripBackgroundWell = ui->chkShowGripBackgroundWell->isChecked();
    config.accentGrips = ui->chkAccentGrips->isChecked();

    config.useFloatingHUD = ui->chkUseFloatingHUD->isChecked();
    config.closeButtonColorPolicy = static_cast<CloseButtonColorPolicy>(ui->cbCloseColorPolicy->currentData().toInt());
}
