#ifndef LC_PRESET_MANAGER_METRICS_H
#define LC_PRESET_MANAGER_METRICS_H

#include "lc_metrics_density_header_bar.h"
#include "lc_metrics_repository.h"
#include "lc_preset_manager_base.h"

class LC_PresetManagerMetrics : public LC_PresetManagerBase<StyleMetricsConfig, LC_MetricsRepository> {
    Q_OBJECT

public:
    explicit LC_PresetManagerMetrics(QObject* parent = nullptr);
    ~LC_PresetManagerMetrics() override = default;

    LC_PresetManagerUIStrings presetStrings() const override;
    bool loadPreset(const QString& key) override;
    QString getAppliedPresetKey() const override;
    void applyCurrentPreset() override;
    QWidget* getSharedHeaderWidget() override;

    bool isGated() const override;
    QString gatedMessage() const override;
    QString gatedActionText() const override;
    std::function<void()> gatedActionCallback() const override;

    void onSubPageControlChanged() override {
        if (m_headerBar != nullptr) {
            m_headerBar->setDensityToCustom();
        }
        notifyWorkingConfigChanged();
    }

 signals:
   void configLoaded(const StyleMetricsConfig& config);

public slots:
    void onDensityPresetSelected(const QString& presetName);

protected:
    void updatePreview() override;
    void resetToDefaults(StyleMetricsConfig& config) override;
    void applyActiveConfigToSystem(const QString& activeKey) override;

private:
    std::unique_ptr<LC_MetricsDensityHeaderBar> m_headerBar;
};

#endif
