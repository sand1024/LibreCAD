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

#include "lc_styling_preview_controller.h"
#include <QApplication>
#include <QComboBox>
#include <QHeaderView>
#include <QLayout>
#include <QPlainTextEdit>
#include <QStyleFactory>
#include <QTabWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QGroupBox>
#include <QStatusBar>

#include "lc_caddockwidget.h"
#include "lc_preset_manager_icons_style.h"
#include "lc_proxy_style.h"
#include "lc_repository_fusion_skin.h"
#include "lc_repository_metrics.h"
#include "lc_repository_palette.h"
#include "lc_repository_typography.h"
#include "lc_setting.h"
#include "lc_settings_colors_semantics.h"
#include "lc_settings_page_interface.h"
#include "lc_skin_preview_window.h"
#include "lc_style_metrics_utils.h"
#include "lc_styling_preview_bottom_bar.h"
#include "lc_typography_utils.h"
#include "lc_ui_style_manager.h"

namespace {
    const LC_SettingsGroupBase CFG_StylingPreviewGroup("StylingPreview");
    const LC_Setting<bool> o_ShowLivePreviewWindow(&CFG_StylingPreviewGroup, "ShowLivePreviewWindow", true);
}

LC_StylingPreviewController::LC_StylingPreviewController(QWidget* dialogParent, QObject* parent)
    : QObject(parent)
    , m_dialogParent(dialogParent) {
    LC_PaletteColorUtils::initializeDefaultPalette(m_state.palette);
    LC_PaletteColorUtils::initializeDefaultControlStyle(m_state.skin);
    LC_StyleMetricsUtils::initializeDefault(m_state.metrics);
    LC_TypographyUtils::initializeDefaultConfig(m_state.font);
    m_state.isDarkMode = LC_PaletteColorUtils::isSystemInDarkMode();
}

LC_StylingPreviewController::~LC_StylingPreviewController() {
    closePreview();
}

void LC_StylingPreviewController::setDialogParent(QWidget* dialogParent) {
    m_dialogParent = dialogParent;
}

void LC_StylingPreviewController::initFromStyleManager(LC_UIStyleManager* styleManager) {
    if (styleManager == nullptr) {
        return;
    }

    const QString activePaletteKey = styleManager->getActivePalette();
    if (styleManager->getPaletteRepository() == nullptr ||
        !styleManager->getPaletteRepository()->loadByKey(activePaletteKey, m_state.palette)) {
        LC_PaletteColorUtils::initializeDefaultPalette(m_state.palette);
    }

    const QString activeSkinKey = styleManager->getActiveSkin();
    if (styleManager->getSkinsRepository() == nullptr ||
        !styleManager->getSkinsRepository()->loadByKey(activeSkinKey, m_state.skin)) {
        LC_PaletteColorUtils::initializeDefaultControlStyle(m_state.skin);
    }

    const QString activeMetricsKey = styleManager->getActiveMetrics();
    if (styleManager->getMetricsRepository() == nullptr ||
        !styleManager->getMetricsRepository()->loadByKey(activeMetricsKey, m_state.metrics)) {
        LC_StyleMetricsUtils::initializeDefault(m_state.metrics);
    }

    const QString activeTypoKey = styleManager->getActiveTypography();
    if (styleManager->getTypographyRepository() == nullptr ||
        !styleManager->getTypographyRepository()->loadByKey(activeTypoKey, m_state.font)) {
        LC_TypographyUtils::initializeDefaultConfig(m_state.font);
    }

    m_state.isDarkMode = styleManager->resolveIsDarkMode();
    applyCompositePreview();
}

void LC_StylingPreviewController::ensurePreviewWindow() {
    if (m_previewWindow == nullptr) {
        m_previewWindow = new LC_SkinPreviewWindow(m_dialogParent.data());
        m_previewWindow->setWindowTitle(tr("Live Theme Preview — Sandbox"));
        m_previewWindow->setWindowFlags(Qt::Window);

        if (m_dialogParent != nullptr) {
            m_previewWindow->move(m_dialogParent->geometry().right() + 10, m_dialogParent->geometry().top());
        }

        connect(m_previewWindow, &LC_SkinPreviewWindow::windowClosed, this, [this]() {
            o_ShowLivePreviewWindow = false;
            if (m_bottomBar != nullptr) {
                m_bottomBar->setPreviewChecked(false);
            }
        });

        applyCompositePreview();
    }
}

QWidget* LC_StylingPreviewController::createBottomWidget(bool includePreview, bool includeCvd, QWidget* parent) {
    if (m_bottomBar == nullptr) {
    m_bottomBar = new LC_StylingPreviewBottomBar(parent);

    connect(m_bottomBar, &LC_StylingPreviewBottomBar::previewToggled, this, [this](bool checked) {
        o_ShowLivePreviewWindow = checked;
        setPreviewVisible(checked);
    });

    connect(m_bottomBar, &LC_StylingPreviewBottomBar::disabledStateToggled, this, [this](bool disabled) {
        m_state.isSimulatedDisabled = disabled;
        applyCompositePreview();
        emit disabledStateChanged(disabled);
    });

    connect(m_bottomBar, &LC_StylingPreviewBottomBar::cvdChanged, this, [this](LC_PaletteColorUtils::CVDType cvd) {
        m_state.cvd = cvd;
        applyCompositePreview();
        emit cvdChanged(cvd);
    });
    }
    else if (parent != nullptr && m_bottomBar->parentWidget() != parent) {
        m_bottomBar->setParent(parent);
    }

    m_bottomBar->setPreviewControlsVisible(includePreview);
    m_bottomBar->setCvdVisible(includeCvd);

    const bool shouldShow = o_ShowLivePreviewWindow.get();
    m_bottomBar->setPreviewChecked(shouldShow);

    return m_bottomBar;
}

void LC_StylingPreviewController::setPreviewVisible(bool visible) {
    if (visible) {
        ensurePreviewWindow();
        if (!m_previewWindow->isVisible()) {
            m_previewWindow->show();
            m_previewWindow->raise();
            // Note: activateWindow() is intentionally omitted on passive navigation
            // so the preferences dialog retains keyboard focus without OS window flicker
        }
    }
    else if (m_previewWindow != nullptr && m_previewWindow->isVisible()) {
        m_previewWindow->hide();
    }
}

bool LC_StylingPreviewController::isPreviewVisible() const {
    return (m_previewWindow != nullptr && m_previewWindow->isVisible());
}

void LC_StylingPreviewController::closePreview() {
    if (m_previewWindow != nullptr) {
        // Disconnect windowClosed so teardown on dialog exit does not overwrite the user's setting
        m_previewWindow->disconnect(this);
        m_previewWindow->close();
        m_previewWindow->deleteLater();
        m_previewWindow = nullptr;
    }
}

LC_PaletteColorUtils::CVDType LC_StylingPreviewController::activeCvdType() const {
    return (m_bottomBar != nullptr) ? m_bottomBar->activeCvdType() : LC_PaletteColorUtils::CVDType::Normal;
}

bool LC_StylingPreviewController::isDisabledSimulated() const {
    return (m_bottomBar != nullptr) && m_bottomBar->isDisabledStateChecked();
}

void LC_StylingPreviewController::activatePreviewTab(const QString& tag) {
    if (m_previewWindow != nullptr) {
        m_previewWindow->activateTab(tag);
    }
}

void LC_StylingPreviewController::hidePreviewTemporarily() const {
    if (m_previewWindow != nullptr && m_previewWindow->isVisible()) {
        m_previewWindow->hide();
    }
}

void LC_StylingPreviewController::onCategoryChanged(LC_SettingsPageInterface* page) {
    const bool supportsPreview = (page != nullptr && page->acceptsSharedPreview());

    if (!supportsPreview) {
        hidePreviewTemporarily();
    }
    else if (o_ShowLivePreviewWindow.get()) {
        if (m_previewWindow == nullptr || !m_previewWindow->isVisible()) {
            setPreviewVisible(true);
        }
        if (m_previewWindow != nullptr) {
            m_previewWindow->applyToolbarsAndDocksConfig();
        }
    }
}

void LC_StylingPreviewController::updatePreviewPalette(const PaletteConfig& palette, bool isDarkMode) {
    m_state.palette = palette;
    m_state.isDarkMode = isDarkMode;
    applyCompositePreview();
}

void LC_StylingPreviewController::updatePreviewSkin(const ControlStyleConfig& skin) {
    m_state.skin = skin;
    applyCompositePreview();
}

void LC_StylingPreviewController::updatePreviewMetrics(const StyleMetricsConfig& metrics) {
    m_state.metrics = metrics;
    applyCompositePreview();
}

void LC_StylingPreviewController::updatePreviewTypography(const FontConfig& font) {
    m_state.font = font;
    applyCompositePreview();
}

void LC_StylingPreviewController::updatePreviewToolbarsAndDocks() {
    if (m_previewWindow != nullptr) {
        m_previewWindow->applyToolbarsAndDocksConfig();
    }
}
void LC_StylingPreviewController::applyCompositePreview() {
    if (m_previewWindow == nullptr) {
        return;
    }

    const ColorSchemeData& scheme = m_state.isDarkMode ? m_state.palette.dark : m_state.palette.light;
    const QPalette previewPalette = LC_PaletteColorUtils::createPaletteFromScheme(
        scheme, m_state.skin.styleArchetype, m_state.cvd);

    m_previewWindow->setPalette(previewPalette);

    QStyle* baseStyle = QStyleFactory::create("Fusion");
    auto* proxyStyle = new LC_ProxyStyle(baseStyle, m_state.metrics);
    proxyStyle->setSkin(m_state.skin);
    proxyStyle->setFont(m_state.font);
    m_previewWindow->setStyle(proxyStyle);

    // Apply custom palette again after setStyle()
    m_previewWindow->setPalette(previewPalette);

    applyTypographyToPreview(m_state.font);

    m_previewWindow->setStyleSheet(scheme.qss);


    m_previewWindow->applyToolbarsAndDocksConfig();

    // Resolve in-progress semantic colors from the active preview state
    const QColor searchColor = scheme.semanticColors.value(SEMANTIC_COLOR_KEY_FILTERED_ITEM, QColor("#2a82da"));
    const QColor conflictColor = scheme.semanticColors.value(SEMANTIC_COLOR_KEY_CONFLICTING_ITEM, m_state.isDarkMode ? QColor("#e06c77") : QColor("#d9534f"));
    m_previewWindow->updateSemanticViews(searchColor, conflictColor);

    for (QWidget* child : m_previewWindow->findChildren<QWidget*>()) {
        child->setStyle(proxyStyle);
        child->setPalette(previewPalette);
        child->updateGeometry();
        child->update();
    }

    if (auto* tabs = m_previewWindow->findChild<QTabWidget*>()) {
        tabs->setEnabled(!m_state.isSimulatedDisabled);
    }

    if (m_previewWindow->layout() != nullptr) {
        m_previewWindow->layout()->invalidate();
        m_previewWindow->layout()->activate();
    }

    m_previewWindow->update();
}

void LC_StylingPreviewController::applyTypographyToPreview(const FontConfig& font) const {
    if (m_previewWindow == nullptr) {
        return;
    }

    auto makeFont = [&font](const FontRoleConfig& role, const QString& familyOverride = QString()) -> QFont {
        const QString family = familyOverride.isEmpty() ? font.mainFamily : familyOverride;
        QFont f(family, qMax(6, font.mainSize + role.sizeOffset));
        f.setBold(role.bold);
        f.setItalic(role.italic);
        return f;
    };

    // 1. Resolve role fonts
    const QFont baseFont(font.mainFamily, font.mainSize);
    const QFont headingsFont   = makeFont(font.headings);
    const QFont tabBarsFont    = makeFont(font.tabBars);
    const QFont menuBarFont    = makeFont(font.menuBar);
    const QFont menusFont      = makeFont(font.menus);
    const QFont buttonsFont    = makeFont(font.buttons);
    const QFont inputsFont     = makeFont(font.inputs);
    const QFont groupBoxesFont = makeFont(font.groupBoxes);
    const QFont itemViewsFont  = makeFont(font.itemViews);
    const QFont technicalFont  = makeFont(font.technical, font.techFamily);
    const QFont statusBarFont  = makeFont(font.statusBar);
    const QFont genericDockFont = makeFont(font.genericDockTitle);
    const QFont specialDockFont = makeFont(font.specialDockTitle);
    const QFont propertiesFont  = makeFont(font.propertiesWidget);

    // 2. Base container font
    m_previewWindow->setFont(baseFont);

    // 3. Tab Bars
    for (auto* tb : m_previewWindow->findChildren<QTabBar*>()) {
        if (tb != nullptr) {
            tb->setFont(tabBarsFont);
        }
    }

    // 4. Section Headings (Headers)
    for (auto* hv : m_previewWindow->findChildren<QHeaderView*>()) {
        if (hv != nullptr) {
            hv->setFont(headingsFont);
        }
    }

    // 5. Item Views (Trees, Lists, Tables)
    for (auto* iv : m_previewWindow->findChildren<QAbstractItemView*>()) {
        if (iv != nullptr) {
            iv->setFont(itemViewsFont);
        }
    }

    // 6. Buttons & Toolbars
    for (auto* btn : m_previewWindow->findChildren<QAbstractButton*>()) {
        if (btn != nullptr && !btn->inherits("QRadioButton") && !btn->inherits("QCheckBox")) {
            btn->setFont(buttonsFont);
        }
    }
    for (auto* tb : m_previewWindow->findChildren<QToolBar*>()) {
        if (tb != nullptr && tb->property("_group").toInt() != 3) {
            tb->setFont(buttonsFont);
        }
    }

    // 7. Inputs & Spinboxes
    for (auto* le : m_previewWindow->findChildren<QLineEdit*>()) {
        if (le != nullptr) {
            le->setFont(inputsFont);
        }
    }
    for (auto* sb : m_previewWindow->findChildren<QAbstractSpinBox*>()) {
        if (sb != nullptr) {
            sb->setFont(inputsFont);
        }
    }
    for (auto* cb : m_previewWindow->findChildren<QComboBox*>()) {
        if (cb != nullptr) {
            cb->setFont(inputsFont);
        }
    }

    // 8. Group Boxes (Title font isolated to title rendering; see Section 2)
    for (auto* gb : m_previewWindow->findChildren<QGroupBox*>()) {
        if (gb != nullptr) {
            gb->setProperty("_lc_custom_title_font", groupBoxesFont);
            gb->update();
        }
        }

    // 9. Status Bar & Status Pill Toolbars
    for (auto* sb : m_previewWindow->findChildren<QStatusBar*>()) {
        if (sb != nullptr) {
            sb->setFont(statusBarFont);
        }
        }
    for (auto* tb : m_previewWindow->findChildren<QToolBar*>()) {
        if (tb != nullptr && tb->property("_group").toInt() == 3) {
            tb->setFont(statusBarFont);
        }
        }

    // 10. Technical / Monospace Areas
    for (auto* te : m_previewWindow->findChildren<QPlainTextEdit*>()) {
        if (te != nullptr) {
            te->setFont(technicalFont);
        }
    }
    for (auto* te : m_previewWindow->findChildren<QTextEdit*>()) {
        if (te != nullptr) {
            te->setFont(technicalFont);
        }
}

    m_previewWindow->update();
}
