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
#include "lc_palette_repository.h"
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
    const QString baseDir = getStyleConfigurationBaseDir();

    m_paletteRepository     = std::make_unique<LC_PaletteRepository>(baseDir + "/palettes");
    m_fusionSkinsRepository = std::make_unique<LC_FusionSkinsRepository>(baseDir + "/skins");
    m_iconStylesRepository  = std::make_unique<LC_IconsStyleRepository>(baseDir + "/icons");
    m_typographyRepository  = std::make_unique<LC_TypographyRepository>(baseDir + "/typography");
    m_metricsRepository     = std::make_unique<LC_MetricsRepository>(baseDir + "/metrics");

    m_paletteRepository->initializeIndex();
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

    if (QApplication::style() != nullptr) {
        m_nativeSystemStyleName = QApplication::style()->objectName();
        m_nativeSystemPalette   = QApplication::style()->standardPalette();
    }

    m_customStyleHelper = std::make_unique<LC_CustomStyleHelper>(appWindow);

    applyActiveStyleAndTheme();

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

QString LC_UIStyleManager::getActivePalette() const {
    return CFG_AppStyling::o_ActivePalette;
}

void LC_UIStyleManager::setActivePalette(const QString& name) {
    CFG_AppStyling::o_ActivePalette = name;
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
                                      QString& outPaletteName,
                                      QString& outControlStyleName,
                                      QString& outIconStyleName,
                                      QString& outTypographyName,
                                      QString& outMetricsName) const {
    if (m_importExportHelper) {
        return m_importExportHelper->importProfile(importFilePath,
                                                   outProfileName,
                                                   outPaletteName,
                                                   outControlStyleName,
                                                   outIconStyleName,
                                                   outTypographyName,
                                                   outMetricsName);
    }
    return false;
}

bool LC_UIStyleManager::exportProfile(const QString& exportFilePath,
                                      const QString& profileName,
                                      const PaletteConfig* palette,
                                      const ControlStyleConfig* controlStyle,
                                      const IconStyleConfig* icon,
                                      const FontConfig* font,
                                      const StyleMetricsConfig* metrics) const {
    if (m_importExportHelper) {
        return m_importExportHelper->exportProfile(exportFilePath,
                                                   profileName,
                                                   palette,
                                                   controlStyle,
                                                   icon,
                                                   font,
                                                   metrics);
    }
    return false;
}

// --- High-Level Theme & Style Application Actions ---
bool LC_UIStyleManager::applyThemeToApplication(const PaletteConfig &paletteConfig,
                                                const ControlStyleConfig &controlStyle,
                                                const StyleMetricsConfig &metrics,
                                                const FontConfig &font,
                                                bool isDarkMode) {
    const ColorSchemeData &scheme = isDarkMode ? paletteConfig.dark : paletteConfig.light;

    // 1. Build custom QPalette using scheme colors and controlStyle's visual archetype
    const QPalette palette = LC_PaletteColorUtils::createPaletteFromScheme(
        scheme, controlStyle.styleArchetype, LC_PaletteColorUtils::CVDType::Normal
    );

    // 2. Set custom palette FIRST so widgets polished in setStyle resolve correct colors
    QApplication::setPalette(palette);

    // 3. Set dynamic proxy style and configure metrics & control decorators
    QStyle *baseStyle = QStyleFactory::create("Fusion");
    auto* proxyStyle = new LC_ProxyStyle(baseStyle, metrics);
    proxyStyle->setSkin(controlStyle);
    proxyStyle->setFont(font);
    QApplication::setStyle(proxyStyle);

    // 4. Set custom palette AGAIN (setStyle resets to style->standardPalette())
    QApplication::setPalette(palette);

    // 5. Apply typography offsets
    applyThemeTypography(font);

    // 6. Set QCSS overrides from scheme
    qApp->setStyleSheet(scheme.qss);

    // 7. Apply icons styling
    const auto linkedIconsTheme = paletteConfig.useThemeDefaultIcons ? paletteConfig.linkedIconStyleName : "";
    applyActiveOrThemeIconStyle(linkedIconsTheme, isDarkMode);

    // 8. Force update on top-level widgets
    for (QWidget *widget : QApplication::topLevelWidgets()) {
        widget->update();
    }

    return true;
}

void LC_UIStyleManager::applyActiveStyleAndTheme() {
    const bool allowStyle = isStyleAllowed();
    if (allowStyle) {
        const QString styleName = getActiveStyle();
        if (styleName.compare(QStringLiteral("Fusion"), Qt::CaseInsensitive) == 0) {
            applyActiveThemeOverride();
        } else {
            QStyle *nativeStyle = QStyleFactory::create(styleName);
            if (nativeStyle != nullptr) {
                QApplication::setStyle(nativeStyle);
                QApplication::setPalette(nativeStyle->standardPalette());
            }
            qApp->setStyleSheet("");
            applyActiveStyleSheet();
            applyGlobalTypographyAndIcons();
        }
    } else {
        resetToNativeStyle();
    }
    updateSemanticColorsCache();
}

void LC_UIStyleManager::applyActiveThemeOverride() {
    const bool allowStyle = isStyleAllowed();
    if (!allowStyle) return;

    const QString styleName = getActiveStyle();
    if (styleName != "Fusion") return;

    const bool isDarkMode = resolveIsDarkMode();

    PaletteConfig palette;
    ControlStyleConfig skin;
    StyleMetricsConfig metrics;
    FontConfig font;

    const QString activePalette = getActivePalette();
    if (activePalette.isEmpty() || !m_paletteRepository->loadByKey(activePalette, palette)) {
        LC_PaletteColorUtils::initializeDefaultPalette(palette);
    }

    const QString activeSkin = getActiveSkin();
    if (activeSkin.isEmpty() || !m_fusionSkinsRepository->loadByKey(activeSkin, skin)) {
        LC_PaletteColorUtils::initializeDefaultControlStyle(skin);
    }

    const QString activeMetrics = getActiveMetrics();
    if (activeMetrics.isEmpty() || !m_metricsRepository->loadByKey(activeMetrics, metrics)) {
        LC_StyleMetricsUtils::initializeDefault(metrics);
    }

    const QString activeTypography = getActiveTypography();
    if (activeTypography.isEmpty() || !m_typographyRepository->loadByKey(activeTypography, font)) {
        LC_TypographyUtils::initializeDefaultConfig(font);
    }

    applyThemeToApplication(palette, skin, metrics, font, isDarkMode);
}

void LC_UIStyleManager::applyActiveIconStyle() const {
    const bool isDarkMode = resolveIsDarkMode();
    QString linkedIconStyle;

    if (!getIgnoreIconStylingInTheme() && m_paletteRepository != nullptr) {
        PaletteConfig activePalette;
        if (m_paletteRepository->loadByKey(getActivePalette(), activePalette) &&
            activePalette.useThemeDefaultIcons) {
            linkedIconStyle = activePalette.linkedIconStyleName;
            }
    }

    // Delegates directly to applyActiveOrThemeIconStyle
    applyActiveOrThemeIconStyle(linkedIconStyle, isDarkMode);
}

void LC_UIStyleManager::applyTransientTheme(bool allowStyle,
                         const QString& styleName,
                                            const QString& paletteKey,
                         const QString& skinKey,
                         const QString& metricsKey,
                         const QString& typographyKey,
                         const QString& iconStyleKey,
                         ThemeModeOverride themeModeOverride) {

    if (!allowStyle) {
        resetToNativeStyle();

        FontConfig font;
        if (typographyKey.isEmpty() || typographyKey == DEFAULT_THEME_KEY || !m_typographyRepository->loadByKey(typographyKey, font)) {
            LC_TypographyUtils::initializeDefaultConfig(font);
        }
        applyThemeTypography(font);

        const bool isDarkMode = LC_PaletteColorUtils::isPaletteDarkMode();
        applyActiveOrThemeIconStyle(iconStyleKey, isDarkMode);
        return;
    }

    bool isDarkMode = false;

    if (styleName.toLower() == "fusion") {
        PaletteConfig palette;
        ControlStyleConfig skin;
        StyleMetricsConfig metrics;
        FontConfig font;

        if (themeModeOverride == ThemeModeOverride::ForceDark) {
            isDarkMode = true;
        } else if (themeModeOverride == ThemeModeOverride::ForceLight) {
            isDarkMode = false;
        } else {
            isDarkMode = LC_PaletteColorUtils::isSystemInDarkMode();
        }

        if (paletteKey.isEmpty() || paletteKey == DEFAULT_THEME_KEY || !m_paletteRepository->loadByKey(paletteKey, palette)) {
            PaletteConfig tempSkin;
            LC_PaletteColorUtils::initializeDefaultPalette(tempSkin);
            palette.light = tempSkin.light;
            palette.dark = tempSkin.dark;
            palette.name = DEFAULT_THEME_NAME;
        }

        if (skinKey.isEmpty() || skinKey == DEFAULT_THEME_KEY || !m_fusionSkinsRepository->loadByKey(skinKey, skin)) {
            skin = ControlStyleConfig();
            skin.name = DEFAULT_THEME_NAME;
        }

        if (metricsKey.isEmpty() || metricsKey == DEFAULT_THEME_KEY || !m_metricsRepository->loadByKey(metricsKey, metrics)) {
            LC_StyleMetricsUtils::initializeDefault(metrics);
        }

        if (typographyKey.isEmpty() || typographyKey == DEFAULT_THEME_KEY || !m_typographyRepository->loadByKey(typographyKey, font)) {
            LC_TypographyUtils::initializeDefaultConfig(font);
        }

        applyThemeToApplication(palette, skin, metrics, font, isDarkMode);
    } else {
        QStyle* nativeStyle = QStyleFactory::create(styleName);
        if (nativeStyle) {
            QApplication::setStyle(nativeStyle);
            QApplication::setPalette(nativeStyle->standardPalette());
        }
        qApp->setStyleSheet("");

        FontConfig font;
        if (typographyKey.isEmpty() || typographyKey == DEFAULT_THEME_KEY || !m_typographyRepository->loadByKey(typographyKey, font)) {
            LC_TypographyUtils::initializeDefaultConfig(font);
        }
        applyThemeTypography(font);

        isDarkMode = LC_PaletteColorUtils::isPaletteDarkMode();
    }

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

void LC_UIStyleManager::resetToNativeStyle() {
    QStyle* nativeStyle = nullptr;
    if (!m_nativeSystemStyleName.isEmpty()) {
        nativeStyle = QStyleFactory::create(m_nativeSystemStyleName);
    }

    if (nativeStyle != nullptr) {
        QApplication::setStyle(nativeStyle);
        QApplication::setPalette(nativeStyle->standardPalette());
    } else {
        QApplication::setPalette(m_nativeSystemPalette);
    }

    qApp->setStyleSheet("");

    applyGlobalTypographyAndIcons();

    for (QWidget* widget : QApplication::topLevelWidgets()) {
        widget->update();
    }
}


void LC_UIStyleManager::updateSemanticColorsCache() {
    const bool isDark = resolveIsDarkMode();
    const bool useFusion = isStyleAllowed() && (getActiveStyle() == "Fusion");

    PaletteConfig activePalette;
    bool hasPalette = false;
    if (useFusion && m_paletteRepository != nullptr) {
        hasPalette = m_paletteRepository->loadByKey(getActivePalette(), activePalette);
    }

    const ColorSchemeData& scheme = isDark ? activePalette.dark : activePalette.light;

    LC_PaletteColorUtils::resolveSemanticColors(hasPalette, scheme, isDark);
}
