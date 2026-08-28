/*
**********************************************************************************
**
** This file was created for the LibreCAD project (librecad.org), a 2D CAD program.
**
** Copyright (C) 2015 ravas (github.com/r-a-v-a-s)
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** as published by the Free Software Foundation; either version 2
** of the License, or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
**********************************************************************************
 */

#include "lc_widgetoptionsdialog.h"

#include <csignal>
#include <QColorDialog>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QPixmapCache>
#include <QStatusBar>
#include <QStyleFactory>
#include <QTimer>

#include "lc_dlg_preset_editor.h"
#include "lc_dlg_styles_presets_generator.h"
#include "lc_editor_utils.h"
#include "lc_fusion_skins_repository.h"
#include "lc_proxy_style.h"
#include "lc_icons_style_manager.h"
#include "lc_inputtextdialog.h"
#include "lc_metrics_repository.h"
#include "lc_palette_color_utils.h"
#include "lc_settings_app_styling.h"
#include "lc_settings_startup.h"
#include "lc_settings_widget.h"
#include "lc_style_editor_fusion_skin.h"
#include "lc_style_editor_icons_style.h"
#include "lc_style_editor_metrics.h"
#include "lc_style_editor_typography.h"
#include "lc_style_preset_generator.h"
#include "lc_typography_repository.h"

#include "lc_widgetfactory.h"
#include "qc_applicationwindow.h"
#include "rs_settings.h"

#define ENABLE_PRESETS_GENERATOR true

LC_WidgetOptionsDialog::LC_WidgetOptionsDialog(QWidget* parent, LC_UIStyleManager *styleManager)
    : LC_Dialog(parent, "WidgetOptions")
    , m_styleManager(styleManager) {
    setupUi(this);

    m_origAllowStyle = m_styleManager->isStyleAllowed();
    m_origStyle                 = m_styleManager->getActiveStyle();
    m_origThemeMode             = m_styleManager->getThemeModeOverride();
    m_origStyleSheet            = m_styleManager->getActiveStyleSheet();
    m_origIgnoreIconStyling     = m_styleManager->getIgnoreIconStylingInTheme();

    m_origSkinKey               = m_styleManager->getActiveSkin();
    m_origIconStyleKey          = m_styleManager->getActiveIconStyle();
    m_origTypographyKey         = m_styleManager->getActiveTypography();
    m_origMetricsKey            = m_styleManager->getActiveMetrics();


    populateDropdowns();

    style_checkbox->setChecked(m_origAllowStyle);
    style_combobox->addItems(QStyleFactory::keys());
    if (!m_origStyle.isEmpty()) {
        const int styleIdx = style_combobox->findText(m_origStyle);
        if (styleIdx >= 0) {
            style_combobox->setCurrentIndex(styleIdx);
        }
    }

   cbIgnoreIconStylingInTheme->setChecked( m_styleManager->getIgnoreIconStylingInTheme());

    connect(pbPaletteEditor, &QPushButton::clicked, this, &LC_WidgetOptionsDialog::onEditSkinsClicked);
    connect(pbAdvancedIcons, &QPushButton::clicked, this, &LC_WidgetOptionsDialog::onEditIconStylesClicked);
    connect(pbTypographyEditor, &QPushButton::clicked, this, &LC_WidgetOptionsDialog::onEditTypographyClicked);
    connect(pbMetricsEditor, &QPushButton::clicked, this, &LC_WidgetOptionsDialog::onEditMetricsClicked);


    connect(pbImportProfile, &QPushButton::clicked, this, &LC_WidgetOptionsDialog::onImportProfileClicked);
    connect(pbExportProfile, &QPushButton::clicked, this, &LC_WidgetOptionsDialog::onExportProfileClicked);
    pbPresetsGenerator->setVisible(ENABLE_PRESETS_GENERATOR);
    if (ENABLE_PRESETS_GENERATOR) {
        connect(pbPresetsGenerator, &QPushButton::clicked, [this](bool)-> void {
            LC_DlgStylesPresetsGenerator dlg(this, m_styleManager);
            dlg.showModal();
        });
    }

    connect(style_checkbox, &QCheckBox::toggled, this, &LC_WidgetOptionsDialog::updateStyleDependencyStates);
    connect(style_combobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_WidgetOptionsDialog::updateStyleDependencyStates);

    connect(style_checkbox, &QCheckBox::toggled, m_styleManager, &LC_UIStyleManager::applyActiveStyleAndTheme);
    connect(style_combobox, QOverload<int>::of(&QComboBox::currentIndexChanged), m_styleManager, &LC_UIStyleManager::applyActiveStyleAndTheme);
    // connect(cbIgnoreIconStylingInTheme, &QCheckBox::toggled, m_styleManager, &LC_UIStyleManager::applyActiveIconStyle);

    connect(tbSelectStylesheet, &QPushButton::released, this, &LC_WidgetOptionsDialog::chooseStyleSheet);
    connect(stylesheet_field, &QLineEdit::editingFinished, m_styleManager, &LC_UIStyleManager::applyActiveStyleSheet);
    connect(tbOverridesDir, &QToolButton::clicked, this, &LC_WidgetOptionsDialog::setIconsOverrideFolder);


    connect(style_checkbox, &QCheckBox::toggled, this, &LC_WidgetOptionsDialog::applyTransientStylePreview);
    connect(style_combobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_WidgetOptionsDialog::applyTransientStylePreview);
    connect(cbFusionSkin, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_WidgetOptionsDialog::applyTransientStylePreview);
    connect(cbActiveMetrics, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_WidgetOptionsDialog::applyTransientStylePreview);
    connect(cbTypography, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_WidgetOptionsDialog::applyTransientStylePreview);
    connect(cbIconStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_WidgetOptionsDialog::applyTransientStylePreview);
    connect(cbThemeModeOverride, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_WidgetOptionsDialog::applyTransientStylePreview);
    connect(cbIgnoreIconStylingInTheme, &QCheckBox::toggled, this, &LC_WidgetOptionsDialog::applyTransientStylePreview);

    setupThemeModeOverrideCombobox();
    updateStyleDependencyStates();
    setupGenericTabUI(); // Load legacy CAD widgets and status bar configurations
}

void LC_WidgetOptionsDialog::setupThemeModeOverrideCombobox() {
    cbThemeModeOverride->blockSignals(true);
    cbThemeModeOverride->addItem(tr("Follow System Settings"), static_cast<int>(ThemeModeOverride::FollowSystem));
    cbThemeModeOverride->addItem(tr("Force Light Mode"), static_cast<int>(ThemeModeOverride::ForceLight));
    cbThemeModeOverride->addItem(tr("Force Dark Mode"), static_cast<int>(ThemeModeOverride::ForceDark));

    auto data = QVariant(static_cast<int>(m_origThemeMode));
    const int modeIdx = cbThemeModeOverride->findData(data);
    if (modeIdx >= 0) {
        cbThemeModeOverride->setCurrentIndex(modeIdx);
    }
    cbThemeModeOverride->blockSignals(false);
    connect(cbThemeModeOverride, QOverload<int>::of(&QComboBox::currentIndexChanged), m_styleManager, &LC_UIStyleManager::applyActiveThemeOverride);
}

void LC_WidgetOptionsDialog::populateSkinsCombobox() {
    cbFusionSkin->blockSignals(true);
    cbFusionSkin->clear();
    cbFusionSkin->addItem(tr("Default (Follow Theme)"), DEFAULT_THEME_KEY);
    for (const auto& choice : m_styleManager->getSkinsRepository()->getPresetChoices()) {
        cbFusionSkin->addItem(choice.first, choice.second);
    }
    setComboboxToActive( m_styleManager->getActiveSkin(), cbFusionSkin);
    cbFusionSkin->blockSignals(false);
    updateSkinComboFonts();
}

void LC_WidgetOptionsDialog::populateIconsStyleCombobox() {
    cbIconStyle->blockSignals(true);
    cbIconStyle->clear();
    cbIconStyle->addItem(tr("Default (Classic)"), DEFAULT_THEME_KEY);
    for (const auto& choice : m_styleManager->getIconsStyleRepository()->getPresetChoices()) {
        cbIconStyle->addItem(choice.first, choice.second);
    }
    setComboboxToActive( m_styleManager->getActiveIconStyle(), cbIconStyle);
    cbIconStyle->blockSignals(false);
    updateIconStyleComboFonts();
}

void LC_WidgetOptionsDialog::populateTypographyCombobox() {
    cbTypography->blockSignals(true);
    cbTypography->clear();
    cbTypography->addItem(tr("Default Typography"), DEFAULT_THEME_KEY);
    for (const auto& choice : m_styleManager->getTypographyRepository()->getPresetChoices()) {
        cbTypography->addItem(choice.first, choice.second);
    }
    setComboboxToActive( m_styleManager->getActiveTypography(), cbTypography);
    cbTypography->blockSignals(false);
    updateTypographyComboFonts();
}

void LC_WidgetOptionsDialog::setComboboxToActive(QString activeIconStyleKey, QComboBox* cb) {
    const int index = cb->findData(activeIconStyleKey);
    cb->setCurrentIndex(index >= 0 ? index : 0);
}

void LC_WidgetOptionsDialog::populateMetricsCombobox() {
    cbActiveMetrics->blockSignals(true);
    cbActiveMetrics->clear();
    cbActiveMetrics->addItem(tr("Standard Density"), DEFAULT_THEME_KEY);
    for (const auto& choice : m_styleManager->getMetricsRepository()->getPresetChoices()) {
        cbActiveMetrics->addItem(choice.first, choice.second);
    }
    setComboboxToActive( m_styleManager->getActiveMetrics(), cbActiveMetrics);
    cbActiveMetrics->blockSignals(false);
    updateMetricsComboFonts();
}

void LC_WidgetOptionsDialog::populateDropdowns() {
    populateSkinsCombobox();
    populateIconsStyleCombobox();
    populateTypographyCombobox();
    populateMetricsCombobox();
}

void LC_WidgetOptionsDialog::updateStyleDependencyStates() {
    const bool allowStyle = style_checkbox->isChecked();
    style_combobox->setEnabled(allowStyle);

    const bool isFusion = allowStyle && (style_combobox->currentText() == "Fusion");
    cbFusionSkin->setEnabled(isFusion);
    pbPaletteEditor->setEnabled(isFusion);
    cbActiveMetrics->setEnabled(isFusion);
    pbMetricsEditor->setEnabled(isFusion);
    // cbTypography->setEnabled(isFusion);
    // pbTypographyEditor->setEnabled(isFusion);
    
    cbThemeModeOverride->setEnabled(isFusion);
    lblThemeModeOverride->setEnabled(isFusion);

    stylesheet_field->setEnabled(!isFusion);
    tbSelectStylesheet->setEnabled(!isFusion);
    lblStylesheet->setEnabled(!isFusion);

    cbIgnoreIconStylingInTheme->setEnabled(isFusion);
}

void LC_WidgetOptionsDialog::onEditSkinsClicked() {
    auto* editor = new LC_StyleEditorFusionSkin(this, m_styleManager);
    QString selectedKey = cbFusionSkin->currentData().toString();
    LC_DlgPresetEditor dlg(this, m_styleManager, editor, "PaletteEditor", selectedKey);

    dlg.exec();
    populateSkinsCombobox();
}

void LC_WidgetOptionsDialog::onEditIconStylesClicked() {
    auto* editor = new LC_StyleEditorIconsStyle(this, m_styleManager);
    QString selectedKey = cbIconStyle->currentData().toString();
    LC_DlgPresetEditor dlg(this, m_styleManager, editor, "IconsStyling", selectedKey);
    dlg.exec();
    populateIconsStyleCombobox();
}

void LC_WidgetOptionsDialog::onEditTypographyClicked() {
    auto* editor = new LC_StyleEditorTypography(this, m_styleManager);
    QString selectedKey = cbTypography->currentData().toString();
    LC_DlgPresetEditor dlg(this, m_styleManager, editor, "TypographyEditor", selectedKey);

    dlg.exec();
    populateTypographyCombobox();
}

void LC_WidgetOptionsDialog::onEditMetricsClicked() {
    auto* editor = new LC_StyleEditorMetrics(this, m_styleManager);
    QString selectedKey = cbActiveMetrics->currentData().toString();
    LC_DlgPresetEditor dlg(this, m_styleManager, editor, "MetricsEditor", selectedKey);
    dlg.exec();
    populateMetricsCombobox();
}

void LC_WidgetOptionsDialog::onImportProfileClicked() {
    const QString path = QFileDialog::getOpenFileName(this, tr("Import Workspace Profile"), "", tr("LibreCAD Workspace Profiles (*.lcws)"));
    if (path.isEmpty()) return;

    QString profileName, skinFile, iconFile, typographyFile, metricsFile;
    if (m_styleManager->importProfile(path, profileName, skinFile, iconFile, typographyFile, metricsFile)) {
        populateDropdowns(); // Synchronously rebuild combobox indexes

        // Dynamic visual update selecting the unpacked active keys
        if (!skinFile.isEmpty()) cbFusionSkin->setCurrentIndex(cbFusionSkin->findData(skinFile));
        if (!iconFile.isEmpty()) cbIconStyle->setCurrentIndex(cbIconStyle->findData(iconFile));
        if (!typographyFile.isEmpty()) cbTypography->setCurrentIndex(cbTypography->findData(typographyFile));
        if (!metricsFile.isEmpty()) cbActiveMetrics->setCurrentIndex(cbActiveMetrics->findData(metricsFile));

        QMessageBox::information(this, tr("Profile Imported"), tr("Workspace profile '%1' has been successfully imported and applied.").arg(profileName));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Could not parse or import the workspace profile."));
    }
}

void LC_WidgetOptionsDialog::onExportProfileClicked() {
    QString path = QFileDialog::getSaveFileName(this, tr("Export Workspace Profile"), "", tr("LibreCAD Workspace Profiles (*.lcws)"));
    if (path.isEmpty()) return;

    bool ok;
    QString profileName = QInputDialog::getText(this, tr("Export Profile"), tr("Enter profile name:"), QLineEdit::Normal, "My Custom Profile", &ok);
    if (!ok || profileName.trimmed().isEmpty()) return;

    SkinConfig skin;
    bool hasSkin = m_styleManager->getSkinsRepository()->loadByKey(cbFusionSkin->currentData().toString(), skin);

    IconStyleConfig icon;
    bool hasIcon = m_styleManager->getIconsStyleRepository()->loadByKey(cbIconStyle->currentData().toString(), icon);

    FontConfig font;
    bool hasFont = m_styleManager->getTypographyRepository()->loadByKey(cbTypography->currentData().toString(), font);

    StyleMetricsConfig metrics;
    bool hasMetrics = m_styleManager->getMetricsRepository()->loadByKey(cbActiveMetrics->currentData().toString(), metrics);

    if (m_styleManager->exportProfile(path, profileName.trimmed(),
                                      hasSkin ? &skin : nullptr, 
                                      hasIcon ? &icon : nullptr, 
                                      hasFont ? &font : nullptr, 
                                      hasMetrics ? &metrics : nullptr)) {
        QMessageBox::information(this, tr("Profile Exported"), tr("Workspace profile '%1' has been successfully exported.").arg(profileName));
    } else {
        QMessageBox::critical(this, tr("Error"), tr("Could not export the workspace profile."));
    }
}

void LC_WidgetOptionsDialog::updateSkinComboFonts() const {
    LC_EditorUtils::updatePresetComboFonts(cbFusionSkin, m_styleManager->getActiveSkin());
}

void LC_WidgetOptionsDialog::updateIconStyleComboFonts() const {
    LC_EditorUtils::updatePresetComboFonts(cbIconStyle, m_styleManager->getActiveIconStyle());
}


void LC_WidgetOptionsDialog::updateTypographyComboFonts() const {
    LC_EditorUtils::updatePresetComboFonts(cbTypography, m_styleManager->getActiveTypography());
}

void LC_WidgetOptionsDialog::updateMetricsComboFonts() const {
    LC_EditorUtils::updatePresetComboFonts(cbActiveMetrics, m_styleManager->getActiveMetrics());
}

void LC_WidgetOptionsDialog::reject() {
    // Transactional Rollback: Restore original active selections to discard on-the-fly previews
    m_styleManager->setStyleAllowed(m_origAllowStyle);
    m_styleManager->setActiveStyle(m_origStyle);
    m_styleManager->setThemeModeOverride(m_origThemeMode);
    m_styleManager->setActiveStyleSheet(m_origStyleSheet);

    m_styleManager->setActiveSkin(m_origSkinKey);
    m_styleManager->setActiveIconStyle(m_origIconStyleKey);
    m_styleManager->setActiveTypography(m_origTypographyKey);
    m_styleManager->setActiveMetrics(m_origMetricsKey);

    m_styleManager->setIgnoreIconStylingInTheme(m_origIgnoreIconStyling);

    m_styleManager->applyActiveStyleAndTheme();

    LC_Dialog::reject();
}

void LC_WidgetOptionsDialog::setupMegaCADBarSettingUI() const {
    using namespace CFG_Widgets;
    const bool cadSidebarUngrouped = CFG_Startup::o_CADSideBarUngrouped;
    gbCADWidgets->setEnabled(!cadSidebarUngrouped);
    gbCADWidgetsUngrouped->setEnabled(cadSidebarUngrouped);

    const bool leftToolbarAllFlatIcons = o_LeftToolbarAllFlatIcons;
    cbLeftTBAllFlatButtons->setChecked(leftToolbarAllFlatIcons);

    const int leftToolbarIconSize = o_LeftToolbarIconSize;
    sbLeftTBIconSize->setValue(leftToolbarIconSize);

    const int leftToolbarAllIconSize = o_LeftToolbarAllIconSize;
    sbLeftTBAllIconSize->setValue(leftToolbarAllIconSize);
}

void LC_WidgetOptionsDialog::setupCADBarSettingsUI() const {
    using namespace CFG_Widgets;

    const int leftToolbarColumnsCount = o_LeftToolbarColumnsCount;
    sbLeftTBColumnCount->setValue(leftToolbarColumnsCount);

    const int leftToolbarAllColumnsCount = o_LeftToolbarAllColumnsCount;
    sbLeftTBAllColumnCount->setValue(leftToolbarAllColumnsCount);

    const bool leftToolbarFlatIcons = o_LeftToolbarFlatIcons;
    cbLeftTBFlatButtons->setChecked(leftToolbarFlatIcons);
}

void LC_WidgetOptionsDialog::setupDockWidgetSettingsUI() const {
    using namespace CFG_Widgets;

    const bool dockWidgetsFlatIcons = o_DockWidgetsFlatIcons;
    cbDockWidgetsFlatButtons->setChecked(dockWidgetsFlatIcons);

    const bool pickValuesButtonsFlatIcons = o_PickValueButtonsFlatIcons;
    cbFlatPickValuesButtons->setChecked(pickValuesButtonsFlatIcons);
}

void LC_WidgetOptionsDialog::setupToolbarsSettingsUI() const {
    using namespace CFG_Widgets;
    const bool allow_toolbar_icon_size = o_AllowToolbarIconSize;
    toolbar_icon_size_checkbox->setChecked(allow_toolbar_icon_size);

    const int toolbar_icon_size = o_ToolbarIconSize;
    toolbar_icon_size_spinbox->setValue(toolbar_icon_size);
}

void LC_WidgetOptionsDialog::setupGenericTabUI() const {
    setupToolbarsSettingsUI();
    setupCADBarSettingsUI();
    setupDockWidgetSettingsUI();
    setupDockingSettingsUI();
    setupMegaCADBarSettingUI();
    setupStatusBarSettingsUI();
}

void LC_WidgetOptionsDialog::setupDockingSettingsUI() const {
    using namespace CFG_Widgets;
    const int docWidgetsIconSize = o_DockWidgetsIconSize;
    sbDocWidgtetIconSize->setValue(docWidgetsIconSize);

    const bool allowDockNesting = o_DockAllowNested;
    cbDockingAllowNested->setChecked(allowDockNesting);

    const bool titleBarVertical = o_DockTitleBarVertical;
    cbDockingVerticalTitleBar->setChecked(titleBarVertical);

    const bool verticalTabs = o_DockVerticalTabs;
    cbDockingVerticalTabs->setChecked(verticalTabs);
}

void LC_WidgetOptionsDialog::setupStatusBarSettingsUI() const {
    {
        using namespace CFG_Widgets;
        const bool allow_statusbar_height = o_AllowStatusbarHeight;
        statusbar_height_checkbox->setChecked(allow_statusbar_height);

        const int statusbar_height = o_StatusbarHeight;
        statusbar_height_spinbox->setValue(statusbar_height);

        const bool allow_statusbar_fontsize = o_AllowStatusbarFontSize;
        statusbar_fontsize_checkbox->setChecked(allow_statusbar_fontsize);

        const int statusbar_fontsize = o_StatusbarFontSize;
        statusbar_fontsize_spinbox->setValue(statusbar_fontsize);
    }

    const bool useClassicalStatusBar = CFG_Startup::o_UseClassicStatusBar;

    statusbar_height_spinbox->setEnabled(useClassicalStatusBar);
    statusbar_height_checkbox->setEnabled(useClassicalStatusBar);
    statusbar_fontsize_checkbox->setEnabled(useClassicalStatusBar);
    statusbar_fontsize_spinbox->setEnabled(useClassicalStatusBar);
}

QString LC_WidgetOptionsDialog::selectFolder(const QString& title) {
    QString folder = nullptr;
    QFileDialog dlg(this);
    if (title != nullptr) {
        const QString dlgTitle = title;
        dlg.setWindowTitle(dlgTitle);
    }
    dlg.setFileMode(QFileDialog::Directory);
    dlg.setOption(QFileDialog::ShowDirsOnly);

    if (dlg.exec() != 0) {
        folder = dlg.selectedFiles().at(0);
    }
    return folder;
}

void LC_WidgetOptionsDialog::accept() {
    if (m_styleManager) {
        m_styleManager->setStyleAllowed(style_checkbox->isChecked());
        m_styleManager->setActiveStyle(style_combobox->currentText());
        m_styleManager->setThemeModeOverride(static_cast<ThemeModeOverride>(cbThemeModeOverride->currentData().toInt()));
        m_styleManager->setActiveStyleSheet(stylesheet_field->text());

        // Save selections as stable keys (filenames without paths) to protect I/O
        m_styleManager->setActiveSkin(cbFusionSkin->currentData().toString());
        m_styleManager->setActiveIconStyle(cbIconStyle->currentData().toString());
        m_styleManager->setActiveTypography(cbTypography->currentData().toString());
        m_styleManager->setActiveMetrics(cbActiveMetrics->currentData().toString());

        m_styleManager->setIgnoreIconStylingInTheme(cbIgnoreIconStylingInTheme->isChecked());
        m_styleManager->applyActiveStyleAndTheme();
    }

    // 4. Save low-level local CAD widget preferences

    {
        using namespace  CFG_Widgets;
        const bool pickValuesButtonsFlatIcons = cbFlatPickValuesButtons->isChecked();
        o_PickValueButtonsFlatIcons = pickValuesButtonsFlatIcons;

        const bool allow_theme = false;
        CFG_AppStyling::o_AllowTheme = allow_theme;

        const bool allow_toolbar_icon_size = toolbar_icon_size_checkbox->isChecked();
        o_AllowToolbarIconSize = allow_toolbar_icon_size;
        const auto& appWindow = QC_ApplicationWindow::getAppWindow();
        if (allow_toolbar_icon_size && appWindow != nullptr) {
            const int toolbar_icon_size = toolbar_icon_size_spinbox->value();
            o_ToolbarIconSize = toolbar_icon_size;
            appWindow->setIconSize(QSize(toolbar_icon_size, toolbar_icon_size));
        }

        const bool allow_statusbar_fontsize = statusbar_fontsize_checkbox->isChecked();
        o_AllowStatusbarFontSize = allow_statusbar_fontsize;
        if (allow_statusbar_fontsize && appWindow != nullptr) {
            const int statusbar_fontsize = statusbar_fontsize_spinbox->value();
            o_StatusbarFontSize = statusbar_fontsize;
            QFont font;
            font.setPointSize(statusbar_fontsize);
            appWindow->statusBar()->setFont(font);
        }

        const bool allow_statusbar_height = statusbar_height_checkbox->isChecked();
        o_AllowStatusbarHeight =  allow_statusbar_height;
        if (allow_statusbar_height && appWindow != nullptr) {
            const int statusbar_height = statusbar_height_spinbox->value();
            o_StatusbarHeight = statusbar_height;
            appWindow->statusBar()->setMinimumHeight(statusbar_height);
        }

        o_LeftToolbarColumnsCount  = sbLeftTBColumnCount->value();
        o_LeftToolbarAllColumnsCount  = sbLeftTBAllColumnCount->value();

        o_LeftToolbarFlatIcons  = cbLeftTBFlatButtons->isChecked();
        o_LeftToolbarAllFlatIcons =  cbLeftTBAllFlatButtons->isChecked();

        o_LeftToolbarIconSize = sbLeftTBIconSize->value();
        o_LeftToolbarAllIconSize = sbLeftTBAllIconSize->value();

        o_DockWidgetsFlatIcons = cbDockWidgetsFlatButtons->isChecked();
        o_DockWidgetsIconSize =  sbDocWidgtetIconSize->value();

        const bool allowDockNesting = cbDockingAllowNested->isChecked();
        o_DockAllowNested = allowDockNesting;

        const bool titleBarVertical = cbDockingVerticalTitleBar->isChecked();
        o_DockTitleBarVertical = titleBarVertical;

        const bool verticalTabs = cbDockingVerticalTabs->isChecked();
        o_DockVerticalTabs = verticalTabs;

        if (appWindow != nullptr) {
            LC_WidgetFactory::updateDockOptions(appWindow.get(), allowDockNesting, verticalTabs);
            LC_WidgetFactory::updateDockWidgetsTitleBarType(appWindow.get(), titleBarVertical);
        }
    }
    LC_GROUP_END();

    // 5. Commit baseline icon colors and custom directory configurations
    const QString iconsOverrideDir = leIconsOverrideDir->text();
    m_iconColorsOptions.setIconsOverridesDir(iconsOverrideDir);
    m_iconColorsOptions.save();

    if (m_iconColorsOptions.isIconOverridesChanged()) {
        QMessageBox::warning(this, tr("Preferences"),
                             tr("Icons overrides directory changed. Please restart the application to apply."));
    }

    LC_Dialog::accept();
}

/*void LC_WidgetOptionsDialog::reject() {
    // 1. Terminate any outstanding asynchronous loading loops

    const auto& appWindow = QC_ApplicationWindow::getAppWindow();

    // 2. Re-apply the original visual parameters to revert the application background
    if (m_styleManager) {
        if (m_origAllowStyle && !m_origStyle.isEmpty()) {
            QApplication::setStyle(QStyleFactory::create(m_origStyle));

            if (m_origStyle == "Fusion") {
                bool isDarkMode = false;
                if (m_origThemeMode == 2) { // ForceDark
                    isDarkMode = true;
                } else if (m_origThemeMode == 1) { // ForceLight
                    isDarkMode = false;
                } else {
                    isDarkMode = LC_PaletteColorUtils::isSystemInDarkMode();
                }

                if (!m_origSkinName.isEmpty() && m_origSkinName != "Default") {
                    FusionSkinConfig config;
                    if (m_styleManager->loadFusionSkinByName(m_origSkinName, config)) {
                        // Apply the original theme. Override fonts if the original setting required it.
                        if (m_origIgnoreThemeTypography) {
                            config.font = m_origFontConfig;
                        }
                        m_styleManager->applyThemeToApplication(config, isDarkMode);
                    }
                } else {
                    m_styleManager->applyCleanFusionTheme();
                }
            } else {
                // Non-Fusion native styles
                QApplication::setPalette(QApplication::style()->standardPalette());
            }
        } else {
            m_styleManager->applyCleanFusionTheme();
        }
    }

    // 3. Restore the original stylesheet path in the active UI on Cancel
    if (appWindow != nullptr) {
        m_styleManager->loadStyleSheet(m_origStyleSheet);
    }

    // 4. Restore the original in-memory icon color options and re-apply
    m_iconColorsOptions.restore();
    if (m_styleManager) {
        if (m_origIconStyleName == "Default") {
            bool isDarkMode = (m_origThemeMode == 2) || (m_origThemeMode == 0 && LC_PaletteColorUtils::isSystemInDarkMode());
            LC_IconsStyleManager::applyThemeLinkedIcons("Default", true, isDarkMode);
        } else {
            IconStyleConfig iconStyle;
            if (m_styleManager->loadIconStyle(m_origIconStyleName, iconStyle)) {
                bool isDarkMode = (m_origThemeMode == 2) || (m_origThemeMode == 0 && LC_PaletteColorUtils::isSystemInDarkMode());
                LC_IconColorsOptions iconOptions;
                iconOptions.loadSettings();
                iconOptions.importStyleConfig(iconStyle, isDarkMode);
                LC_IconsStyleManager::applyStyle(iconOptions, isDarkMode);
            }
        }
    }

    LC_Dialog::reject();
}*/



void LC_WidgetOptionsDialog::chooseStyleSheet(){
    const QString path = QFileDialog::getOpenFileName(this);
    if (!path.isEmpty()){
        stylesheet_field->setText(QDir::toNativeSeparators(path));
        m_styleManager->applyActiveIconStyle();;
    }
}

void LC_WidgetOptionsDialog::setIconsOverrideFolder() {
    const QString folder = selectFolder(tr("Select External Icons Folder"));
    if (folder != nullptr) {
        leIconsOverrideDir->setText(QDir::toNativeSeparators(folder));
    }
}

void LC_WidgetOptionsDialog::applyTransientStylePreview() {
    const bool allowStyle = style_checkbox->isChecked();
    const auto styleName = style_combobox->currentText();
    const auto skinKey = cbFusionSkin->currentData().toString();
    const auto metricsKey = cbActiveMetrics->currentData().toString();
    const auto typographyKey = cbTypography->currentData().toString();
    const auto iconStyleKey = cbIconStyle->currentData().toString();
    const auto themeModeOverride = static_cast<ThemeModeOverride>(cbThemeModeOverride->currentData().toInt());
        m_styleManager->applyTransientTheme(allowStyle, styleName,
                                        skinKey, metricsKey,
                                        typographyKey, iconStyleKey,
                                        themeModeOverride);

        updateSkinComboFonts();
        updateIconStyleComboFonts();
        updateTypographyComboFonts();
        updateMetricsComboFonts();
}
