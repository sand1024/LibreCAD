#ifndef LC_PRESET_MANAGER_TYPOGRAPHY_H
#define LC_PRESET_MANAGER_TYPOGRAPHY_H

#include "lc_preset_manager_base.h"
#include "lc_typography_repository.h"

class LC_PresetManagerTypography : public LC_PresetManagerBase<FontConfig, LC_TypographyRepository> {
    Q_OBJECT

public:
    explicit LC_PresetManagerTypography(QObject* parent = nullptr);
    ~LC_PresetManagerTypography() override = default;

    LC_PresetManagerUIStrings presetStrings() const override;
    bool loadPreset(const QString& key) override;
    QString getAppliedPresetKey() const override;
    void applyCurrentPreset() override;

    signals:
        void configLoaded(const FontConfig& config);

protected:
    void updatePreview() override;
    void resetToDefaults(FontConfig& config) override;
    void applyActiveConfigToSystem(const QString& activeKey) override;
};

#endif // LC_PRESET_MANAGER_TYPOGRAPHY_H
