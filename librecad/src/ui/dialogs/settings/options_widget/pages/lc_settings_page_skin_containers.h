#ifndef LC_SETTINGS_PAGE_SKIN_CONTAINERS_H
#define LC_SETTINGS_PAGE_SKIN_CONTAINERS_H

#include "lc_settings_page_base.h"

namespace Ui {
    class LC_SettingsPageSkinContainers;
}

class LC_PresetManagerFusionSkin;

class LC_SettingsPageSkinContainers : public LC_SettingsPageBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageSkinContainers(QObject* parent = nullptr);
    ~LC_SettingsPageSkinContainers() override;

    void bindToPresetManager(LC_PresetManagerInterface* manager) override;
    void loadSettings() override;
    bool saveSettings() override;
    bool isModified() const override;
    bool disablesWidgetOnGating() const override { return false; }
protected:
    void setupUi() override;
    void setupBehavior() override;
    void updateCloseButtonUiState() const;

private slots:
    void onControlChanged();
    void updateGroupBoxUiState();
    void updateSplitterUiState() const;

private:
    void setupComboboxes();
    void populateUiFromWorkingConfig();
    void syncUiToWorkingConfig();
    void updateArchetypeGating();

    LC_PresetManagerFusionSkin* m_presetManager = nullptr;
    std::unique_ptr<Ui::LC_SettingsPageSkinContainers> ui;
    bool m_blockSignals = false;
};

#endif
