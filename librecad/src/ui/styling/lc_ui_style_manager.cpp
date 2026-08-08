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

#include "lc_ui_style_manager.h"
#include <QApplication>
#include <QStyleFactory>
#include <QCoreApplication>

#include "lc_custom_style_helper.h"
#include "lc_palette_color_utils.h"
#include "lc_icons_style_manager.h"
#include "lc_proxy_style.h"
#include "lc_fusion_skins_repository.h"
#include "lc_icons_style_repository.h"
#include "lc_metrics_repository.h"
#include "lc_settings_app_styling.h"
#include "lc_settings_paths.h"
#include "lc_settings_types.h"
#include "lc_settings_widget.h"
#include "lc_style_metrics_utils.h"
#include "lc_typography_repository.h"
#include "lc_typography_utils.h"
#include "lc_workspace_import_export_helper.h"
#include "qc_applicationwindow.h"
#include "rs_settings.h"
#include "rs_system.h"

LC_UIStyleManager::LC_UIStyleManager() {
    m_fusionSkinsRepository.reset(nullptr);
    m_iconStylesRepository.reset(nullptr);
}

LC_UIStyleManager::~LC_UIStyleManager() = default;

QString LC_UIStyleManager::getStyleConfigurationBaseDir() const {
    // Tier 1: Check for a direct user override configured in settings
    QString styleDir = LC_GET_ONE_STR("Paths", "StyleConfigDir", "").trimmed();
    if (!styleDir.isEmpty() && QDir(styleDir).exists()) {
        return QDir::toNativeSeparators(styleDir);
    }

    // Tier 2: Check inside the user's personal settings or app data directory
    QString otherSettings = CFG_Paths::o_OtherSettingsDir;

    styleDir = otherSettings.trimmed() + "/styling";
    if (QDir(styleDir).exists()) {
        return QDir::toNativeSeparators(styleDir);
    }

    // Tier 3: Symmetrical Fallback to the installation's bundled default directory
    return QDir::toNativeSeparators(QCoreApplication::applicationDirPath() + "/styling");
}


void LC_UIStyleManager::initializeRepositories() {
    QString baseDir = getStyleConfigurationBaseDir();

    // Initialize repositories with their specific subdirectories under the base directory
    m_fusionSkinsRepository = std::make_unique<LC_FusionSkinsRepository>(baseDir + "/skins");
    m_iconStylesRepository  = std::make_unique<LC_IconsStyleRepository>(baseDir + "/icons");
    m_typographyRepository  = std::make_unique<LC_TypographyRepository>(baseDir + "/typography");
    m_metricsRepository     = std::make_unique<LC_MetricsRepository>(baseDir + "/metrics");

    m_importExportHelper    = std::make_unique<LC_WorkspaceImportExportHelper>(
        m_fusionSkinsRepository.get(),
        m_iconStylesRepository.get(),
        m_typographyRepository.get(),
        m_metricsRepository.get()
    );

    // Perform synchronous index handshakes for all repositories
    m_fusionSkinsRepository->initializeIndex();
    m_iconStylesRepository->initializeIndex();
    m_typographyRepository->initializeIndex();
    m_metricsRepository->initializeIndex();
}

void LC_UIStyleManager::reloadStyleConfiguration() {
    initializeRepositories();
    applyActiveStyleAndTheme();
}

// --- Centralized Application Boot Initializer ---
void LC_UIStyleManager::initialize(QC_ApplicationWindow* appWindow) {
    initializeRepositories();

    m_customStyleHelper = std::make_unique<LC_CustomStyleHelper>(appWindow);
    bool allowStyle = isStyleAllowed();
    QString activeStyleName = getActiveStyle();

    if (allowStyle && !activeStyleName.isEmpty()) {
        if (activeStyleName.toLower() == "fusion") {
            bool isDarkMode = resolveIsDarkMode();

            SkinConfig skin;
            StyleMetricsConfig metrics;
            FontConfig font;

            // Load configurations individually with fallback defaults
            QString activeSkin = getActiveSkin();
            if (activeSkin.isEmpty() || !m_fusionSkinsRepository->loadByKey(activeSkin, skin)) {
                LC_PaletteColorUtils::initializeDefaultConfig(skin);
            }

            QString activeMetrics = getActiveMetrics();
            if (activeMetrics.isEmpty() || !m_metricsRepository->loadByKey(activeMetrics, metrics)) {
                LC_StyleMetricsUtils::initializeDefault(metrics);
            }

            QString activeTypography = getActiveTypography();
            if (activeTypography.isEmpty() || !m_typographyRepository->loadByKey(activeTypography, font)) {
                LC_TypographyUtils::initializeDefaultConfig(font);
            }

            applyThemeToApplication(skin, metrics, font, isDarkMode);
        } else {
            // Symmetrical: load native non-Fusion styles
            QStyle* nativeStyle = QStyleFactory::create(activeStyleName);
            if (nativeStyle) {
                QApplication::setStyle(nativeStyle);
                QApplication::setPalette(nativeStyle->standardPalette());
            }
            qApp->setStyleSheet("");

            // Apply global typography and icons on non-Fusion platforms
            applyGlobalTypographyAndIcons();
        }
    } else {
        // Style disabled fallback
        applyGlobalTypographyAndIcons();
    }
}
bool LC_UIStyleManager::isStyleAllowed() const {
    return CFG_AppStyling::o_AllowStyle;
}
void LC_UIStyleManager::setStyleAllowed(bool allowed) {
    CFG_AppStyling::o_AllowStyle = allowed;
}

void LC_UIStyleManager::setActiveStyle(const QString &style) {
    CFG_AppStyling::o_Style = style;
}

QString LC_UIStyleManager::getActiveStyle() const {
    return CFG_AppStyling::o_Style;
}

ThemeModeOverride LC_UIStyleManager::getThemeModeOverride() const {
    return CFG_AppStyling::o_ThemeModeOverride;
}
void LC_UIStyleManager::setThemeModeOverride(ThemeModeOverride mode) {
    CFG_AppStyling::o_ThemeModeOverride = mode;
}

QString LC_UIStyleManager::getActiveStyleSheet() const {
    return CFG_AppStyling::o_StyleSheet;
}
void LC_UIStyleManager::setActiveStyleSheet(const QString &sheet) {
    CFG_AppStyling::o_StyleSheet = sheet;
}

bool LC_UIStyleManager::getIgnoreIconStylingInTheme() const {
    return LC_GET_ONE_BOOL("Widgets", "IgnoreIconStylingInTheme", false);
}
void LC_UIStyleManager::setIgnoreIconStylingInTheme(bool ignore) {
    LC_SET_ONE("Widgets", "IgnoreIconStylingInTheme", ignore);
}

QString LC_UIStyleManager::getActiveIconStyle() const {
    return LC_GET_ONE_STR("UiIconsStyling", "Style", "Default");
}
void LC_UIStyleManager::setActiveIconStyle(const QString &style) {
    LC_SET_ONE("UiIconsStyling", "Style", style);
}

QString LC_UIStyleManager::getIconsOverridesDir() const {
    return LC_GET_ONE_STR("UiIconsStyling", "IconOverridesDir", "");
}
void LC_UIStyleManager::setIconsOverridesDir(const QString &dir) {
    LC_SET_ONE("UiIconsStyling", "IconOverridesDir", dir);
}

QString LC_UIStyleManager::getActiveSkin() const {
    return CFG_AppStyling::o_ActiveSkinName;
}
void LC_UIStyleManager::setActiveSkin(const QString& name) {
    CFG_AppStyling::o_ActiveSkinName = name;
}

QString LC_UIStyleManager::getActiveTypography() const {
    return CFG_AppStyling::o_ActiveTypographyName;
}
void LC_UIStyleManager::setActiveTypography(const QString& name) {
    CFG_AppStyling::o_ActiveTypographyName = name;
}

QString LC_UIStyleManager::getActiveMetrics() const {
    return CFG_AppStyling::o_ActiveMetricsName;
}
void LC_UIStyleManager::setActiveMetrics(const QString& name) {
    CFG_AppStyling::o_ActiveMetricsName = name;
}

void LC_UIStyleManager::loadIconColorsOptions(LC_IconColorsOptions &options) const {
    options.loadSettings();
}
void LC_UIStyleManager::saveIconColorsOptions(LC_IconColorsOptions &options) const {
    options.save();
}

// --- Import/Export Profile Facade ---
bool LC_UIStyleManager::importProfile(const QString& importFilePath,
                                      QString& outProfileName,
                                      QString& outSkinName,
                                      QString& outIconStyleName,
                                      QString& outTypographyName,
                                      QString& outMetricsName) const {
    if (m_importExportHelper) {
        return m_importExportHelper->importProfile(importFilePath,
                                                   outProfileName,
                                                   outSkinName,
                                                   outIconStyleName,
                                                   outTypographyName,
                                                   outMetricsName);
    }
    return false;
}

bool LC_UIStyleManager::exportProfile(const QString& exportFilePath,
                                      const QString& profileName,
                                      const SkinConfig* skin,
                                      const IconStyleConfig* icon,
                                      const FontConfig* font,
                                      const StyleMetricsConfig* metrics) const {
    if (m_importExportHelper) {
        return m_importExportHelper->exportProfile(exportFilePath, profileName, skin, icon, font, metrics);
    }
    return false;
}

// --- High-Level Theme & Style Application Actions ---
bool LC_UIStyleManager::applyThemeToApplication(const SkinConfig &skin,
                                                const StyleMetricsConfig &metrics,
                                                const FontConfig &font,
                                                bool isDarkMode) {
    const ColorSchemeData &scheme = isDarkMode ? skin.dark : skin.light;

    // 1. Build the custom QPalette using the skin's colors and visual style archetype
    const QPalette palette = LC_PaletteColorUtils::createPaletteFromScheme(
        scheme, skin.styleArchetype, LC_PaletteColorUtils::CVDType::Normal
    );

    // 2. Set the custom palette FIRST so that widgets being recursively polished
    // inside QApplication::setStyle() resolve the correct custom colors.
    QApplication::setPalette(palette);

    // 3. Set the dynamic proxy style and configure metrics
    QStyle *baseStyle = QStyleFactory::create("Fusion");
    auto* proxyStyle = new LC_ProxyStyle(baseStyle, metrics);
    proxyStyle->setSkin(skin);
    proxyStyle->setFont(font);
    QApplication::setStyle(proxyStyle);

    // 4. Set the custom palette AGAIN because setStyle() internally resets
    // the application-wide palette to the style's standardPalette() to match guidelines.
    QApplication::setPalette(palette);

    // 5. Apply typography offsets
    applyThemeTypography(font);

    // 6. Set QCSS overrides
    qApp->setStyleSheet(scheme.qss);

    // 7. Apply icons styling
    auto linkedIconsTheme = skin.useThemeDefaultIcons ? skin.linkedIconStyleName : "";
    applyActiveOrThemeIconStyle(linkedIconsTheme, isDarkMode);

    // 8. Safely trigger an update on all top-level windows to paint with the correct style
    for (QWidget *widget : QApplication::topLevelWidgets()) {
        widget->update();
    }

    return true;
}

void LC_UIStyleManager::applyActiveStyleAndTheme() {
    bool allowStyle = isStyleAllowed();
    if (allowStyle) {
        QString styleName = getActiveStyle();
        if (styleName == "Fusion") {
            applyActiveThemeOverride();
        } else {
            QStyle *nativeStyle = QStyleFactory::create(styleName);
            if (nativeStyle) {
                QApplication::setStyle(nativeStyle);
                QApplication::setPalette(nativeStyle->standardPalette());
            }
            qApp->setStyleSheet("");
            applyGlobalTypographyAndIcons();
        }
    } else {
        applyGlobalTypographyAndIcons();
    }
}

void LC_UIStyleManager::applyActiveThemeOverride() {
    bool allowStyle = isStyleAllowed();
    if (!allowStyle) return;

    QString styleName = getActiveStyle();
    if (styleName != "Fusion") return;

    bool isDarkMode = resolveIsDarkMode();

    SkinConfig skin;
    StyleMetricsConfig metrics;
    FontConfig font;

    QString activeSkin = getActiveSkin();
    if (activeSkin.isEmpty() || !m_fusionSkinsRepository->loadByKey(activeSkin, skin)) {
        LC_PaletteColorUtils::initializeDefaultConfig(skin);
    }

    QString activeMetrics = getActiveMetrics();
    if (activeMetrics.isEmpty() || !m_metricsRepository->loadByKey(activeMetrics, metrics)) {
        LC_StyleMetricsUtils::initializeDefault(metrics);
    }

    QString activeTypography = getActiveTypography();
    if (activeTypography.isEmpty() || !m_typographyRepository->loadByKey(activeTypography, font)) {
        LC_TypographyUtils::initializeDefaultConfig(font);
    }

    applyThemeToApplication(skin, metrics, font, isDarkMode);
}

void LC_UIStyleManager::applyActiveIconStyle() const {
    bool isDarkMode = resolveIsDarkMode();
    QString activeIconStyle = "Default";
    bool ignoreIconStyleInSkin = getIgnoreIconStylingInTheme();

    if (ignoreIconStyleInSkin) {
        activeIconStyle = getActiveIconStyle();
    } else {
        activeIconStyle = getActiveIconStyle();
    }

    if (activeIconStyle == "Default") {
        LC_IconsStyleManager::applyThemeLinkedIcons("Default", true, isDarkMode);
    } else {
        IconStyleConfig iconStyle;
        if (m_iconStylesRepository->loadByKey(activeIconStyle, iconStyle)) {
            LC_IconColorsOptions iconOptions;
            iconOptions.loadSettings();
            iconOptions.importStyleConfig(iconStyle, isDarkMode);
            LC_IconsStyleManager::applyStyle(iconOptions, isDarkMode);
        }
    }
}

void LC_UIStyleManager::applyTransientTheme(bool allowStyle,
                         const QString& styleName,
                         const QString& skinKey,
                         const QString& metricsKey,
                         const QString& typographyKey,
                         const QString& iconStyleKey,
                         ThemeModeOverride themeModeOverride) {


    bool isDarkMode = false;

    if (allowStyle && styleName.toLower() == "fusion") {
        SkinConfig skin;
        StyleMetricsConfig metrics;
        FontConfig font;

        if (themeModeOverride == ThemeModeOverride::ForceDark) {
            isDarkMode = true;
        } else if (themeModeOverride == ThemeModeOverride::ForceLight) {
            isDarkMode = false;
        } else {
            // Fallback to checking the system standard color scheme
            isDarkMode = LC_PaletteColorUtils::isSystemInDarkMode();
        }


        // 1. Resolve and load the Skin configuration from repository
        if (skinKey.isEmpty() || skinKey == DEFAULT_THEME_KEY || !m_fusionSkinsRepository->loadByKey(skinKey, skin)) {
            LC_PaletteColorUtils::initializeDefaultConfig(skin);
        }

        // 2. Resolve and load the Metrics configuration from repository
        if (metricsKey.isEmpty() || metricsKey == DEFAULT_THEME_KEY || !m_metricsRepository->loadByKey(metricsKey, metrics)) {
            LC_StyleMetricsUtils::initializeDefault(metrics);
        }

        // 3. Resolve and load the Typography configuration from repository
        if (typographyKey.isEmpty() || typographyKey == DEFAULT_THEME_KEY || !m_typographyRepository->loadByKey(typographyKey, font)) {
            LC_TypographyUtils::initializeDefaultConfig(font);
        }

        // 4. Apply the computed 3-tuple configuration globally
        applyThemeToApplication(skin, metrics, font, isDarkMode);
    } else {
        // Native platform styles
        QStyle* nativeStyle = QStyleFactory::create(styleName);
        if (nativeStyle) {
            QApplication::setStyle(nativeStyle);
            QApplication::setPalette(nativeStyle->standardPalette());
        }
        qApp->setStyleSheet("");

        // Typography still applies globally on native styles
        FontConfig font;
        if (typographyKey.isEmpty() || typographyKey == DEFAULT_THEME_KEY || !m_typographyRepository->loadByKey(typographyKey, font)) {
            LC_TypographyUtils::initializeDefaultConfig(font);
        }
        applyThemeTypography(font);

        isDarkMode = LC_PaletteColorUtils::isPaletteDarkMode();
    }

    // Apply the requested Icon Style globally
    applyActiveOrThemeIconStyle(iconStyleKey, isDarkMode);
}

void LC_UIStyleManager::applyActiveOrThemeIconStyle(const QString& themeLinkedIconStyleName, bool isDarkMode) const {
    bool ignoreIconStyleInSkin = getIgnoreIconStylingInTheme();
    QString activeIconStyle = "Default";

    if (ignoreIconStyleInSkin) {
        activeIconStyle = getActiveIconStyle();
    } else {
        if (themeLinkedIconStyleName.isEmpty()) {
            activeIconStyle = getActiveIconStyle();
        }
        else {
            activeIconStyle = themeLinkedIconStyleName;
        }
    }

    if (activeIconStyle == "Default") {
        LC_IconsStyleManager::applyThemeLinkedIcons("Default", true, isDarkMode);
    } else {
        IconStyleConfig iconStyle;
        if (m_iconStylesRepository->loadByKey(activeIconStyle, iconStyle)) {
            LC_IconColorsOptions iconOptions;
            iconOptions.loadSettings();
            iconOptions.importStyleConfig(iconStyle, isDarkMode);
            LC_IconsStyleManager::applyStyle(iconOptions, isDarkMode);
        }
    }
}

void LC_UIStyleManager::applyActiveOrDefaultIconStyle(bool isDarkMode) const {
    QString activeIconStyle = getActiveIconStyle();

    if (activeIconStyle == "Default") {
        LC_IconsStyleManager::applyThemeLinkedIcons("Default", true, isDarkMode);
    } else {
        IconStyleConfig iconStyle;
        if (m_iconStylesRepository->loadByKey(activeIconStyle, iconStyle)) {
            LC_IconColorsOptions iconOptions;
            iconOptions.loadSettings();
            iconOptions.importStyleConfig(iconStyle, isDarkMode);
            LC_IconsStyleManager::applyStyle(iconOptions, isDarkMode);
        }
    }
}

void LC_UIStyleManager::applyActiveStyleSheet() const {
    loadStyleSheet(getActiveStyleSheet());
}

void LC_UIStyleManager::reloadStyleSheet() const {
    applyActiveStyleSheet();
}

void LC_UIStyleManager::loadStyleSheet(const QString& stylesheetFilePath) const {
    m_customStyleHelper->loadStyleSheet(stylesheetFilePath);
}

bool LC_UIStyleManager::resolveIsDarkMode() const {
    ThemeModeOverride themeMode = getThemeModeOverride();
    if (themeMode == ThemeModeOverride::ForceDark) {
        return true;
    } else if (themeMode == ThemeModeOverride::ForceLight) {
        return false;
    }
    return LC_PaletteColorUtils::isSystemInDarkMode();
}

void LC_UIStyleManager::applyGlobalTypographyAndIcons() {
    FontConfig font;
    QString activeTypography = getActiveTypography();
    if (activeTypography.isEmpty() || !m_typographyRepository->loadByKey(activeTypography, font)) {
        LC_TypographyUtils::initializeDefaultConfig(font);
    }
    applyThemeTypography(font);

    bool isDarkMode = resolveIsDarkMode();
    applyActiveOrThemeIconStyle("", isDarkMode);
}

void LC_UIStyleManager::applyThemeTypography(const FontConfig &activeFont) {
    // Role 1: Main Interface Base
    const QFont mainFont(activeFont.mainFamily, activeFont.mainSize);
    QApplication::setFont(mainFont);

    // Role 2: Section Headings
    QFont headingFont(activeFont.mainFamily, activeFont.mainSize + activeFont.headings.sizeOffset);
    headingFont.setBold(activeFont.headings.bold);
    headingFont.setItalic(activeFont.headings.italic);
    QApplication::setFont(headingFont, "QHeaderView");
    QApplication::setFont(headingFont, "QTabBar");

    // Role 3: Main Menu Bar
    QFont menuBarFont(activeFont.mainFamily, activeFont.mainSize + activeFont.menuBar.sizeOffset);
    menuBarFont.setBold(activeFont.menuBar.bold);
    menuBarFont.setItalic(activeFont.menuBar.italic);
    QApplication::setFont(menuBarFont, "QMenuBar");

    // Role 4: Popup & Dropdown Menus
    QFont menuFont(activeFont.mainFamily, activeFont.mainSize + activeFont.menus.sizeOffset);
    menuFont.setBold(activeFont.menus.bold);
    menuFont.setItalic(activeFont.menus.italic);
    QApplication::setFont(menuFont, "QMenu");

    // Role 5: Buttons & Toolbars
    QFont buttonFont(activeFont.mainFamily, activeFont.mainSize + activeFont.buttons.sizeOffset);
    buttonFont.setBold(activeFont.buttons.bold);
    buttonFont.setItalic(activeFont.buttons.italic);
    QApplication::setFont(buttonFont, "QPushButton");
    QApplication::setFont(buttonFont, "QToolButton");
    QApplication::setFont(buttonFont, "QToolBar");

    // Role 6: Inputs & Editors
    QFont inputFont(activeFont.mainFamily, activeFont.mainSize + activeFont.inputs.sizeOffset);
    inputFont.setBold(activeFont.inputs.bold);
    inputFont.setItalic(activeFont.inputs.italic);
    QApplication::setFont(inputFont, "QLineEdit");
    QApplication::setFont(inputFont, "QComboBox");
    QApplication::setFont(inputFont, "QAbstractSpinBox");

    // Role 7: Code & Technical
    QFont techFont(activeFont.techFamily, activeFont.mainSize + activeFont.technical.sizeOffset);
    techFont.setBold(activeFont.technical.bold);
    techFont.setItalic(activeFont.technical.italic);
    QApplication::setFont(techFont, "QTextEdit");
    QApplication::setFont(techFont, "QPlainTextEdit");
}
