#ifndef LC_SETTINGS_PAGE_SKIN_PALETTE_H
#define LC_SETTINGS_PAGE_SKIN_PALETTE_H

#include "lc_settings_page_base.h"
#include "lc_palette_editor_shared.h"

namespace Ui {
    class LC_SettingsPageSkinPalette;
}

class LC_PresetManagerPalette;

class LC_SettingsPageSkinPalette : public LC_SettingsPageBase {
    Q_OBJECT
public:
    explicit LC_SettingsPageSkinPalette(QObject* parent = nullptr);
    ~LC_SettingsPageSkinPalette() override;

    void bindToPresetManager(LC_PresetManagerInterface* manager) override;
    void loadSettings() override;
    bool saveSettings() override;
    bool isModified() const override;
protected:
    void setupUi() override;
    void setupBehavior() override;
    void onPaletteTabChanged(int index);
    void setReadOnly(bool readOnly) override;

private slots:
    void onControlChanged();
    void onVariantToggled(bool checked);
    void onAutoCalc3DToggled(bool checked);
    void onBevelSeedChanged(int index);
    void onContrastWeightChanged(int index);
    void onContrastPolicyChanged(int index);
    void onUseThemeIconsToggled(bool checked);
    void onGenerateHarmonizedTheme();
    void onGenerateTwoColorTheme();
    void onGenerateHighContrastTheme();

private:
    void setupComboboxes() const;
    void setupGeneratorMenu();
    void setupTablesStructure();
    void populateTablesFromConfig();
    void syncUiToWorkingConfig();
    void syncUiToWorkingConfig(bool isDark);
    QString getRoleTooltip(QPalette::ColorRole role) const;
    QString getRoleDisplayName(QPalette::ColorRole role) const;

    LC_PresetManagerPalette* m_presetManager = nullptr;
    std::unique_ptr<Ui::LC_SettingsPageSkinPalette> ui;
    bool m_blockSignals = false;
};

#endif
