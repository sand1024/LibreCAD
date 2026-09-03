#ifndef LC_PRESET_MANAGER_BASE_H
#define LC_PRESET_MANAGER_BASE_H

#include <QObject>
#include "lc_preset_manager_interface.h"
#include "lc_styling_preview_controller.h"
#include "lc_style_repository_base.h"
#include "lc_ui_style_manager.h"
#include "qc_applicationwindow.h"

template <typename TConfig, typename TRepo>
class LC_PresetManagerBase : public QObject, public LC_PresetManagerInterface, public LC_StylingPreviewAware {
public:
    explicit LC_PresetManagerBase(LC_UIStyleManager* styleManager, TRepo* repo, const QString& initialKey, QObject* parent = nullptr)
        : QObject(parent)
        , m_repository(repo)
        , m_originalActiveKey(initialKey)
        , m_activeKey(initialKey)
        , m_styleManager (styleManager){
        if (m_repository != nullptr) {
            m_repository->initializeIndex();
        }

        if (m_originalActiveKey.isEmpty() || m_originalActiveKey == DEFAULT_THEME_NAME ||
            (m_repository != nullptr && !m_repository->exists(m_originalActiveKey))) {
            m_originalActiveKey = DEFAULT_THEME_KEY;
        }
        m_activeKey = m_originalActiveKey;
    }

    bool supportsPreviewWindow() const override { return true; }
    bool supportsAccessibilityCheck() const override { return false; }

    ~LC_PresetManagerBase() override = default;

    // 1. Common Preset Interface Implementations
    QList<QPair<QString, QString>> getAvailablePresets() const override {
        QList<QPair<QString, QString>> choices;
        choices.append(qMakePair(defaultPresetDisplayName(), DEFAULT_THEME_KEY));
        if (m_repository != nullptr) {
            choices.append(m_repository->getPresetChoices());
        }
        return choices;
    }

    QString getActivePresetKey() const override {
        return m_activeKey;
    }

    bool isPresetModified() override {
        return m_isDirty;
    }

    bool saveCurrentPreset() override {
        if (m_activeKey == DEFAULT_THEME_KEY || m_repository == nullptr) {
            return false;
        }
        QString outKey;
        if (m_repository->save(m_workingConfig.name, m_workingConfig, outKey)) {
            m_activeKey = outKey;
            m_isDirty = false;
            if (m_changedCallback != nullptr) {
                m_changedCallback(false);
            }
            return true;
        }
        return false;
    }

    bool savePresetAs(const QString& name, QString& outKey) override {
        if (m_repository == nullptr) {
            return false;
        }
        m_workingConfig.name = name;
        if (m_repository->save(name, m_workingConfig, outKey)) {
            m_activeKey = outKey;
            m_isDirty = false;
            if (m_changedCallback != nullptr) {
                m_changedCallback(false);
            }
            return true;
        }
        return false;
    }

    bool deletePreset(const QString& key) override {
        if (key == DEFAULT_THEME_KEY || m_repository == nullptr) {
            return false;
        }
        if (key == m_originalActiveKey) {
            return false;
        }
        return m_repository->removeByKey(key);
    }

    void rollbackState() override {
        loadPreset(m_originalActiveKey);
    }

    void setChangedCallback(std::function<void(bool)> callback) override {
        m_changedCallback = std::move(callback);
    }

    bool onDialogAccept(QWidget* parentDialog) override {
        if (!handlePromptSaveOnAccept(parentDialog)) {
            return false;
        }
        applyActiveConfigToSystem(m_activeKey);
        return true;
    }

    bool onDialogReject(QWidget* parentDialog) override {
        return handlePromptDiscardOnReject(parentDialog);
    }

    // 2. Common Preview Controller Wiring
    void setPreviewController(LC_StylingPreviewController* controller) override {
        m_previewController = controller;
        updatePreview();
    }

    QWidget* getSharedBottomWidget() override {
        return (m_previewController != nullptr)
                   ? m_previewController->createBottomWidget(supportsPreviewWindow(), supportsAccessibilityCheck())
                   : nullptr;
    }

    // 3. Common Accessors & Dirty State Management
    const TConfig& workingConfig() const { return m_workingConfig; }
    TConfig& workingConfig() { return m_workingConfig; }

    void notifyWorkingConfigChanged() {
        m_isDirty = true;
        if (m_changedCallback != nullptr) {
            m_changedCallback(true);
        }
        updatePreview();
    }

    bool isGated() const override {
        return isReadOnlyDefault();
    }

    QString gatedMessage() const override {
        if (isReadOnlyDefault()) {
            return presetStrings().defaultReadOnlyMessage;
        }
        return QString();
    }

    QString gatedActionText() const override {
        if (isReadOnlyDefault()) {
            return presetStrings().duplicateActionText;
        }
        return QString();
    }

    std::function<void()> gatedActionCallback() const override {
        return nullptr;
    }

    bool supportsApply() const override { return true; }
    bool supportsImportExport() const override { return false; }

protected:
    virtual void updatePreview() {}
    virtual void resetToDefaults(TConfig& config) = 0;
    virtual void applyActiveConfigToSystem(const QString& activeKey) = 0;

    TConfig m_workingConfig;
    TRepo* m_repository = nullptr;
    LC_UIStyleManager* m_styleManager = nullptr;
    LC_StylingPreviewController* m_previewController = nullptr;
    QString m_originalActiveKey;
    QString m_activeKey;
    bool m_isDirty = false;
    std::function<void(bool)> m_changedCallback;
};

#endif
