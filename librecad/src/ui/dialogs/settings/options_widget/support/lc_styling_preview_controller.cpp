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
#include <QLayout>
#include <QStyleFactory>
#include <QTabWidget>
#include "lc_caddockwidget.h"
#include "lc_proxy_style.h"
#include "lc_skin_preview_window.h"
#include "lc_styling_preview_bottom_bar.h"

LC_StylingPreviewController::LC_StylingPreviewController(QWidget* dialogParent, QObject* parent)
    : QObject(parent)
    , m_dialogParent(dialogParent) {
}

LC_StylingPreviewController::~LC_StylingPreviewController() {
    closePreview();
}

void LC_StylingPreviewController::setDialogParent(QWidget* dialogParent) {
    m_dialogParent = dialogParent;
}

void LC_StylingPreviewController::ensurePreviewWindow() {
    if (m_previewWindow == nullptr) {
        m_previewWindow = new LC_SkinPreviewWindow(m_dialogParent.data());
        m_previewWindow->setWindowFlags(Qt::Window);

        if (m_dialogParent != nullptr) {
            m_previewWindow->move(m_dialogParent->geometry().right() + 10, m_dialogParent->geometry().top());
        }

        connect(m_previewWindow, &LC_SkinPreviewWindow::windowClosed, this, [this]() {
            if (m_bottomBar != nullptr) {
                m_bottomBar->setPreviewChecked(false);
            }
        });
    }
}

QWidget* LC_StylingPreviewController::createBottomWidget(bool includePreview, bool includeCvd, QWidget* parent) {
    m_bottomBar = new LC_StylingPreviewBottomBar(parent);
    m_bottomBar->setPreviewControlsVisible(includePreview);
    m_bottomBar->setCvdVisible(includeCvd);

    connect(m_bottomBar, &LC_StylingPreviewBottomBar::previewToggled, this, &LC_StylingPreviewController::setPreviewVisible);
    connect(m_bottomBar, &LC_StylingPreviewBottomBar::disabledStateToggled, this, [this](bool disabled) {
        if (m_previewWindow != nullptr) {
            if (auto* tabs = m_previewWindow->findChild<QTabWidget*>()) {
                tabs->setEnabled(!disabled);
            }
        }
        emit disabledStateChanged(disabled);
    });
    connect(m_bottomBar, &LC_StylingPreviewBottomBar::cvdChanged, this, &LC_StylingPreviewController::cvdChanged);

    return m_bottomBar;
}

void LC_StylingPreviewController::setPreviewVisible(bool visible) {
    if (visible) {
        ensurePreviewWindow();
        if (m_bottomBar != nullptr) {
            if (auto* tabs = m_previewWindow->findChild<QTabWidget*>()) {
                tabs->setEnabled(!m_bottomBar->isDisabledStateChecked());
            }
        }
        m_previewWindow->show();
        m_previewWindow->raise();
        m_previewWindow->activateWindow();
    }
    else if (m_previewWindow != nullptr) {
        m_previewWindow->hide();
    }
}

bool LC_StylingPreviewController::isPreviewVisible() const {
    return (m_previewWindow != nullptr && m_previewWindow->isVisible());
}

void LC_StylingPreviewController::closePreview() {
    if (m_previewWindow != nullptr) {
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

void LC_StylingPreviewController::updatePreviewTypography(const FontConfig& font) {
    if (!isPreviewVisible()) {
        return;
    }
    applyTypographyToPreview(font);
}

void LC_StylingPreviewController::applyTypographyToPreview(const FontConfig& font) const {
    if (m_previewWindow == nullptr) {
        return;
    }

    const QFont mainFont(font.mainFamily, font.mainSize);

    QFont headingFont(font.mainFamily, font.mainSize + font.headings.sizeOffset);
    headingFont.setBold(font.headings.bold);
    headingFont.setItalic(font.headings.italic);

    QFont menuBarFont(font.mainFamily, font.mainSize + font.menuBar.sizeOffset);
    menuBarFont.setBold(font.menuBar.bold);
    menuBarFont.setItalic(font.menuBar.italic);

    QFont menuFont(font.mainFamily, font.mainSize + font.menus.sizeOffset);
    menuFont.setBold(font.menus.bold);
    menuFont.setItalic(font.menus.italic);

    QFont buttonFont(font.mainFamily, font.mainSize + font.buttons.sizeOffset);
    buttonFont.setBold(font.buttons.bold);
    buttonFont.setItalic(font.buttons.italic);

    QFont inputFont(font.mainFamily, font.mainSize + font.inputs.sizeOffset);
    inputFont.setBold(font.inputs.bold);
    inputFont.setItalic(font.inputs.italic);

    QFont genericDockFont(font.mainFamily, font.mainSize + font.genericDockTitle.sizeOffset);
    genericDockFont.setBold(font.genericDockTitle.bold);
    genericDockFont.setItalic(font.genericDockTitle.italic);

    QFont specialDockFont(font.mainFamily, font.mainSize + font.specialDockTitle.sizeOffset);
    specialDockFont.setBold(font.specialDockTitle.bold);
    specialDockFont.setItalic(font.specialDockTitle.italic);

    QFont techFont(font.techFamily, font.mainSize + font.technical.sizeOffset);
    techFont.setBold(font.technical.bold);
    techFont.setItalic(font.technical.italic);

    m_previewWindow->setFont(mainFont);

    const QList<QWidget*> children = m_previewWindow->findChildren<QWidget*>();
    for (QWidget* child : children) {
        if (child->inherits("QHeaderView") || child->inherits("QTabBar")) {
            child->setFont(headingFont);
        }
        else if (child->inherits("QMenuBar")) {
            child->setFont(menuBarFont);
        }
        else if (child->inherits("QMenu")) {
            child->setFont(menuFont);
        }
        else if (child->inherits("QDockWidget") || child->inherits("LC_CustomTitleBarWidget")) {
            const bool isSpecial = child->property(LC_CADDockWidget::PROPERTY_CAD_DOC_WIDGET).toBool() ||
                                   (child->parentWidget() && child->parentWidget()->property(LC_CADDockWidget::PROPERTY_CAD_DOC_WIDGET).toBool());
            child->setFont(isSpecial ? specialDockFont : genericDockFont);
        }
        else if (child->inherits("QToolBar") || child->inherits("QToolButton") || child->inherits("QPushButton")) {
            child->setFont(buttonFont);
        }
        else if (child->inherits("QLineEdit") || child->inherits("QComboBox") || child->inherits("QAbstractSpinBox")) {
            child->setFont(inputFont);
        }
        else if (child->inherits("QTextEdit") || child->inherits("QPlainTextEdit") || child->inherits("QListWidget")) {
            child->setFont(techFont);
        }
        else {
            child->setFont(mainFont);
        }
        child->update();
    }
    m_previewWindow->update();
}

void LC_StylingPreviewController::updatePreviewMetrics(const StyleMetricsConfig& metrics, const SkinConfig& activeSkin) const {
    if (!isPreviewVisible()) {
        return;
    }
    QStyle* baseStyle = QStyleFactory::create("Fusion");
    auto* proxyStyle = new LC_ProxyStyle(baseStyle, metrics);
    proxyStyle->setSkin(activeSkin);
    m_previewWindow->setStyle(proxyStyle);

    for (QWidget* child : m_previewWindow->findChildren<QWidget*>()) {
        child->setStyle(proxyStyle);
        child->updateGeometry();
        child->update();
    }

    if (m_previewWindow->layout() != nullptr) {
        m_previewWindow->layout()->invalidate();
        m_previewWindow->layout()->activate();
    }
    m_previewWindow->update();
}

void LC_StylingPreviewController::updatePreviewSkin(const SkinConfig& skin, bool isDarkMode, LC_PaletteColorUtils::CVDType cvd) const {
    if (!isPreviewVisible()) {
        return;
    }
    const ColorSchemeData& scheme = isDarkMode ? skin.dark : skin.light;
    const QPalette previewPalette = LC_PaletteColorUtils::createPaletteFromScheme(scheme, skin.styleArchetype, cvd);
    m_previewWindow->setPalette(previewPalette);
    m_previewWindow->setStyleSheet(scheme.qss);

    QStyle* baseStyle = QStyleFactory::create("Fusion");
    auto* proxyStyle = new LC_ProxyStyle(baseStyle);
    proxyStyle->setSkin(skin);
    m_previewWindow->setStyle(proxyStyle);

    for (QWidget* child : m_previewWindow->findChildren<QWidget*>()) {
        child->setStyle(proxyStyle);
        child->setPalette(previewPalette);
        child->update();
    }
    m_previewWindow->update();
}
