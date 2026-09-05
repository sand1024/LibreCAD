
#ifndef LC_SETTINGS_MANAGER_STYLING_H
#define LC_SETTINGS_MANAGER_STYLING_H

#include <QString>
#include <memory>

#include "lc_settings_manager_base.h"

class QWidget;
class LC_UIStyleManager;
class LC_StylingPreviewController;

namespace LC_SettingsPagesStyling {
    inline const QString DLG_STYLING_PREFERENCES             = "ui_styling_preferences";

    inline const QString PAGE_STYLING_GENERAL                = "styling.general";
    inline const QString PAGE_STYLING_WIDGETS                = "styling.widgets";
    inline const QString PAGE_STYLING_PROFILES               = "styling.profiles";
    inline const QString PAGE_STYLING_TYPOGRAPHY             = "styling.typography";
    inline const QString PAGE_STYLING_ICONS                  = "styling.icons";
    inline const QString PAGE_STYLING_FUSION                 = "styling.fusion";

    inline const QString PAGE_STYLING_PALETTE                = "styling.palette";

    inline const QString PAGE_STYLING_SKINS                  = "styling.skins";
    inline const QString PAGE_STYLING_SKINS_PALETTE          = "styling.skins.palette";
    inline const QString PAGE_STYLING_SKINS_CONTAINERS       = "styling.skins.containers";
    inline const QString PAGE_STYLING_SKINS_TOOLBARS_MENUS   = "styling.skins.toolbars_menus";
    inline const QString PAGE_STYLING_SKINS_CONTROLS         = "styling.skins.controls";
    inline const QString PAGE_STYLING_SKINS_QSS              = "styling.skins.qss";

    inline const QString PAGE_STYLING_METRICS                = "styling.metrics";
    inline const QString PAGE_STYLING_METRICS_LAYOUT         = "styling.metrics.layout";
    inline const QString PAGE_STYLING_METRICS_MENUS_TOOLBARS = "styling.metrics.menus_toolbars";
    inline const QString PAGE_STYLING_METRICS_CONTROLS       = "styling.metrics.controls";
    inline const QString PAGE_STYLING_METRICS_VIEWS_TABS     = "styling.metrics.views_tabs";
    inline const QString PAGE_STYLING_METRICS_BEHAVIOR       = "styling.metrics.behavior";
}


class LC_SettingsManagerStyling : public LC_SettingsManagerBase {
public:
    static void initialize();
    static bool showStylingSettings(QWidget* parent,const QString& initialPageId);
    static bool isFusionGated();
    static QString fusionGatedMessage(const QString& domainName = QString());
    static QString fusionGatedActionText();
    static void enableFusionStyling(LC_UIStyleManager* styleMgr = nullptr);
private:
    static std::shared_ptr<LC_StylingPreviewController> s_previewController;
};

#endif
