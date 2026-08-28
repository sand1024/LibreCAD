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

#include "lc_styling_preview_bottom_bar.h"
#include "ui_lc_styling_preview_bottom_bar.h"

LC_StylingPreviewBottomBar::LC_StylingPreviewBottomBar(QWidget* parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::LC_StylingPreviewBottomBar>()) {
    ui->setupUi(this);

    ui->cbCvdSimulator->addItem(tr("Normal Vision"), static_cast<int>(LC_PaletteColorUtils::CVDType::Normal));
    ui->cbCvdSimulator->addItem(tr("Protanopia (Red-Blind)"), static_cast<int>(LC_PaletteColorUtils::CVDType::Protanopia));
    ui->cbCvdSimulator->addItem(tr("Deuteranopia (Green-Blind)"), static_cast<int>(LC_PaletteColorUtils::CVDType::Deuteranopia));
    ui->cbCvdSimulator->addItem(tr("Tritanopia (Blue-Blind)"), static_cast<int>(LC_PaletteColorUtils::CVDType::Tritanopia));

    connect(ui->chkShowPreview, &QCheckBox::toggled, this, [this](bool checked) {
        if (!m_blockSignals) {
            emit previewToggled(checked);
        }
        ui->wEmulateDisabledState->setVisible(checked);
    });

    connect(ui->chkDisablePreview, &QCheckBox::toggled, this, [this](bool checked) {
        emit disabledStateToggled(checked);
    });

    connect(ui->cbCvdSimulator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
        emit cvdChanged(activeCvdType());
    });
}

LC_StylingPreviewBottomBar::~LC_StylingPreviewBottomBar() = default;

void LC_StylingPreviewBottomBar::setPreviewControlsVisible(bool visible) const {
    ui->chkShowPreview->setVisible(visible);
    ui->wEmulateDisabledState->setVisible(visible);
}

void LC_StylingPreviewBottomBar::setCvdVisible(bool visible) const {
    ui->lblCvd->setVisible(visible);
    ui->cbCvdSimulator->setVisible(visible);
}

void LC_StylingPreviewBottomBar::setPreviewChecked(bool checked) {
    m_blockSignals = true;
    ui->chkShowPreview->setChecked(checked);
    m_blockSignals = false;
}

bool LC_StylingPreviewBottomBar::isPreviewChecked() const {
    return ui->chkShowPreview->isChecked();
}

bool LC_StylingPreviewBottomBar::isDisabledStateChecked() const {
    return ui->chkDisablePreview->isChecked();
}

LC_PaletteColorUtils::CVDType LC_StylingPreviewBottomBar::activeCvdType() const {
    int idx = ui->cbCvdSimulator->currentIndex();
    if (idx >= 0) {
        return static_cast<LC_PaletteColorUtils::CVDType>(ui->cbCvdSimulator->itemData(idx).toInt());
    }
    return LC_PaletteColorUtils::CVDType::Normal;
}
