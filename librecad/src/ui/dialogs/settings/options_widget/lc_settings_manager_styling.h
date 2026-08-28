
#ifndef LC_SETTINGS_MANAGER_STYLING_H
#define LC_SETTINGS_MANAGER_STYLING_H

#include <QString>
#include <memory>

class QWidget;
class LC_UIStyleManager;
class LC_StylingPreviewController;

class LC_SettingsManagerStyling {
public:
    static void initializeStylingSettings();
    static bool showStylingSettings(QWidget* parent,const QString& initialPageId);
    static bool isFusionGated();
    static QString fusionGatedMessage(const QString& domainName = QString());
    static QString fusionGatedActionText();
    static void enableFusionStyling(LC_UIStyleManager* styleMgr = nullptr);
private:
    static std::shared_ptr<LC_StylingPreviewController> s_previewController;
};

#endif
