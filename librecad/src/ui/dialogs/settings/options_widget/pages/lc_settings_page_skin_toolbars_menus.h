#ifndef LC_SETTINGS_PAGE_SKIN_TOOLBARS_MENUS_H
#define LC_SETTINGS_PAGE_SKIN_TOOLBARS_MENUS_H

#include "lc_settings_page_base.h"

namespace Ui {
    class LC_SettingsPageSkinToolbarsMenus;
}

class LC_PresetManagerFusionSkin;

class LC_SettingsPageSkinToolbarsMenus : public LC_SettingsPageBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageSkinToolbarsMenus(QObject* parent = nullptr);
    ~LC_SettingsPageSkinToolbarsMenus() override;

    void bindToPresetManager(LC_PresetManagerInterface* manager) override;
    void loadSettings() override;
    bool saveSettings() override;
    bool isModified() const override;
    bool disablesWidgetOnGating() const override { return false; }

protected:
    void setupUi() override;
    void setupBehavior() override;

private slots:
    void onControlChanged();

private:
    void setupComboboxes();
    void updateToolButtonUiState();
    void updateSegmentedButtonsUiState();
    void populateUiFromWorkingConfig();
    void syncUiToWorkingConfig() const;
    void updateArchetypeGating();

    LC_PresetManagerFusionSkin* m_presetManager = nullptr;
    std::unique_ptr<Ui::LC_SettingsPageSkinToolbarsMenus> ui;
    bool m_blockSignals = false;
};

#endif
