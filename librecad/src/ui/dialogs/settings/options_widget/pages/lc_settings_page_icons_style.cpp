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

#include "lc_settings_page_icons_style.h"
#include "ui_lc_settings_page_icons_style.h"
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QTableWidget>

#include "lc_color_button.h"
#include "lc_icons_color_utils.h"
#include "lc_icons_style_manager.h"
#include "lc_palette_color_utils.h"
#include "lc_preset_manager_icons_style.h"

LC_SettingsPageIconsStyle::LC_SettingsPageIconsStyle(QObject* parent)
    : LC_SettingsPageBase(tr("Icons Style"), nullptr, parent)
    , ui(std::make_unique<Ui::LC_SettingsPageIconsStyle>()) {
    setSortWeight(40);
}

LC_SettingsPageIconsStyle::~LC_SettingsPageIconsStyle() = default;

void LC_SettingsPageIconsStyle::bindToPresetManager(LC_PresetManagerInterface* manager) {
    m_presetManager = dynamic_cast<LC_PresetManagerIconsStyle*>(manager);
    if (m_presetManager != nullptr) {
        connect(m_presetManager, &LC_PresetManagerIconsStyle::configLoaded, this, [this]() {
            populateUiFromWorkingConfig();
        });
        connect(m_presetManager, &LC_PresetManagerIconsStyle::variantChanged, this, [this](bool isDark) {
            m_blockSignals = true;
            ui->rbDarkMode->setChecked(isDark);
            ui->rbLightMode->setChecked(!isDark);
            m_blockSignals = false;
            populateUiFromWorkingConfig();
        });
        populateUiFromWorkingConfig();
    }
}

void LC_SettingsPageIconsStyle::setupUi() {
    ui->setupUi(m_widget);

    const bool isDark = LC_PaletteColorUtils::isSystemInDarkMode();
    ui->rbDarkMode->setChecked(isDark);
    ui->rbLightMode->setChecked(!isDark);

    setupComboboxes();
    setupStatesTableStructure();
    setupHarmonizationButton();
    setupAccentControls();
}

void LC_SettingsPageIconsStyle::setupComboboxes() {
    auto configureRoleCombo = [](QComboBox* cb, const QList<QPair<QString, QString>>& roles) {
        cb->clear();
        for (const auto& role : roles) {
            cb->addItem(role.first, role.second);
        }
    };

    const QList<QPair<QString, QString>> mainRoles = {
        { tr("Theme Button Text (ButtonText)"), "ButtonText" },
        { tr("Theme Window Text (WindowText)"), "WindowText" },
        { tr("Theme Input Text (Text)"), "Text" }
    };
    configureRoleCombo(ui->cbMainPaletteRole, mainRoles);

    const QList<QPair<QString, QString>> accentRoles = {
        { tr("Active Highlight (Highlight)"), "Highlight" },
        { tr("Interactive Link (Link)"), "Link" }
    };
    configureRoleCombo(ui->cbAccentPaletteRole, accentRoles);
    configureRoleCombo(ui->cbAccentAltPaletteRole, accentRoles);

    const QList<QPair<QString, QString>> backRoles = {
        { tr("Fully Transparent (None)"), "transparent" },
        { tr("Theme Button Background (Button)"), "Button" },
        { tr("Theme Window Background (Window)"), "Window" },
        { tr("Theme Input Background (Base)"), "Base" }
    };
    configureRoleCombo(ui->cbBackPaletteRole, backRoles);
}

void LC_SettingsPageIconsStyle::setupStatesTableStructure() {
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

            connect(btn, &LC_ColorButton::colorChanged, this, &LC_SettingsPageIconsStyle::onControlChanged);
            ui->tableStates->setCellWidget(row, colIdx, btn);
        }
    }
}

void LC_SettingsPageIconsStyle::setupHarmonizationButton() {
    ui->tbHarmonizeSeeds->setPopupMode(QToolButton::InstantPopup);
    auto* generateMenu = new QMenu(ui->tbHarmonizeSeeds);

    QAction* actSoft = generateMenu->addAction(QIcon(":/icons/palette_1_point.lci"), tr("Soft Contrast"));
    QAction* actBalanced = generateMenu->addAction(QIcon(":/icons/palette_2_points.lci"), tr("Balanced"));
    QAction* actHighContrast = generateMenu->addAction(QIcon(":/icons/palette_hi_contrast.lci"), tr("High Contrast"));

    actSoft->setProperty("_harmonizeMode", static_cast<int>(IconContrastMode::Soft));
    actBalanced->setProperty("_harmonizeMode", static_cast<int>(IconContrastMode::Balanced));
    actHighContrast->setProperty("_harmonizeMode", static_cast<int>(IconContrastMode::Contrast));

    ui->tbHarmonizeSeeds->setMenu(generateMenu);

    connect(actSoft, &QAction::triggered, this, &LC_SettingsPageIconsStyle::onHarmonizeSeedsPressed);
    connect(actBalanced, &QAction::triggered, this, &LC_SettingsPageIconsStyle::onHarmonizeSeedsPressed);
    connect(actHighContrast, &QAction::triggered, this, &LC_SettingsPageIconsStyle::onHarmonizeSeedsPressed);
}

void LC_SettingsPageIconsStyle::setupAccentControls() {
    ui->chkSyncAccents->setChecked(true);

    connect(ui->btnAccentColor, &LC_ColorButton::colorChanged, this, [this](const QColor& color) {
        if (ui->chkSyncAccents->isChecked()) {
            ui->btnAccentAltColor->blockSignals(true);
            ui->btnAccentAltColor->setColor(color);
            ui->btnAccentAltColor->blockSignals(false);
            onControlChanged();
        }
    });

    connect(ui->btnAccentAltColor, &LC_ColorButton::colorChanged, this, [this](const QColor& color) {
        if (ui->chkSyncAccents->isChecked()) {
            ui->btnAccentColor->blockSignals(true);
            ui->btnAccentColor->setColor(color);
            ui->btnAccentColor->blockSignals(false);
            onControlChanged();
        }
    });

    auto checkSyncDisable = [this](bool checked) {
        if (checked) {
            ui->chkSyncAccents->setChecked(false);
        }
    };
    connect(ui->chkAccentPalette, &QCheckBox::toggled, this, checkSyncDisable);
    connect(ui->chkAccentAltPalette, &QCheckBox::toggled, this, checkSyncDisable);
}

void LC_SettingsPageIconsStyle::setupBehavior() {
    connect(ui->rbLightMode, &QRadioButton::toggled, this, &LC_SettingsPageIconsStyle::onVariantToggled);
    connect(ui->rbDarkMode, &QRadioButton::toggled, this, &LC_SettingsPageIconsStyle::onVariantToggled);

    connect(ui->pbAutoGenerateOpposite, &QPushButton::clicked, this, &LC_SettingsPageIconsStyle::onAutoGenerateOppositeSchemePressed);
    connect(ui->chkAutoCalcStates, &QCheckBox::toggled, this, &LC_SettingsPageIconsStyle::onAutoCalcStatesToggled);

    auto wireCompoundControl = [this](QCheckBox* chk, LC_ColorButton* btn, QComboBox* cb) {
        connect(chk, &QCheckBox::toggled, this, [=](bool checked) {
            btn->setVisible(!checked);
            cb->setVisible(checked);
            onControlChanged();
        });
        connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LC_SettingsPageIconsStyle::onControlChanged);
        connect(btn, &LC_ColorButton::colorChanged, this, &LC_SettingsPageIconsStyle::onControlChanged);
    };

    wireCompoundControl(ui->chkMainPalette, ui->btnMainColor, ui->cbMainPaletteRole);
    wireCompoundControl(ui->chkAccentPalette, ui->btnAccentColor, ui->cbAccentPaletteRole);
    wireCompoundControl(ui->chkAccentAltPalette, ui->btnAccentAltColor, ui->cbAccentAltPaletteRole);
    wireCompoundControl(ui->chkBackPalette, ui->btnBackColor, ui->cbBackPaletteRole);
}

void LC_SettingsPageIconsStyle::loadSettings() {
    populateUiFromWorkingConfig();
}

bool LC_SettingsPageIconsStyle::saveSettings() {
    syncUiToWorkingConfig();
    return true;
}

bool LC_SettingsPageIconsStyle::isModified() const {
    return m_presetManager ? m_presetManager->isPresetModified() : false;
}

void LC_SettingsPageIconsStyle::updateLivePreview() {
    if (m_presetManager != nullptr) {
        syncUiToWorkingConfig();
        m_presetManager->applyTransientStyle();
    }
    emit livePreviewRequested();
}

void LC_SettingsPageIconsStyle::onControlChanged() {
    if (m_blockSignals) {
        return;
    }
    if (ui->chkAutoCalcStates->isChecked()) {
        calculateAndApplyAutoStates();
    }
    syncUiToWorkingConfig();
    if (m_presetManager != nullptr) {
        m_presetManager->notifyWorkingConfigChanged();
    }
}

void LC_SettingsPageIconsStyle::onVariantToggled(bool checked) {
    if (m_blockSignals || !checked || m_presetManager == nullptr) {
        return;
    }
    const bool darkSelected = ui->rbDarkMode->isChecked();
    if (darkSelected == m_presetManager->isCurrentVariantDark()) {
        return;
    }

    syncUiToWorkingConfig();
    m_presetManager->setCurrentVariantDark(darkSelected);
}

void LC_SettingsPageIconsStyle::onAutoCalcStatesToggled(bool checked) {
    if (m_blockSignals || m_presetManager == nullptr) {
        return;
    }
    syncUiToWorkingConfig();
    m_presetManager->iconColorsOptions().setAutoCalculateStates(checked);

    if (checked) {
        calculateAndApplyAutoStates();
        syncUiToWorkingConfig();
    }
    else {
        for (int row = 0; row < ui->tableStates->rowCount(); ++row) {
            for (int col = 1; col <= 3; ++col) {
                if (auto* btn = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, col))) {
                    btn->setLocked(false);
                }
            }
        }
    }
    m_presetManager->notifyWorkingConfigChanged();
}

void LC_SettingsPageIconsStyle::onHarmonizeSeedsPressed() {
    QColor accentColor = ui->btnAccentColor->color();
    if (!accentColor.isValid()) {
        accentColor = QColor("#00ff7f");
    }

    const QPalette activePalette = QGuiApplication::palette();
    const QColor bgWindow = activePalette.color(QPalette::Window);

    QColor mainColor, harmonizedAccentColor, accentAltColor, backColor;
    IconContrastMode mode = IconContrastMode::Balanced;
    const QVariant candidate = sender()->property("_harmonizeMode");
    if (candidate.isValid()) {
        mode = static_cast<IconContrastMode>(candidate.toInt());
    }

    const bool isDark = ui->rbDarkMode->isChecked();
    LC_IconsColorUtils::harmonizeSeeds(accentColor, isDark, bgWindow, mode,
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

void LC_SettingsPageIconsStyle::onAutoGenerateOppositeSchemePressed() {
    if (m_presetManager == nullptr) {
        return;
    }

    syncUiToWorkingConfig();
    const bool currentDark = m_presetManager->isCurrentVariantDark();
    m_presetManager->iconColorsOptions().exportStyleConfig(m_presetManager->workingConfig(), currentDark);

    LC_IconColorsOptions tempOptions = m_presetManager->iconColorsOptions();
    const bool targetDark = !currentDark;

    auto runBidiConversion = [&](LC_SVGIconEngineAPI::IconMode mode, LC_SVGIconEngineAPI::IconState state, bool toDark) {
        auto convertColorType = [&](LC_SVGIconEngineAPI::ColorType type) {
            QString colorStr = tempOptions.getColor(mode, state, type);
            if (colorStr.isEmpty()) {
                return;
            }

            QColor color = QColor(colorStr);
            int h, s, v;
            color.getHsv(&h, &s, &v);

            QColor targetColor;
            if (type == LC_SVGIconEngineAPI::Main || type == LC_SVGIconEngineAPI::Background) {
                targetColor.setHsv(h, s, qBound(40, 255 - v, 240));
            }
            else if (type == LC_SVGIconEngineAPI::Accent) {
                if (toDark) {
                    targetColor.setHsv(h, qBound(0, static_cast<int>(s * 0.70), 255), qBound(0, v + 50, 255));
                }
                else {
                    targetColor.setHsv(h, qBound(0, qMin(255, static_cast<int>(s * 1.35)), 255), qBound(0, qMax(0, v - 40), 255));
                }
            }
            tempOptions.setColor(mode, state, type, targetColor.name(QColor::HexArgb));
        };

        convertColorType(LC_SVGIconEngineAPI::Main);
        convertColorType(LC_SVGIconEngineAPI::Accent);
        convertColorType(LC_SVGIconEngineAPI::Background);
    };

    runBidiConversion(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, targetDark);
    for (const auto& row : ICON_STATE_ROWS) {
        runBidiConversion(row.mode, row.state, targetDark);
    }

    tempOptions.exportStyleConfig(m_presetManager->workingConfig(), targetDark);

    const QString targetName = targetDark ? tr("Dark Mode") : tr("Light Mode");
    QMessageBox::information(m_widget, tr("Auto-Generation Complete"),
                             tr("Opposite scheme successfully generated and stored in %1 configuration. Switch mode tabs to view or edit.").arg(targetName));

    m_presetManager->notifyWorkingConfigChanged();
}

void LC_SettingsPageIconsStyle::populateUiFromWorkingConfig() {
    if (m_presetManager == nullptr || getEditingWidget() == nullptr) {
        return;
    }

    m_blockSignals = true;
    const auto& options = m_presetManager->iconColorsOptions();

    loadSeedControl(options.getColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Main),
                    ui->chkMainPalette, ui->btnMainColor, ui->cbMainPaletteRole);
    loadSeedControl(options.getColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Accent),
                    ui->chkAccentPalette, ui->btnAccentColor, ui->cbAccentPaletteRole);
    loadSeedControl(options.getColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::Accent),
                    ui->chkAccentAltPalette, ui->btnAccentAltColor, ui->cbAccentAltPaletteRole);
    loadSeedControl(options.getColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Background),
                    ui->chkBackPalette, ui->btnBackColor, ui->cbBackPaletteRole);

    ui->chkAutoCalcStates->setChecked(options.isAutoCalculateStates());

    m_blockSignals = false;

    if (options.isAutoCalculateStates()) {
        calculateAndApplyAutoStates();
    }
    else {
        ui->tableStates->blockSignals(true);
        for (int row = 0; row < ui->tableStates->rowCount(); ++row) {
            const auto mode = ICON_STATE_ROWS[row].mode;
            const auto state = ICON_STATE_ROWS[row].state;

            auto* btnMain = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 1));
            auto* btnAccent = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 2));
            auto* btnBack = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 3));

            if (btnMain) {
                QString colorStr = options.getColor(mode, state, LC_SVGIconEngineAPI::Main);
                btnMain->setColor(colorStr.isEmpty() ? QColor() : QColor(colorStr));
                btnMain->setLocked(false);
            }
            if (btnAccent) {
                QString colorStr = options.getColor(mode, state, LC_SVGIconEngineAPI::Accent);
                btnAccent->setColor(colorStr.isEmpty() ? QColor() : QColor(colorStr));
                btnAccent->setLocked(false);
            }
            if (btnBack) {
                QString colorStr = options.getColor(mode, state, LC_SVGIconEngineAPI::Background);
                btnBack->setColor(colorStr.isEmpty() ? QColor() : QColor(colorStr));
                btnBack->setLocked(false);
            }
        }
        ui->tableStates->blockSignals(false);
    }
}

void LC_SettingsPageIconsStyle::syncUiToWorkingConfig() {
    if (m_presetManager == nullptr) {
        return;
    }

    auto& options = m_presetManager->iconColorsOptions();
    options.setColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Main,
                     getSeedValueFromUi(ui->chkMainPalette, ui->btnMainColor, ui->cbMainPaletteRole));
    options.setColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Accent,
                     getSeedValueFromUi(ui->chkAccentPalette, ui->btnAccentColor, ui->cbAccentPaletteRole));
    options.setColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::On, LC_SVGIconEngineAPI::Accent,
                     getSeedValueFromUi(ui->chkAccentAltPalette, ui->btnAccentAltColor, ui->cbAccentAltPaletteRole));
    options.setColor(LC_SVGIconEngineAPI::AnyMode, LC_SVGIconEngineAPI::AnyState, LC_SVGIconEngineAPI::Background,
                     getSeedValueFromUi(ui->chkBackPalette, ui->btnBackColor, ui->cbBackPaletteRole));

    options.setAutoCalculateStates(ui->chkAutoCalcStates->isChecked());

    for (int row = 0; row < ui->tableStates->rowCount(); ++row) {
        const auto mode = ICON_STATE_ROWS[row].mode;
        const auto state = ICON_STATE_ROWS[row].state;

        const auto* btnMain = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 1));
        const auto* btnAccent = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 2));
        const auto* btnBack = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 3));

        if (btnMain) {
            options.setColor(mode, state, LC_SVGIconEngineAPI::Main,
                             (btnMain->color().isValid() && btnMain->color().alpha() > 0) ? btnMain->color().name(QColor::HexRgb) : "");
        }
        if (btnAccent) {
            options.setColor(mode, state, LC_SVGIconEngineAPI::Accent,
                             (btnAccent->color().isValid() && btnAccent->color().alpha() > 0) ? btnAccent->color().name(QColor::HexRgb) : "");
        }
        if (btnBack) {
            options.setColor(mode, state, LC_SVGIconEngineAPI::Background,
                             (btnBack->color().isValid() && btnBack->color().alpha() > 0) ? btnBack->color().name(QColor::HexRgb) : "");
        }
    }
}

void LC_SettingsPageIconsStyle::calculateAndApplyAutoStates() {
    m_blockSignals = true;

    const bool isDark = ui->rbDarkMode->isChecked();
    QColor mainSeed = QColor(LC_IconsStyleManager::resolveColorValue(getSeedValueFromUi(ui->chkMainPalette, ui->btnMainColor, ui->cbMainPaletteRole)));
    QColor accentSeed = QColor(LC_IconsStyleManager::resolveColorValue(getSeedValueFromUi(ui->chkAccentPalette, ui->btnAccentColor, ui->cbAccentPaletteRole)));
    QColor accentAltSeed = QColor(LC_IconsStyleManager::resolveColorValue(getSeedValueFromUi(ui->chkAccentAltPalette, ui->btnAccentAltColor, ui->cbAccentAltPaletteRole)));
    QColor backSeed = QColor(LC_IconsStyleManager::resolveColorValue(getSeedValueFromUi(ui->chkBackPalette, ui->btnBackColor, ui->cbBackPaletteRole)));

    if (!mainSeed.isValid()) {
        mainSeed = isDark ? QColor("#e6e6e6") : QColor("#000000");
    }
    if (!accentSeed.isValid()) {
        accentSeed = QColor("#00ff7f");
    }
    if (!accentAltSeed.isValid()) {
        accentAltSeed = accentSeed;
    }

    const QPalette activePalette = QGuiApplication::palette();
    const QColor highlightColor = activePalette.color(QPalette::Highlight);
    const QColor highlightedTextColor = activePalette.color(QPalette::HighlightedText);
    const QColor windowColor = activePalette.color(QPalette::Window);

    for (int row = 0; row < ui->tableStates->rowCount(); ++row) {
        const auto mode = ICON_STATE_ROWS[row].mode;
        const auto state = ICON_STATE_ROWS[row].state;

        auto* btnMain = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 1));
        auto* btnAccent = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 2));
        auto* btnBack = qobject_cast<LC_ColorButton*>(ui->tableStates->cellWidget(row, 3));

        if (btnMain) {
            btnMain->setColor(LC_IconsColorUtils::calculateStateColor(mode, state, LC_SVGIconEngineAPI::Main, mainSeed, accentSeed, accentAltSeed, backSeed, highlightColor, highlightedTextColor, windowColor));
            btnMain->setLocked(true);
        }
        if (btnAccent) {
            btnAccent->setColor(LC_IconsColorUtils::calculateStateColor(mode, state, LC_SVGIconEngineAPI::Accent, mainSeed, accentSeed, accentAltSeed, backSeed, highlightColor, highlightedTextColor, windowColor));
            btnAccent->setLocked(true);
        }
        if (btnBack) {
            btnBack->setColor(LC_IconsColorUtils::calculateStateColor(mode, state, LC_SVGIconEngineAPI::Background, mainSeed, accentSeed, accentAltSeed, backSeed, highlightColor, highlightedTextColor, windowColor));
            btnBack->setLocked(true);
        }
    }

    m_blockSignals = false;
}

void LC_SettingsPageIconsStyle::loadSeedControl(const QString& colorStr, QCheckBox* chk, LC_ColorButton* btn, QComboBox* cb) {
    chk->blockSignals(true);
    btn->blockSignals(true);
    cb->blockSignals(true);

    if (colorStr.startsWith("palette:") || colorStr == "transparent") {
        chk->setChecked(true);
        btn->setVisible(false);
        cb->setVisible(true);

        const QString roleKey = colorStr.startsWith("palette:") ? colorStr.mid(8) : colorStr;
        const int idx = cb->findData(roleKey);
        if (idx >= 0) {
            cb->setCurrentIndex(idx);
        }
    }
    else {
        chk->setChecked(false);
        btn->setVisible(true);
        cb->setVisible(false);
        btn->setColor(colorStr.isEmpty() ? Qt::white : QColor(colorStr));
    }

    chk->blockSignals(false);
    btn->blockSignals(false);
    cb->blockSignals(false);
}

QString LC_SettingsPageIconsStyle::getSeedValueFromUi(const QCheckBox* chk, const LC_ColorButton* btn, const QComboBox* cb) const {
    if (chk->isChecked()) {
        const QString dataVal = cb->currentData().toString();
        if (dataVal == "transparent") {
            return "transparent";
        }
        return "palette:" + dataVal;
    }
    return (btn->color().isValid() && btn->color().alpha() > 0) ? btn->color().name(QColor::HexRgb) : "transparent";
}
