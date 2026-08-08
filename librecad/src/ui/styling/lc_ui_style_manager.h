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
#include "lc_palette_editor_shared.h"
#include "lc_icons_style_shared.h"

// Forward declarations
class QC_ApplicationWindow;
class LC_FusionSkinsRepository;
class LC_IconsStyleRepository;
class LC_TypographyRepository;
class LC_MetricsRepository;
class LC_WorkspaceImportExportHelper;
class LC_CustomStyleHelper;
class LC_IconColorsOptions;
class LC_IconColorsOptions;

class LC_UIStyleManager : public QObject {
    Q_OBJECT
public:
    LC_UIStyleManager();
    ~LC_UIStyleManager() override;

    // Prevent copy assignments to maintain strict unique repository ownership
    LC_UIStyleManager(const LC_UIStyleManager&) = delete;
    LC_UIStyleManager& operator=(const LC_UIStyleManager&) = delete;

    // --- Application Startup Initializer ---
    void initialize(QC_ApplicationWindow* appWindow);

    // --- Centralized Base Directory & Re-initialization ---
    QString getStyleConfigurationBaseDir() const;
    void reloadStyleConfiguration();

    // --- Dynamic Settings Accessors (Facade for RS_Settings) ---
    bool isStyleAllowed() const;
    void setStyleAllowed(bool allowed);

    QString getActiveStyle() const;
    void setActiveStyle(const QString &style);

    ThemeModeOverride getThemeModeOverride() const;
    void setThemeModeOverride(ThemeModeOverride mode);

    QString getActiveStyleSheet() const;
    void setActiveStyleSheet(const QString &sheet);

    bool getIgnoreIconStylingInTheme() const;
    void setIgnoreIconStylingInTheme(bool ignore);

    QString getActiveIconStyle() const;
    void setActiveIconStyle(const QString &style);

    QString getIconsOverridesDir() const;
    void setIconsOverridesDir(const QString &dir);

    // --- Active Decoupled Preset Selection Accessors ---
    QString getActiveSkin() const;
    void setActiveSkin(const QString& name);

    QString getActiveTypography() const;
    void setActiveTypography(const QString& name);

    QString getActiveMetrics() const;
    void setActiveMetrics(const QString& name);

    // --- Legacy Compatibility Aliases (Helps migrating older UI dialogs) ---
    QString getAppliedThemeName() const { return getActiveSkin(); }
    void saveAppliedThemeName(const QString &themeName) { setActiveSkin(themeName); }

    // --- Icon Colors Options Facade ---
    void loadIconColorsOptions(LC_IconColorsOptions &options) const;
    void saveIconColorsOptions(LC_IconColorsOptions &options) const;

    // --- Repository & Helper Accessors ---
    LC_FusionSkinsRepository* getSkinsRepository() const { return m_fusionSkinsRepository.get(); }
    LC_IconsStyleRepository* getIconsStyleRepository() const { return m_iconStylesRepository.get(); }
    LC_TypographyRepository* getTypographyRepository() const { return m_typographyRepository.get(); }
    LC_MetricsRepository* getMetricsRepository() const { return m_metricsRepository.get(); }
    LC_WorkspaceImportExportHelper* getImportExportHelper() const { return m_importExportHelper.get(); }

    // --- High-Level Theme & Style Application Actions ---
    bool applyThemeToApplication(const SkinConfig &skin, 
                                 const StyleMetricsConfig &metrics, 
                                 const FontConfig &font, 
                                 bool isDarkMode);
    void applyActiveStyleAndTheme();
    void applyActiveThemeOverride();
    void applyActiveIconStyle() const;
    void applyTransientTheme(bool allowStyle,
                         const QString& styleName,
                         const QString& skinKey,
                         const QString& metricsKey,
                         const QString& typographyKey,
                         const QString& iconStyleKey,
                         ThemeModeOverride themeModeOverride);
    void applyActiveOrThemeIconStyle(const QString& themeLinkedIconStyleName, bool isDarkMode) const;
    void applyActiveOrDefaultIconStyle(bool isDarkMode) const;
    void applyActiveStyleSheet() const;

    void reloadStyleSheet() const;
    bool resolveIsDarkMode() const;
    void loadStyleSheet(const QString& stylesheetFilePath) const;

    bool importProfile(const QString& importFilePath,
                      QString& outProfileName,
                      QString& outSkinName,
                      QString& outIconStyleName,
                      QString& outTypographyName,
                      QString& outMetricsName) const;

    bool exportProfile(const QString& exportFilePath,
                       const QString& profileName,
                       const SkinConfig* skin,
                       const IconStyleConfig* icon,
                       const FontConfig* font,
                       const StyleMetricsConfig* metrics) const;

private:
    void initializeRepositories();
    void applyGlobalTypographyAndIcons();
    void applyThemeTypography(const FontConfig &activeFont);

    std::unique_ptr<LC_CustomStyleHelper>           m_customStyleHelper;
    std::unique_ptr<LC_FusionSkinsRepository>       m_fusionSkinsRepository;
    std::unique_ptr<LC_IconsStyleRepository>        m_iconStylesRepository;
    std::unique_ptr<LC_TypographyRepository>        m_typographyRepository;
    std::unique_ptr<LC_MetricsRepository>           m_metricsRepository;
    std::unique_ptr<LC_WorkspaceImportExportHelper> m_importExportHelper;
};

#endif
