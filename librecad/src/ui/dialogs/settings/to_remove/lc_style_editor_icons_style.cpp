/*******************************************************************************
 *
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

#include "lc_style_editor_icons_style.h"

#include <QCheckBox>
#include <QComboBox>

#include "ui_lc_style_editor_icons_style.h"
#include "lc_ui_style_manager.h"
#include "lc_preset_container_widget.h"
#include "lc_color_button.h"
#include "lc_icons_color_utils.h"
#include "lc_icons_style_manager.h"
#include "lc_icons_style_repository.h"
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QRadioButton>
#include <qtablewidget.h>
#include <QToolButton>

LC_StyleEditorIconsStyle::LC_StyleEditorIconsStyle(QWidget* parent, LC_UIStyleManager* styleManager)
    : LC_StyleEditorBase(parent, styleManager, styleManager->getIconsStyleRepository())
    , ui(new Ui::LC_StyleEditorIconsStyle) {
    ui->setupUi(this);

    m_currentVariantDark = LC_PaletteColorUtils::isSystemInDarkMode();
    ui->rbDarkMode->setChecked(m_currentVariantDark);
    ui->rbLightMode->setChecked(!m_currentVariantDark);

    setupComboboxes();
    setupStatesTableStructure();
    setupHarmonizationButton();
    setupAccentControls();
    setupConnections();
}

LC_StyleEditorIconsStyle::~LC_StyleEditorIconsStyle() {
    delete ui;
}

void LC_StyleEditorIconsStyle::setupComboboxes() const {
    auto configureRoleCombo = [](QComboBox* cb, const QList<QPair<QString, QString>>& roles) {
        cb->clear();
        for (const auto& role : roles) {
            cb->addItem(role.first, role.second);
        }
    };

    QList<QPair<QString, QString>> mainRoles = {
        { tr("Theme Button Text (ButtonText)"), "ButtonText" },
        { tr("Theme Window Text (WindowText)"), "WindowText" },
        { tr("Theme Input Text (Text)"), "Text" }
    };
    configureRoleCombo(ui->cbMainPaletteRole, mainRoles);

    QList<QPair<QString, QString>> accentRoles = {
        { tr("Active Highlight (Highlight)"), "Highlight" },
        { tr("Interactive Link (Link)"), "Link" }
    };
    configureRoleCombo(ui->cbAccentPaletteRole, accentRoles);
    configureRoleCombo(ui->cbAccentAltPaletteRole, accentRoles);

    QList<QPair<QString, QString>> backRoles = {
        { tr("Fully Transparent (None)"), "transparent" },
        { tr("Theme Button Background (Button)"), "Button" },
        { tr("Theme Window Background (Window)"), "Window" },
        { tr("Theme Input Background (Base)"), "Base" }
    };
    configureRoleCombo(ui->cbBackPaletteRole, backRoles);
}

void LC_StyleEditorIconsStyle::setupStatesTableStructure() {
    ui->tableStates->setColumnCount(4);
    ui->tableStates->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("State")));
    ui->tableStates->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Main Color")));
    ui->tableStates->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Accent Color")));
    ui->tableStates->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Back Color")));

    ui->tableStates->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableStates->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableStates->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableStates->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->tableStates->verticalHeader()->setVisible(false);
    ui->tableStates->verticalHeader()->setDefaultSectionSize(24);

    ui->tableStates->setRowCount(ICON_STATE_ROWS.size());
    for (int row = 0; row < ui->tableStates->rowCount(); ++row) {
        const QString stateLabel = ICON_STATE_ROWS[row].name;
        auto* item = new QTableWidgetItem(tr(stateLabel.toUtf8().constData()));
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        ui->tableStates->setItem(row, 0, item);

        for (int colIdx = 1; colIdx <= 3; ++colIdx) {
            auto* btn = new LC_ColorButton(ui->tableStates);

            QString colName = (colIdx == 1) ? tr("Main (Stroke)") : ((colIdx == 2) ? tr("Accent (Detail)") : tr("Back (Fill)"));
            QString baseTooltip = tr("%1 color for the %2 state.").arg(colName, tr(stateLabel.toUtf8().constData()));
            btn->setToolTip(baseTooltip);
            btn->setLockedToolTip(baseTooltip + "\n" + tr("[Auto-calculated: Uncheck 'Auto-calculate Advanced States' to edit manually]"));
            btn->setDialogTitle(tr("Select %1 Color for State: %2").arg(colName, tr(stateLabel.toUtf8().constData())));

            connect(btn, &LC_ColorButton::colorChanged, this, &LC_StyleEditorIconsStyle::onControlChanged);
            ui->tableStates->setCellWidget(row, colIdx, btn);
        }
    }
}

void LC_StyleEditorIconsStyle::setupHarmonizationButton() {
    ui->tbHarmonizeSeeds->setPopupMode(QToolButton::InstantPopup);
    auto* generateMenu = new QMenu(ui->tbHarmonizeSeeds);

    QAction* actSoft   = generateMenu->addAction(QIcon(":/icons/palette_1_point.lci"), tr("Soft style"));
    QAction* actBalanced     = generateMenu->addAction(QIcon(":/icons/palette_2_points.lci"), tr("Balanced"));
    QAction* actHighContrast = generateMenu->addAction(QIcon(":/icons/palette_hi_contrast.lci"), tr("High-Contrast"));

    actSoft->setProperty("_harmonizeMode", static_cast<int>(IconContrastMode::Soft));
    actBalanced->setProperty("_harmonizeMode", static_cast<int>(IconContrastMode::Balanced));
    actHighContrast->setProperty("_harmonizeMode", static_cast<int>(IconContrastMode::Contrast));

    ui->tbHarmonizeSeeds->setMenu(generateMenu);

    connect(actSoft, &QAction::triggered, this, &LC_StyleEditorIconsStyle::onHarmonizeSeedsPressed);
    connect(actBalanced, &QAction::triggered, this, &LC_StyleEditorIconsStyle::onHarmonizeSeedsPressed);
    connect(actHighContrast, &QAction::triggered, this, &LC_StyleEditorIconsStyle::onHarmonizeSeedsPressed);
}

void LC_StyleEditorIconsStyle::setupAccentControls() {
    ui->chkSyncAccents->setChecked(true);

    connect(ui->btnAccentColor, &LC_ColorButton::colorChanged, this, [this](const QColor &color) {
        if (ui->chkSyncAccents->isChecked()) {
            ui->btnAccentAltColor->blockSignals(true);
            ui->btnAccentAltColor->setColor(color);
            ui->btnAccentAltColor->blockSignals(false);
            onControlChanged();
        }
    });

    connect(ui->btnAccentAltColor, &LC_ColorButton::colorChanged, this, [this](const QColor &color) {
        if (ui->chkSyncAccents->isChecked()) {
            ui->btnAccentColor->blockSignals(true);
            ui->btnAccentColor->setColor(color);
            ui->btnAccentColor->blockSignals(false);
            onControlChanged();
        }
    });

    // Automatically disable synchronization if either seed is linked to the palette manually
    auto checkSyncDisable = [this](bool checked) {
        if (checked) {
            ui->chkSyncAccents->setChecked(false);
        }
    };
    connect(ui->chkAccentPalette, &QCheckBox::toggled, this, checkSyncDisable);
    connect(ui->chkAccentAltPalette, &QCheckBox::toggled, this, checkSyncDisable);
}

void LC_StyleEditorIconsStyle::setupConnections() {
    connect(ui->rbLightMode, &QRadioButton::toggled, this, &LC_StyleEditorIconsStyle::onVariantTabChanged);
    connect(ui->rbDarkMode, &QRadioButton::toggled, this, &LC_StyleEditorIconsStyle::onVariantTabChanged);

    connect(ui->pbAutoGenerateOpposite, &QPushButton::clicked, this, &LC_StyleEditorIconsStyle::onAutoGenerateOppositeSchemePressed);
    connect(ui->chkAutoCalcStates, &QCheckBox::toggled, this, &LC_StyleEditorIconsStyle::onAutoCalcStatesToggled);

    auto wireCompoundControl = [this](QCheckBox* chk, LC_ColorButton* btn, QComboBox* cb) {
        connect(chk, &QCheckBox::toggled, this, [=](bool checked) {
            btn->setVisible(!checked);
            cb->setVisible(checked);
            onControlChanged();
        });
        connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_StyleEditorIconsStyle::onControlChanged);
    };

    wireCompoundControl(ui->chkMainPalette, ui->btnMainColor, ui->cbMainPaletteRole);
    wireCompoundControl(ui->chkAccentPalette, ui->btnAccentColor, ui->cbAccentPaletteRole);
    wireCompoundControl(ui->chkAccentAltPalette, ui->btnAccentAltColor, ui->cbAccentAltPaletteRole);
    wireCompoundControl(ui->chkBackPalette, ui->btnBackColor, ui->cbBackPaletteRole);
}

void LC_StyleEditorIconsStyle::onControlChanged() {
    if (m_blockSignals) return;

    if (ui->chkAutoCalcStates->isChecked()) {
        calculateAndApplyAutoStates();
    }

    saveUiToConfig();
    notifyChanged(true);
}

void LC_StyleEditorIconsStyle::onVariantTabChanged(bool checked) {
    if (m_blockSignals || !checked) return;

    bool newVariantDark = ui->rbDarkMode->isChecked();
    if (newVariantDark == m_currentVariantDark) return;

    m_blockSignals = true;
    saveUiToConfig();
    m_iconColorsOptions.exportStyleConfig(m_currentIconStyle, m_currentVariantDark);

    m_currentVariantDark = newVariantDark;
    m_iconColorsOptions.importStyleConfig(m_currentIconStyle, m_currentVariantDark);

    m_blockSignals = false;
    loadConfigToUi();
}

bool LC_StyleEditorIconsStyle::doLoadPreset(const QString& key) {
    if (key == DEFAULT_THEME_KEY) {
        m_iconColorsOptions.resetToDefaults();
        m_currentIconStyle = IconStyleConfig();
        m_currentIconStyle.name = "Default";
        m_iconColorsOptions.exportStyleConfig(m_currentIconStyle, true);
        m_iconColorsOptions.exportStyleConfig(m_currentIconStyle, false);
    } else {
        m_styleManager->getIconsStyleRepository()->loadByKey(key, m_currentIconStyle);
        m_iconColorsOptions.importStyleConfig(m_currentIconStyle, m_currentVariantDark);
    }


    m_blockSignals = true;
    loadConfigToUi();
    m_blockSignals = false;

    return true;
}

void LC_StyleEditorIconsStyle::loadConfigToUi() {
    m_blockSignals = true;

    loadSeedControl(m_iconColorsOptions.getColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Main),
                    ui->chkMainPalette, ui->btnMainColor, ui->cbMainPaletteRole);
    loadSeedControl(m_iconColorsOptions.getColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Accent),
                    ui->chkAccentPalette, ui->btnAccentColor, ui->cbAccentPaletteRole);
    loadSeedControl(m_iconColorsOptions.getColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::Accent),
                    ui->chkAccentAltPalette, ui->btnAccentAltColor, ui->cbAccentAltPaletteRole);
    loadSeedControl(m_iconColorsOptions.getColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Background),
                    ui->chkBackPalette, ui->btnBackColor, ui->cbBackPaletteRole);

    ui->chkAutoCalcStates->setChecked(m_iconColorsOptions.isAutoCalculateStates());

    m_blockSignals = false;

    if (m_iconColorsOptions.isAutoCalculateStates()) {
        calculateAndApplyAutoStates();
    } else {
        ui->tableStates->blockSignals(true);
        for (int row = 0; row < ui->tableStates->rowCount(); ++row) {
            const auto mode = ICON_STATE_ROWS[row].mode;
            const auto state = ICON_STATE_ROWS[row].state;

            auto* btnMain   = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 1));
            auto* btnAccent = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 2));
            auto* btnBack   = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 3));

            if (btnMain) {
                QString colorStr = m_iconColorsOptions.getColor(mode, state, LC_SVGIconEngineAPI::Main);
                btnMain->setColor(colorStr.isEmpty() ? QColor() : QColor(colorStr));
                btnMain->setLocked(false);
            }
            if (btnAccent) {
                QString colorStr = m_iconColorsOptions.getColor(mode, state, LC_SVGIconEngineAPI::Accent);
                btnAccent->setColor(colorStr.isEmpty() ? QColor() : QColor(colorStr));
                btnAccent->setLocked(false);
            }
            if (btnBack) {
                QString colorStr = m_iconColorsOptions.getColor(mode, state, LC_SVGIconEngineAPI::Background);
                btnBack->setColor(colorStr.isEmpty() ? QColor() : QColor(colorStr));
                btnBack->setLocked(false);
            }
        }
        ui->tableStates->blockSignals(false);
    }
}

bool LC_StyleEditorIconsStyle::saveCurrentPreset() {
    saveUiToConfig();
    m_iconColorsOptions.exportStyleConfig(m_currentIconStyle, m_currentVariantDark);
    QString outKey;
    return m_styleManager->getIconsStyleRepository()->save(m_currentIconStyle.name, m_currentIconStyle, outKey);
}

bool LC_StyleEditorIconsStyle::doSavePresetAs(const QString& name, QString& key) {
    saveUiToConfig();
    m_currentIconStyle.name = name;
    m_iconColorsOptions.exportStyleConfig(m_currentIconStyle, m_currentVariantDark);
    return m_styleManager->getIconsStyleRepository()->save(name, m_currentIconStyle, key);
}

void LC_StyleEditorIconsStyle::saveUiToConfig() {
    m_iconColorsOptions.setColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Main,
                                  getSeedValueFromUi(ui->chkMainPalette, ui->btnMainColor, ui->cbMainPaletteRole));
    m_iconColorsOptions.setColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Accent,
                                  getSeedValueFromUi(ui->chkAccentPalette, ui->btnAccentColor, ui->cbAccentPaletteRole));
    m_iconColorsOptions.setColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::Accent,
                                  getSeedValueFromUi(ui->chkAccentAltPalette, ui->btnAccentAltColor, ui->cbAccentAltPaletteRole));
    m_iconColorsOptions.setColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Background,
                                  getSeedValueFromUi(ui->chkBackPalette, ui->btnBackColor, ui->cbBackPaletteRole));

    m_iconColorsOptions.setAutoCalculateStates(ui->chkAutoCalcStates->isChecked());

    for (int row = 0; row < ui->tableStates->rowCount(); ++row) {
        const auto mode = ICON_STATE_ROWS[row].mode;
        const auto state = ICON_STATE_ROWS[row].state;

        const auto* btnMain   = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 1));
        const auto* btnAccent = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 2));
        const auto* btnBack   = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 3));

        if (btnMain)
            m_iconColorsOptions.setColor(mode, state, LC_SVGIconEngineAPI::Main,
                (btnMain->color().isValid() && btnMain->color().alpha() > 0) ? btnMain->color().name(QColor::HexRgb) : "");
        if (btnAccent)
            m_iconColorsOptions.setColor(mode, state, LC_SVGIconEngineAPI::Accent,
                (btnAccent->color().isValid() && btnAccent->color().alpha() > 0) ? btnAccent->color().name(QColor::HexRgb) : "");
        if (btnBack)
            m_iconColorsOptions.setColor(mode, state, LC_SVGIconEngineAPI::Background,
                (btnBack->color().isValid() && btnBack->color().alpha() > 0) ? btnBack->color().name(QColor::HexRgb) : "");
    }
}

QString LC_StyleEditorIconsStyle::getActivePresetKey() const {
    return m_styleManager->getActiveIconStyle();
}

QList<QPair<QString, QString>> LC_StyleEditorIconsStyle::getAvailablePresets() const {
    QList<QPair<QString, QString>> choices;
    choices.append(qMakePair(tr("Default (Classic)"), DEFAULT_THEME_KEY));
    choices.append(m_styleManager->getIconsStyleRepository()->getPresetChoices());
    return choices;
}

void LC_StyleEditorIconsStyle::applyTransientState(QWidget* previewWindow) const {
    Q_UNUSED(previewWindow);


    // Symmetrical Preview updates the globally running main application icons on-the-fly
    LC_IconColorsOptions tempOptions;
    tempOptions.loadSettings();

    const_cast<LC_StyleEditorIconsStyle*>(this)->saveUiToConfig();
    m_iconColorsOptions.exportStyleConfig(const_cast<IconStyleConfig&>(m_currentIconStyle), m_currentVariantDark);
    tempOptions.importStyleConfig(m_currentIconStyle, m_currentVariantDark);

    auto cvdType = LC_PaletteColorUtils::CVDType::Normal;
    auto* container = qobject_cast<LC_PresetContainerWidget*>(parentWidget());
    if (!container && parentWidget()) {
        container = qobject_cast<LC_PresetContainerWidget*>(parentWidget()->parentWidget());
    }
    if (container) {
        cvdType = container->activeCvdType();
    }

    LC_IconsStyleManager::applyStyle(tempOptions, m_currentVariantDark, cvdType);
}

void LC_StyleEditorIconsStyle::applyCurrentPreset() {
    m_styleManager->setActiveIconStyle(m_currentPresetKey);
    m_styleManager->applyActiveIconStyle();
}

void LC_StyleEditorIconsStyle::rollbackState() {
    // Standard rollback restores the options state
}

void LC_StyleEditorIconsStyle::onAutoCalcStatesToggled(bool checked) {
    if (m_blockSignals) return;
    saveUiToConfig();
    m_iconColorsOptions.setAutoCalculateStates(checked);

    if (checked) {
        calculateAndApplyAutoStates();
        saveUiToConfig();
    } else {
        for (int row = 0; row < ui->tableStates->rowCount(); ++row) {
            for (int col = 1; col <= 3; ++col) {
                auto* btn = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, col));
                if (btn) btn->setLocked(false);
            }
        }
    }
    notifyChanged(true);
    applyTransientState(nullptr);
}

void LC_StyleEditorIconsStyle::onAutoGenerateOppositeSchemePressed() {
    saveUiToConfig();
    m_iconColorsOptions.exportStyleConfig(m_currentIconStyle, m_currentVariantDark);

    LC_IconColorsOptions tempOptions = m_iconColorsOptions;
    bool toDark = !m_currentVariantDark;

    auto runBidiConversion = [&](LC_SVGIconEngineAPI::IconMode mode, LC_SVGIconEngineAPI::IconState state, bool targetDark) {
        auto convertColorType = [&](LC_SVGIconEngineAPI::ColorType type) {
            QString colorStr = tempOptions.getColor(mode, state, type);
            if (colorStr.isEmpty()) return;

            QColor color = QColor(colorStr);
            int h, s, v;
            color.getHsv(&h, &s, &v);

            QColor targetColor;
            if (type == LC_SVGIconEngineAPI::Main || type == LC_SVGIconEngineAPI::Background) {
                targetColor.setHsv(h, s, qBound(40, 255 - v, 240));
            } else if (type == LC_SVGIconEngineAPI::Accent) {
                if (targetDark) {
                    targetColor.setHsv(h, qBound(0, static_cast<int>(s * 0.70), 255), qBound(0, v + 50, 255));
                } else {
                    targetColor.setHsv(h, qBound(0, qMin(255, static_cast<int>(s * 1.35)), 255), qBound(0, qMax(0, v - 40), 255));
                }
            }
            tempOptions.setColor(mode, state, type, targetColor.name(QColor::HexArgb));
        };

        convertColorType(LC_SVGIconEngineAPI::Main);
        convertColorType(LC_SVGIconEngineAPI::Accent);
        convertColorType(LC_SVGIconEngineAPI::Background);
    };

    runBidiConversion(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, toDark);
    for (const auto& row : ICON_STATE_ROWS) {
        runBidiConversion(row.mode, row.state, toDark);
    }

    tempOptions.exportStyleConfig(m_currentIconStyle, toDark);

    QString targetName = toDark ? tr("Dark Mode Theme") : tr("Light Mode Theme");
    QMessageBox::information(this, tr("Auto-Generation Complete"),
                             tr("Symmetrical opposite scheme generated and saved in the %1 configuration. Switch tabs above to preview or edit.").arg(targetName));

    notifyChanged(true);
}

void LC_StyleEditorIconsStyle::onHarmonizeSeedsPressed() {
    QColor accentColor = ui->btnAccentColor->color();
    if (!accentColor.isValid()) {
        accentColor = QColor("#00ff7f");
    }

    QPalette activePalette = QGuiApplication::palette();
    QColor bgWindow = activePalette.color(QPalette::Window);

    QColor mainColor, harmonizedAccentColor, accentAltColor, backColor;

    IconContrastMode mode = IconContrastMode::Balanced;
    QVariant candidate = sender()->property("_harmonizeMode");
    if (candidate.isValid()) {
        mode = static_cast<IconContrastMode>(candidate.toInt());
    }

    LC_IconsColorUtils::harmonizeSeeds(accentColor, m_currentVariantDark, bgWindow, mode,
                                       AccentHarmonizationPolicy::LuminosityBoost,
                                       mainColor, harmonizedAccentColor, accentAltColor, backColor);

    m_blockSignals = true;

    ui->chkMainPalette->setChecked(false);
    ui->chkAccentPalette->setChecked(false);
    ui->chkAccentAltPalette->setChecked(false);
    ui->chkBackPalette->setChecked(false);

    ui->btnMainColor->setColor(mainColor);
    ui->btnAccentColor->setColor(harmonizedAccentColor);
    ui->btnAccentAltColor->setColor(accentAltColor);
    ui->btnBackColor->setColor(backColor);

    m_blockSignals = false;
    onControlChanged();
}

void LC_StyleEditorIconsStyle::calculateAndApplyAutoStates() {
    m_blockSignals = true;

    QColor mainSeed = QColor(LC_IconsStyleManager::resolveColorValue(getSeedValueFromUi(ui->chkMainPalette, ui->btnMainColor, ui->cbMainPaletteRole)));
    QColor accentSeed = QColor(LC_IconsStyleManager::resolveColorValue(getSeedValueFromUi(ui->chkAccentPalette, ui->btnAccentColor, ui->cbAccentPaletteRole)));
    QColor accentAltSeed = QColor(LC_IconsStyleManager::resolveColorValue(getSeedValueFromUi(ui->chkAccentAltPalette, ui->btnAccentAltColor, ui->cbAccentAltPaletteRole)));
    QColor backSeed = QColor(LC_IconsStyleManager::resolveColorValue(getSeedValueFromUi(ui->chkBackPalette, ui->btnBackColor, ui->cbBackPaletteRole)));

    if (!mainSeed.isValid()) {
        mainSeed = m_currentVariantDark ? QColor("#e6e6e6") : QColor("#000000");
    }
    if (!accentSeed.isValid()) {
        accentSeed = QColor("#00ff7f");
    }
    if (!accentAltSeed.isValid()) {
        accentAltSeed = accentSeed;
    }

    QPalette activePalette = QGuiApplication::palette();
    QColor highlightColor = activePalette.color(QPalette::Highlight);
    QColor highlightedTextColor = activePalette.color(QPalette::HighlightedText);
    QColor windowColor = activePalette.color(QPalette::Window);

    for (int row = 0; row < ui->tableStates->rowCount(); ++row) {
        const auto mode = ICON_STATE_ROWS[row].mode;
        const auto state = ICON_STATE_ROWS[row].state;

        auto* btnMain = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 1));
        auto* btnAccent = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 2));
        auto* btnBack = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 3));

        if (btnMain)   btnMain->setColor(LC_IconsColorUtils::calculateStateColor(mode, state, LC_SVGIconEngineAPI::Main, mainSeed, accentSeed, accentAltSeed, backSeed, highlightColor, highlightedTextColor, windowColor));
        if (btnAccent) btnAccent->setColor(LC_IconsColorUtils::calculateStateColor(mode, state, LC_SVGIconEngineAPI::Accent, mainSeed, accentSeed, accentAltSeed, backSeed, highlightColor, highlightedTextColor, windowColor));
        if (btnBack)   btnBack->setColor(LC_IconsColorUtils::calculateStateColor(mode, state, LC_SVGIconEngineAPI::Background, mainSeed, accentSeed, accentAltSeed, backSeed, highlightColor, highlightedTextColor, windowColor));
    }

    m_blockSignals = false;
}

void LC_StyleEditorIconsStyle::loadSeedControl(const QString& colorStr, QCheckBox* chk, LC_ColorButton* btn, QComboBox* cb) {
    chk->blockSignals(true);
    btn->blockSignals(true);
    cb->blockSignals(true);

    if (colorStr.startsWith("palette:") || colorStr == "transparent") {
        chk->setChecked(true);
        btn->setVisible(false);
        cb->setVisible(true);

        QString roleKey = colorStr.startsWith("palette:") ? colorStr.mid(8) : colorStr;
        int idx = cb->findData(roleKey);
        if (idx >= 0) {
            cb->setCurrentIndex(idx);
        }
    } else {
        chk->setChecked(false);
        btn->setVisible(true);
        cb->setVisible(false);
        btn->setColor(colorStr.isEmpty() ? Qt::white : QColor(colorStr));
    }

    chk->blockSignals(false);
    btn->blockSignals(false);
    cb->blockSignals(false);
}

QString LC_StyleEditorIconsStyle::getSeedValueFromUi(const QCheckBox* chk, const LC_ColorButton* btn, const QComboBox* cb) const {
    if (chk->isChecked()) {
        QString dataVal = cb->currentData().toString();
        if (dataVal == "transparent") return "transparent";
        return "palette:" + dataVal;
    }
    return (btn->color().isValid() && btn->color().alpha() > 0) ? btn->color().name(QColor::HexRgb) : "transparent";
}
