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

#ifndef LC_STYLING_PREVIEW_CONTROLLER_H
#define LC_STYLING_PREVIEW_CONTROLLER_H

#include <QObject>
#include <QPointer>
#include "lc_palette_color_utils.h"
#include "lc_palette_editor_shared.h"

class LC_SettingsPageInterface;
class LC_UIStyleManager;
class LC_PresetManagerInterface;
class LC_SkinPreviewWindow;
class LC_StylingPreviewBottomBar;
class LC_StylingPreviewController;
class QWidget;

class LC_StylingPreviewAware {
public:
    virtual ~LC_StylingPreviewAware() = default;
    virtual void setPreviewController(LC_StylingPreviewController* controller) = 0;
    virtual bool supportsPreviewWindow() const { return true; }
    virtual bool supportsAccessibilityCheck() const { return false; }
};

struct LC_PreviewCompositeState {
    PaletteConfig palette;
    ControlStyleConfig skin;
    StyleMetricsConfig metrics;
    FontConfig font;
    bool isDarkMode = false;
    LC_PaletteColorUtils::CVDType cvd = LC_PaletteColorUtils::CVDType::Normal;
    bool isSimulatedDisabled = false;
};

class LC_StylingPreviewController : public QObject {
    Q_OBJECT
public:
    explicit LC_StylingPreviewController(QWidget* dialogParent = nullptr, QObject* parent = nullptr);
    ~LC_StylingPreviewController() override;

    void setDialogParent(QWidget* dialogParent);
    void initFromStyleManager(LC_UIStyleManager* styleManager);

    QWidget* createBottomWidget(bool includePreview, bool includeCvd, QWidget* parent = nullptr);
    void setPreviewVisible(bool visible);
    bool isPreviewVisible() const;
    void closePreview();
    void hidePreviewTemporarily() const;

    void onCategoryChanged(LC_SettingsPageInterface* page);

    void updatePreviewPalette(const PaletteConfig& palette, bool isDarkMode);
    void updatePreviewSkin(const ControlStyleConfig& skin);
    void updatePreviewMetrics(const StyleMetricsConfig& metrics);
    void updatePreviewTypography(const FontConfig& font);
    void updatePreviewToolbarsAndDocks();

    LC_PaletteColorUtils::CVDType activeCvdType() const;
    bool isDisabledSimulated() const;
    void activatePreviewTab(const QString& tag);

signals:
    void cvdChanged(LC_PaletteColorUtils::CVDType type);
    void disabledStateChanged(bool disabled);

private:
    void ensurePreviewWindow();
    void applyCompositePreview();
    void applyTypographyToPreview(const FontConfig& font) const;

    QPointer<QWidget> m_dialogParent;
    QPointer<LC_SkinPreviewWindow> m_previewWindow;
    QPointer<LC_StylingPreviewBottomBar> m_bottomBar;
    LC_PreviewCompositeState m_state;
};
;

#endif
