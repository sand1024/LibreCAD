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

#ifndef LC_UI_STYLE_MANAGER_H
#define LC_UI_STYLE_MANAGER_H

#include <QObject>
#include <memory>

#include "lc_dlg_checklist_selection.h"
#include "lc_icons_style_shared.h"
#include "lc_palette_editor_shared.h"

class LC_RepositoryViewportTheme;
// Forward declarations
class QC_ApplicationWindow;
class LC_RepositoryPalette;
class LC_RepositoryFusionSkin;
class LC_RepositoryIconsStyle;
class LC_RepositoryTypography;
class LC_RepositoryMetrics;
class LC_StylingProfileImportExportHelper;
class LC_CustomStyleHelper;
class LC_IconColorsOptions;

class LC_UIStyleManager : public QObject {
    Q_OBJECT public:
    LC_UIStyleManager(QC_ApplicationWindow* appWindow);
    ~LC_UIStyleManager() override;

    // Prevent copy assignments to maintain strict unique repository ownership
    LC_UIStyleManager(const LC_UIStyleManager&) = delete;
    LC_UIStyleManager& operator=(const LC_UIStyleManager&) = delete;

    void initialize(QC_ApplicationWindow* appWindow);
    QString getStyleConfigurationBaseDir() const;
    void reloadStyleConfiguration();

    bool isStyleAllowed() const;
    void setStyleAllowed(bool allowed);

    QString getActiveStyle() const;
    void setActiveStyle(const QString& style);

    ThemeModeOverride getThemeModeOverride() const;
    void setThemeModeOverride(ThemeModeOverride mode);

    QString getActiveStyleSheet() const;
    void setActiveStyleSheet(const QString& sheet);

    bool getIgnoreIconStylingInPalette() const;
    void setIgnoreIconStylingInPalette(bool ignore);

    QString getActiveIconStyle() const;
    void setActiveIconStyle(const QString& style);

    QString getIconsOverridesDir() const;
    void setIconsOverridesDir(const QString& dir);

    QString getActivePalette() const;
    void setActivePalette(const QString& name);

    QString getActiveSkin() const;
    void setActiveSkin(const QString& name);

    QString getActiveTypography() const;
    void setActiveTypography(const QString& name);

    QString getActiveMetrics() const;
    void setActiveMetrics(const QString& name);

    // --- Legacy Compatibility Aliases ---
    QString getAppliedThemeName() const {
        return getActiveSkin();
    }

    void saveAppliedThemeName(const QString& themeName) {
        setActiveSkin(themeName);
    }

    // --- Icon Colors Options Facade ---
    void loadIconColorsOptions(LC_IconColorsOptions& options) const;
    void saveIconColorsOptions(LC_IconColorsOptions& options) const;

    // --- Repository & Helper Accessors ---
    LC_RepositoryPalette* getPaletteRepository() const {
        return m_paletteRepository.get();
    }

    LC_RepositoryFusionSkin* getSkinsRepository() const {
        return m_fusionSkinsRepository.get();
    }

    LC_RepositoryIconsStyle* getIconsStyleRepository() const {
        return m_iconStylesRepository.get();
    }

    LC_RepositoryTypography* getTypographyRepository() const {
        return m_typographyRepository.get();
    }

    LC_RepositoryMetrics* getMetricsRepository() const {
        return m_metricsRepository.get();
    }

    LC_RepositoryViewportTheme* getViewportStylingRepository() const {
        return m_graphicViewRepository.get();
    }

    LC_StylingProfileImportExportHelper* getImportExportHelper() const {
        return m_importExportHelper.get();
    }

    bool isCurrentActiveStyleFusion() const {
        const bool stylingAllowed = isStyleAllowed();
        const auto currentStyleName = getActiveStyle();
        const bool currentThemeIsFusion = stylingAllowed && (currentStyleName == "Fusion");
        return currentThemeIsFusion;
    }

    // --- High-Level Theme & Style Application Actions ---
    bool applyThemeToApplication(const PaletteConfig& paletteConfig, const ControlStyleConfig& controlStyle,
                                 const StyleMetricsConfig& metrics, const FontConfig& font, bool isDarkMode);

    void applyActiveStyleAndTheme();
    void applyActiveThemeOverride();
    void applyActiveIconStyle() const;
    void applyTransientTheme(bool allowStyle, const QString& styleName, const QString& paletteKey, const QString& skinKey,
                             const QString& metricsKey, const QString& typographyKey, const QString& iconStyleKey,
                             ThemeModeOverride themeModeOverride);
    void doApplyStyle(bool isDarkMode, QString activeIconStyle) const;
    void applyActiveOrThemeIconStyle(const QString& themeLinkedIconStyleName, bool isDarkMode) const;
    void applyActiveOrDefaultIconStyle(bool isDarkMode) const;
    void applyActiveStyleSheet() const;

    void reloadStyleSheet() const;
    bool resolveIsDarkMode() const;
    void loadStyleSheet(const QString& stylesheetFilePath) const;
    bool resolveIsDarkMode(ThemeModeOverride themeMode) const;
    void resetToNativeStyle();
    void updateSemanticColorsCache();

private:
    PaletteConfig loadPaletteOrDefault(const QString& key) const;
    ControlStyleConfig loadSkinOrDefault(const QString& key) const;
    StyleMetricsConfig loadMetricsOrDefault(const QString& key) const;
    FontConfig loadFontOrDefault(const QString& key) const;

    QString resolveEffectiveIconStyle(const QString& themeLinkedIconStyleName) const;
    void applyNonFusionStyle(const QString& styleName);
    void updateAllTopLevelWidgets() const;
    void initRepositories();
    void applyGlobalTypographyAndIcons();
    void applyThemeTypography(const FontConfig& activeFont);

    QString m_nativeSystemStyleName;
    QPalette m_nativeSystemPalette;

    std::unique_ptr<LC_CustomStyleHelper> m_customStyleHelper;
    std::unique_ptr<LC_RepositoryPalette> m_paletteRepository;
    std::unique_ptr<LC_RepositoryFusionSkin> m_fusionSkinsRepository;
    std::unique_ptr<LC_RepositoryIconsStyle> m_iconStylesRepository;
    std::unique_ptr<LC_RepositoryTypography> m_typographyRepository;
    std::unique_ptr<LC_RepositoryMetrics> m_metricsRepository;
    std::unique_ptr<LC_RepositoryViewportTheme> m_graphicViewRepository;
    std::unique_ptr<LC_StylingProfileImportExportHelper> m_importExportHelper;
    QC_ApplicationWindow* m_appWindow {nullptr};
};

#endif
