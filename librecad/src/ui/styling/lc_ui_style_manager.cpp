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
#include "lc_repository_fusion_skin.h"
#include "lc_repository_icons_style.h"
#include "lc_repository_metrics.h"
#include "lc_repository_palette.h"
#include "lc_repository_typography.h"
#include "lc_repository_viewport_theme.h"
#include "lc_settings_app_styling.h"
#include "lc_settings_paths.h"
#include "lc_settings_types.h"
#include "lc_style_metrics_utils.h"
#include "lc_styling_profile_import_export_helper.h"
#include "lc_typography_utils.h"
#include "qc_applicationwindow.h"
#include "rs_settings.h"
#include "rs_system.h"

LC_UIStyleManager::LC_UIStyleManager(QC_ApplicationWindow* appWindow)
    : m_appWindow(appWindow) {
    m_iconStylesRepository = std::make_unique<LC_RepositoryIconsStyle>("");
    m_typographyRepository = std::make_unique<LC_RepositoryTypography>("");
    m_graphicViewRepository = std::make_unique<LC_RepositoryViewportTheme>("");
    m_fusionSkinsRepository = std::make_unique<LC_RepositoryFusionSkin>("");
    m_metricsRepository = std::make_unique<LC_RepositoryMetrics>("");
    m_paletteRepository = std::make_unique<LC_RepositoryPalette>("");
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

void LC_UIStyleManager::initRepositories() {
    const QString baseDir = getStyleConfigurationBaseDir();

    m_iconStylesRepository->setConfigDir(baseDir + "/icon_styles");
    m_typographyRepository->setConfigDir(baseDir + "/typography");
    m_graphicViewRepository->setConfigDir(baseDir + "/drafting");
    m_fusionSkinsRepository->setConfigDir(baseDir + "/fusion_skins");
    m_metricsRepository->setConfigDir(baseDir + "/fusion_metrics");
    m_paletteRepository->setConfigDir(baseDir + "/fusion_color_palettes");
}

void LC_UIStyleManager::reloadStyleConfiguration() {
    initRepositories();
    applyActiveStyleAndTheme();
}

// --- Centralized Application Boot Initializer ---
void LC_UIStyleManager::initialize(QC_ApplicationWindow* appWindow) {
    initRepositories();

    if (QApplication::style() != nullptr) {
        m_nativeSystemStyleName = QApplication::style()->objectName();
        m_nativeSystemPalette = QApplication::style()->standardPalette();
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

void LC_UIStyleManager::setActiveStyle(const QString& style) {
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
    return CFG_AppStyling::o_ActiveFusionPaletteName;
}

void LC_UIStyleManager::setActivePalette(const QString& name) {
    CFG_AppStyling::o_ActiveFusionPaletteName = name;
}

void LC_UIStyleManager::setActiveStyleSheet(const QString& sheet) {
    CFG_AppStyling::o_StyleSheet = sheet;
}

bool LC_UIStyleManager::getIgnoreIconStylingInPalette() const {
    return CFG_AppStyling::o_IgnoreIconStylingInPalette;
}

void LC_UIStyleManager::setIgnoreIconStylingInPalette(bool ignore) {
    CFG_AppStyling::o_IgnoreIconStylingInPalette = ignore;
}

QString LC_UIStyleManager::getActiveIconStyle() const {
    return CFG_AppStyling::o_ActiveIconsStyleName;
}

void LC_UIStyleManager::setActiveIconStyle(const QString& style) {
    CFG_AppStyling::o_ActiveIconsStyleName = style;
}

QString LC_UIStyleManager::getIconsOverridesDir() const {
    return CFG_Paths::o_IconOverridesDir;
}

void LC_UIStyleManager::setIconsOverridesDir(const QString& dir) {
    CFG_Paths::o_IconOverridesDir = dir;
}

QString LC_UIStyleManager::getActiveSkin() const {
    return CFG_AppStyling::o_ActiveFusionSkinName;
}

void LC_UIStyleManager::setActiveSkin(const QString& name) {
    CFG_AppStyling::o_ActiveFusionSkinName = name;
}

QString LC_UIStyleManager::getActiveTypography() const {
    return CFG_AppStyling::o_ActiveTypographyName;
}

void LC_UIStyleManager::setActiveTypography(const QString& name) {
    CFG_AppStyling::o_ActiveTypographyName = name;
}

QString LC_UIStyleManager::getActiveMetrics() const {
    return CFG_AppStyling::o_ActiveFusionMetricsName;
}

void LC_UIStyleManager::setActiveMetrics(const QString& name) {
    CFG_AppStyling::o_ActiveFusionMetricsName = name;
}

void LC_UIStyleManager::loadIconColorsOptions(LC_IconColorsOptions& options) const {
    options.loadSettings();
}

void LC_UIStyleManager::saveIconColorsOptions(LC_IconColorsOptions& options) const {
    options.save();
}

// --- High-Level Theme & Style Application Actions ---
bool LC_UIStyleManager::applyThemeToApplication(const PaletteConfig& paletteConfig, const ControlStyleConfig& controlStyle,
                                                const StyleMetricsConfig& metrics, const FontConfig& font, bool isDarkMode) {
    const ColorSchemeData& scheme = isDarkMode ? paletteConfig.dark : paletteConfig.light;

    // 1. Build custom QPalette using scheme colors and controlStyle's visual archetype
    const QPalette palette = LC_PaletteColorUtils::createPaletteFromScheme(scheme, controlStyle.styleArchetype,
                                                                           LC_PaletteColorUtils::CVDType::Normal);

    // 2. Set custom palette FIRST so widgets polished in setStyle resolve correct colors
    QApplication::setPalette(palette);

    // 3. Set dynamic proxy style and configure metrics & control decorators
    QStyle* baseStyle = QStyleFactory::create("Fusion");
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
    updateAllTopLevelWidgets();

    return true;
}

void LC_UIStyleManager::applyActiveStyleAndTheme() {
    const bool allowStyle = isStyleAllowed();
    if (allowStyle) {
        const QString styleName = getActiveStyle();
        if (styleName.compare(QStringLiteral("Fusion"), Qt::CaseInsensitive) == 0) {
            applyActiveThemeOverride();
        }
        else {
            applyNonFusionStyle(styleName);
            applyActiveStyleSheet();
            applyGlobalTypographyAndIcons();
        }
    }
    else {
        resetToNativeStyle();
    }
    updateSemanticColorsCache();
    if (m_appWindow != nullptr) {
        m_appWindow->onStylingApplied();
    }
}

void LC_UIStyleManager::applyActiveThemeOverride() {
    if (!isStyleAllowed()) {
        return;
    }

    const QString styleName = getActiveStyle();
    if (styleName != "Fusion") {
        return;
    }

    const bool isDarkMode = resolveIsDarkMode();

    const PaletteConfig palette = loadPaletteOrDefault(getActivePalette());
    const ControlStyleConfig skin = loadSkinOrDefault(getActiveSkin());
    const StyleMetricsConfig metrics = loadMetricsOrDefault(getActiveMetrics());
    const FontConfig font = loadFontOrDefault(getActiveTypography());

    applyThemeToApplication(palette, skin, metrics, font, isDarkMode);
}

void LC_UIStyleManager::applyActiveIconStyle() const {
    const bool isDarkMode = resolveIsDarkMode();
    QString linkedIconStyle;

    if (!getIgnoreIconStylingInPalette() && m_paletteRepository != nullptr) {
        PaletteConfig activePalette;
        if (m_paletteRepository->loadByKey(getActivePalette(), activePalette) && activePalette.useThemeDefaultIcons) {
            linkedIconStyle = activePalette.linkedIconStyleName;
        }
    }

    // Delegates directly to applyActiveOrThemeIconStyle
    applyActiveOrThemeIconStyle(linkedIconStyle, isDarkMode);
}

void LC_UIStyleManager::applyTransientTheme(bool allowStyle, const QString& styleName, const QString& paletteKey, const QString& skinKey,
                                            const QString& metricsKey, const QString& typographyKey, const QString& iconStyleKey,
                                            ThemeModeOverride themeModeOverride) {
    if (!allowStyle) {
        resetToNativeStyle();

        const FontConfig font = loadFontOrDefault(typographyKey);
        applyThemeTypography(font);

        const bool isDarkMode = LC_PaletteColorUtils::isPaletteDarkMode();
        applyActiveOrThemeIconStyle(iconStyleKey, isDarkMode);
        return;
    }

    const bool isDarkMode = resolveIsDarkMode(themeModeOverride);

    if (styleName.compare(QStringLiteral("Fusion"), Qt::CaseInsensitive) == 0) {
        const PaletteConfig palette = loadPaletteOrDefault(paletteKey);
        const ControlStyleConfig skin = loadSkinOrDefault(skinKey);
        const StyleMetricsConfig metrics = loadMetricsOrDefault(metricsKey);
        const FontConfig font = loadFontOrDefault(typographyKey);

        applyThemeToApplication(palette, skin, metrics, font, isDarkMode);
    }
    else {
        applyNonFusionStyle(styleName);

        const FontConfig font = loadFontOrDefault(typographyKey);
        applyThemeTypography(font);
    }

    applyActiveOrThemeIconStyle(iconStyleKey, isDarkMode);
}

void LC_UIStyleManager::doApplyStyle(bool isDarkMode, QString activeIconStyle) const {
    IconStyleConfig iconStyle;
    if (m_iconStylesRepository->loadByKey(activeIconStyle, iconStyle)) {
        LC_IconColorsOptions iconOptions;
        iconOptions.loadSettings();
        iconOptions.importStyleConfig(iconStyle, isDarkMode);
        LC_IconsStyleManager::applyStyle(m_appWindow, iconOptions, isDarkMode);
    }
}

QString LC_UIStyleManager::resolveEffectiveIconStyle(const QString& themeLinkedIconStyleName) const {
    if (!getIgnoreIconStylingInPalette() && !themeLinkedIconStyleName.isEmpty()) {
        return themeLinkedIconStyleName;
    }
    return getActiveIconStyle();
}

void LC_UIStyleManager::applyActiveOrThemeIconStyle(const QString& themeLinkedIconStyleName, bool isDarkMode) const {
    const QString activeIconStyle = resolveEffectiveIconStyle(themeLinkedIconStyleName);

    if (activeIconStyle == "Default") {
        LC_IconsStyleManager::applyThemeLinkedIcons(m_appWindow, "Default", true, isDarkMode);
    }
    else {
        doApplyStyle(isDarkMode, activeIconStyle);
    }
}

void LC_UIStyleManager::applyActiveOrDefaultIconStyle(bool isDarkMode) const {
    applyActiveOrThemeIconStyle(QString(), isDarkMode);
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

bool LC_UIStyleManager::resolveIsDarkMode(ThemeModeOverride themeMode) const {
    if (themeMode == ThemeModeOverride::ForceDark) {
        return true;
    }
    if (themeMode == ThemeModeOverride::ForceLight) {
        return false;
    }
    return LC_PaletteColorUtils::isSystemInDarkMode();
}

bool LC_UIStyleManager::resolveIsDarkMode() const {
    return resolveIsDarkMode(getThemeModeOverride());
}

void LC_UIStyleManager::applyGlobalTypographyAndIcons() {
    const FontConfig font = loadFontOrDefault(getActiveTypography());
    applyThemeTypography(font);

    const bool isDarkMode = resolveIsDarkMode();
    applyActiveOrThemeIconStyle(QString(), isDarkMode);
}

void LC_UIStyleManager::applyThemeTypography(const FontConfig& activeFont) {
    static const char* const RESET_WIDGET_CLASSES[] = {
        "QHeaderView", "QTabBar", "QMenuBar", "QMenu", "QPushButton",
        "QToolButton", "QToolBar", "QLineEdit", "QComboBox",
        "QAbstractSpinBox", "QTextEdit", "QPlainTextEdit", "LC_PropertySheetWidget"
    };

    if (activeFont.name == CFG_AppState::DEFAULT_THEME_KEY) {
        const QFont resetFont;
        for (const char* className : RESET_WIDGET_CLASSES) {
            QApplication::setFont(resetFont, className);
        }
        return;
    }

    auto applyFontRole = [](const QString& family, int baseSize, const FontRoleConfig& role,
                            std::initializer_list<const char*> classNames) {
        QFont f(family, baseSize + role.sizeOffset);
        f.setBold(role.bold);
        f.setItalic(role.italic);
        for (const char* className : classNames) {
            QApplication::setFont(f, className);
        }
    };

    // Role 1: Main Interface Base
    const QFont mainFont(activeFont.mainFamily, activeFont.mainSize);
    QApplication::setFont(mainFont);

    // Role 2: Section Headings
    applyFontRole(activeFont.mainFamily, activeFont.mainSize, activeFont.headings, { "QHeaderView", "QTabBar" });

    // Role 3: Main Menu Bar
    applyFontRole(activeFont.mainFamily, activeFont.mainSize, activeFont.menuBar, { "QMenuBar" });

    // Role 4: Popup & Dropdown Menus
    applyFontRole(activeFont.mainFamily, activeFont.mainSize, activeFont.menus, { "QMenu" });

    // Role 5: Buttons & Toolbars
    applyFontRole(activeFont.mainFamily, activeFont.mainSize, activeFont.buttons, { "QPushButton", "QToolButton", "QToolBar" });

    // Role 6: Inputs & Editors
    applyFontRole(activeFont.mainFamily, activeFont.mainSize, activeFont.inputs, { "QLineEdit", "QComboBox", "QAbstractSpinBox" });
    applyFontRole(activeFont.mainFamily, activeFont.mainSize, activeFont.propertiesWidget, { "LC_PropertySheetWidget" });

    // Role 7: Code & Technical
    applyFontRole(activeFont.techFamily, activeFont.mainSize, activeFont.technical, { "QTextEdit", "QPlainTextEdit" });
}

void LC_UIStyleManager::resetToNativeStyle() {
    QStyle* nativeStyle = nullptr;
    if (!m_nativeSystemStyleName.isEmpty()) {
        nativeStyle = QStyleFactory::create(m_nativeSystemStyleName);
    }

    if (nativeStyle != nullptr) {
        QApplication::setStyle(nativeStyle);
        QApplication::setPalette(QPalette());
    }
    else {
        QApplication::setPalette(m_nativeSystemPalette);
    }

    qApp->setStyleSheet("");

    applyGlobalTypographyAndIcons();
    updateAllTopLevelWidgets();
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

PaletteConfig LC_UIStyleManager::loadPaletteOrDefault(const QString& key) const {
    PaletteConfig palette;
    if (key.isEmpty() || key == CFG_AppState::DEFAULT_THEME_KEY || m_paletteRepository == nullptr || !m_paletteRepository->
        loadByKey(key, palette)) {
        LC_PaletteColorUtils::initializeDefaultPalette(palette);
    }
    return palette;
}

ControlStyleConfig LC_UIStyleManager::loadSkinOrDefault(const QString& key) const {
    ControlStyleConfig skin;
    if (key.isEmpty() || key == CFG_AppState::DEFAULT_THEME_KEY || m_fusionSkinsRepository == nullptr || !m_fusionSkinsRepository->
        loadByKey(key, skin)) {
        LC_PaletteColorUtils::initializeDefaultControlStyle(skin);
    }
    return skin;
}

StyleMetricsConfig LC_UIStyleManager::loadMetricsOrDefault(const QString& key) const {
    StyleMetricsConfig metrics;
    if (key.isEmpty() || key == CFG_AppState::DEFAULT_THEME_KEY || m_metricsRepository == nullptr || !m_metricsRepository->
        loadByKey(key, metrics)) {
        LC_StyleMetricsUtils::initializeDefault(metrics);
    }
    return metrics;
}

FontConfig LC_UIStyleManager::loadFontOrDefault(const QString& key) const {
    FontConfig font;
    if (key.isEmpty() || key == CFG_AppState::DEFAULT_THEME_KEY || m_typographyRepository == nullptr || !m_typographyRepository->
        loadByKey(key, font)) {
        LC_TypographyUtils::initializeDefaultConfig(font);
    }
    return font;
}

void LC_UIStyleManager::applyNonFusionStyle(const QString& styleName) {
    QStyle* nativeStyle = QStyleFactory::create(styleName);
    if (nativeStyle != nullptr) {
        QApplication::setStyle(nativeStyle);
        QApplication::setPalette(QPalette());
    }
    qApp->setStyleSheet("");
}

void LC_UIStyleManager::updateAllTopLevelWidgets() const {
    for (QWidget* widget : QApplication::topLevelWidgets()) {
        if (widget != nullptr) {
            widget->update();
        }
    }
}
