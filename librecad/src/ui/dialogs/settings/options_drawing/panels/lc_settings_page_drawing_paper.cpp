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

#include "lc_settings_page_drawing_paper.h"
#include "ui_lc_settings_page_drawing_paper.h"
#include "lc_document_settings_backend.h"
#include "rs_units.h"
#include "rs_math.h"
#include "lc_plot_settings.h"

LC_SettingsPageDrawingPaper::LC_SettingsPageDrawingPaper(RS_Graphic* graphic, QObject* parent)
    : LC_SettingsPageBase(tr("Paper"), std::make_unique<LC_DocumentSettingsBackend>(graphic), parent)
    , m_graphic(graphic)
    , m_paperScene(new QGraphicsScene(this)) {
}

LC_SettingsPageDrawingPaper::~LC_SettingsPageDrawingPaper() = default;

void LC_SettingsPageDrawingPaper::setupUi() {
    ui = std::make_unique<Ui::LC_SettingsPageDrawingPaper>();
    ui->setupUi(m_widget);

    ui->gvPaperPreview->setScene(m_paperScene);
    ui->gvPaperPreview->setBackgroundBrush(m_widget->palette().color(QPalette::Window));

    for (int i = RS2::FirstPaperFormat; i < RS2::NPageFormat; ++i) {
        ui->cbPaperFormat->addItem(RS_Units::paperFormatToString(static_cast<RS2::PaperFormat>(i)));
    }

    ui->splitter->setSizes({60,40});
}

void LC_SettingsPageDrawingPaper::setupBehavior() {
    connect(ui->cbPaperFormat, QOverload<int>::of(&QComboBox::activated), this, &LC_SettingsPageDrawingPaper::onFormatChanged);
    connect(ui->rbLandscape, &QRadioButton::toggled, this, &LC_SettingsPageDrawingPaper::onOrientationChanged);
    connect(ui->rbPortrait, &QRadioButton::toggled, this, &LC_SettingsPageDrawingPaper::onOrientationChanged);

    auto redraw = [this](const QString&) { updatePreview(); };
    connect(ui->lePaperWidth, &QLineEdit::textChanged, redraw);
    connect(ui->lePaperHeight, &QLineEdit::textChanged, redraw);
    connect(ui->leMarginLeft, &QLineEdit::textChanged, redraw);
    connect(ui->leMarginRight, &QLineEdit::textChanged, redraw);
    connect(ui->leMarginTop, &QLineEdit::textChanged, redraw);
    connect(ui->leMarginBottom, &QLineEdit::textChanged, redraw);
}

void LC_SettingsPageDrawingPaper::loadSettings() {
    if (!m_graphic) return;
    const LC_PlotSettings* ps = m_graphic->getPlotSettings();

    bool landscape = false;
    RS2::PaperFormat format = ps->getPaperFormat(&landscape);

    ui->cbPaperFormat->setCurrentIndex(format);
    if (landscape) ui->rbLandscape->setChecked(true);
    else ui->rbPortrait->setChecked(true);

    RS_Vector size = ps->getPaperSize();
    ui->lePaperWidth->setText(QString::number(size.x, 'g', 5));
    ui->lePaperHeight->setText(QString::number(size.y, 'g', 5));

    ui->leMarginLeft->setText(QString::number(ps->getMarginLeftInUnits()));
    ui->leMarginTop->setText(QString::number(ps->getMarginTopInUnits()));
    ui->leMarginRight->setText(QString::number(ps->getMarginRightInUnits()));
    ui->leMarginBottom->setText(QString::number(ps->getMarginBottomInUnits()));

    ui->sbPagesNumH->setValue(ps->getPagesNumHoriz());
    ui->sbPagesNumV->setValue(ps->getPagesNumVert());

    onFormatChanged(format);
}

bool LC_SettingsPageDrawingPaper::saveSettings() {
    if (!m_graphic) return false;
    LC_PlotSettings* ps = m_graphic->getPlotSettings();

    ps->setPaperFormat(static_cast<RS2::PaperFormat>(ui->cbPaperFormat->currentIndex()),
                       ui->rbLandscape->isChecked());

    if (ui->cbPaperFormat->currentIndex() == RS2::Custom) {
        ps->setPaperSize(RS_Vector(RS_Math::eval(ui->lePaperWidth->text()),
                                   RS_Math::eval(ui->lePaperHeight->text())));
    }

    ps->setMarginsInUnits(RS_Math::eval(ui->leMarginLeft->text()),
                          RS_Math::eval(ui->leMarginTop->text()),
                          RS_Math::eval(ui->leMarginRight->text()),
                          RS_Math::eval(ui->leMarginBottom->text()));

    ps->setPagesNum(ui->sbPagesNumH->value(), ui->sbPagesNumV->value());
    return true;
}

bool LC_SettingsPageDrawingPaper::isModified() const {
    if (!m_graphic) return false;
    const LC_PlotSettings* ps = m_graphic->getPlotSettings();

    // 1. Compare Paper Format & Landscape Orientation
    bool docLandscape = false;
    RS2::PaperFormat docFormat = ps->getPaperFormat(&docLandscape);

    if (static_cast<RS2::PaperFormat>(ui->cbPaperFormat->currentIndex()) != docFormat) {
        return true;
    }
    if (ui->rbLandscape->isChecked() != docLandscape) {
        return true;
    }

    // 2. Compare Custom Dimensions (only if custom is currently evaluated)
    if (docFormat == RS2::Custom) {
        double uiW = RS_Math::eval(ui->lePaperWidth->text());
        double uiH = RS_Math::eval(ui->lePaperHeight->text());
        RS_Vector docSize = ps->getPaperSize();
        if (std::abs(uiW - docSize.x) > 1e-9 || std::abs(uiH - docSize.y) > 1e-9) {
            return true;
        }
    }

    // 3. Compare Margins with floating-point tolerance
    double uiLeft = RS_Math::eval(ui->leMarginLeft->text());
    double uiTop = RS_Math::eval(ui->leMarginTop->text());
    double uiRight = RS_Math::eval(ui->leMarginRight->text());
    double uiBottom = RS_Math::eval(ui->leMarginBottom->text());

    if (std::abs(uiLeft - ps->getMarginLeftInUnits()) > 1e-9 ||
        std::abs(uiTop - ps->getMarginTopInUnits()) > 1e-9 ||
        std::abs(uiRight - ps->getMarginRightInUnits()) > 1e-9 ||
        std::abs(uiBottom - ps->getMarginBottomInUnits()) > 1e-9) {
        return true;
    }

    // 4. Compare Number of Pages
    if (ui->sbPagesNumH->value() != ps->getPagesNumHoriz() ||
        ui->sbPagesNumV->value() != ps->getPagesNumVert()) {
        return true;
    }

    return false;
}

namespace CFG_PagePaperSetup
{
    inline const LC_SettingsGroupBase Group("SettingsPage.PaperSetup");
    inline const LC_Setting<double> o_ContentRatio(&Group, "ContentRatio", 0.6);
}

void LC_SettingsPageDrawingPaper::onDialogShown() {
    m_pageWasVisible = true;
    onAboutToShow();
}

void LC_SettingsPageDrawingPaper::onAboutToShow() {
    if (isSettingsDialogVisible()) {
        m_widget->adjustSize();
        const int splitterWidth = ui->splitter->width();
        const double ratio = CFG_PagePaperSetup::o_ContentRatio;
        int first = splitterWidth * ratio;
        int second = splitterWidth - first;
        ui->splitter->setSizes({first, second});
        updatePreview();
    }
}

void LC_SettingsPageDrawingPaper::onAboutToHide() {
    if (m_pageWasVisible) {
        QList<int> currentMainSplitterSizes = ui->splitter->sizes();
        if (!currentMainSplitterSizes.isEmpty()) {
            const int splitterWidth = ui->splitter->width();
            const double width = currentMainSplitterSizes[0];
            const double ratio = width / splitterWidth;
            CFG_PagePaperSetup::o_ContentRatio = ratio;
        }
    }
}

void LC_SettingsPageDrawingPaper::onFormatChanged(int index) {
    const bool isCustom = (index == RS2::Custom);
    ui->lePaperWidth->setEnabled(isCustom);
    ui->lePaperHeight->setEnabled(isCustom);

    if (!isCustom) {
        const RS2::Unit unit = m_graphic->getUnit();
        RS_Vector s = RS_Units::convert(RS_Units::paperFormatToSize(static_cast<RS2::PaperFormat>(index)),
                                        RS2::Millimeter, unit);
        bool currentLandscape = ui->rbLandscape->isChecked();
        if (currentLandscape != (s.x > s.y)) {
            std::swap(s.x, s.y);
        }

        ui->lePaperWidth->setText(QString::number(s.x, 'g', 5));
        ui->lePaperHeight->setText(QString::number(s.y, 'g', 5));
    }
    doUpdatePreview(isCustom);
}

void LC_SettingsPageDrawingPaper::updatePreview() {
    doUpdatePreview(true);
}

void LC_SettingsPageDrawingPaper::doUpdatePreview(bool calcOrientation) {

    const auto paperWidthText = ui->lePaperWidth->text();
    const auto paperHeightText = ui->lePaperHeight->text();

    const double paperW = RS_Math::eval(paperWidthText);
    const double paperH = RS_Math::eval(paperHeightText);

    if (calcOrientation) {
        ui->rbLandscape->blockSignals(true);
        ui->rbPortrait->blockSignals(true);
        if (paperW > paperH) {
            ui->rbLandscape->setChecked(true);
        }
        else {
            ui->rbPortrait->setChecked(true);
        }
        ui->rbLandscape->blockSignals(false);
        ui->rbPortrait->blockSignals(false);
    }
    else {
        ui->lePaperHeight->blockSignals(true);
        ui->lePaperWidth->blockSignals(true);
        if (paperW > paperH) {
             if (ui->rbPortrait->isChecked()) {
                 QString tmp = ui->lePaperHeight->text();
                 ui->lePaperHeight->setText(ui->lePaperWidth->text());
                 ui->lePaperWidth->setText(tmp);
             }
        }
        else {
             if (ui->rbLandscape->isChecked()) {
                 QString tmp = ui->lePaperHeight->text();
                 ui->lePaperHeight->setText(ui->lePaperWidth->text());
                 ui->lePaperWidth->setText(tmp);
             }
        }
        ui->lePaperHeight->blockSignals(false);
        ui->lePaperWidth->blockSignals(false);
    }

    /* Margins of preview are 5 px */
    const int previewW = ui->gvPaperPreview->width() - 10;
    const int previewH = ui->gvPaperPreview->height() - 10;
    const double scale = qMin(previewW / paperW, previewH / paperH);
    int lMargin = qRound(RS_Math::eval(ui->leMarginLeft->text(), -1) * scale);
    const LC_PlotSettings* ps = m_graphic->getPlotSettings();
    if (lMargin < 0.0) {
        lMargin = ps->getMarginLeftInUnits();
    }
    int tMargin = qRound(RS_Math::eval(ui->leMarginTop->text(), -1) * scale);
    if (tMargin < 0.0) {
        tMargin = ps->getMarginTopInUnits();
    }
    int rMargin = qRound(RS_Math::eval(ui->leMarginRight->text(), -1) * scale);
    if (rMargin < 0.0) {
        rMargin = ps->getMarginRightInUnits();
    }
    int bMargin = qRound(RS_Math::eval(ui->leMarginBottom->text(), -1) * scale);
    if (bMargin < 0.0) {
        bMargin = ps->getMarginBottomInUnits();
    }
    const int printAreaW = qRound(paperW * scale) - lMargin - rMargin;
    const int printAreaH = qRound(paperH * scale) - tMargin - bMargin;
    m_paperScene->clear();
    m_paperScene->setSceneRect(0, 0, qRound(paperW * scale), qRound(paperH * scale));
    m_paperScene->addRect(0, 0, qRound(paperW * scale), qRound(paperH * scale), QPen(Qt::black), QBrush(Qt::lightGray));
    m_paperScene->addRect(lMargin + 1, tMargin + 1, printAreaW - 1, printAreaH - 1, QPen(Qt::NoPen), QBrush(Qt::white));

}

void LC_SettingsPageDrawingPaper::onOrientationChanged() {
    bool ok;
    const double w = RS_Math::eval(ui->lePaperWidth->text(), &ok);
    const double h = RS_Math::eval(ui->lePaperHeight->text(), &ok);

    if (ui->rbLandscape->isChecked()) {
        if (w < h) {
            ui->lePaperWidth->setText(QString::number(h, 'g', 5));
            ui->lePaperHeight->setText(QString::number(w, 'g', 5));
        }
    } else {
        if (h < w) {
            ui->lePaperWidth->setText(QString::number(h, 'g', 5));
            ui->lePaperHeight->setText(QString::number(w, 'g', 5));
        }
    }
    updatePreview();
}
