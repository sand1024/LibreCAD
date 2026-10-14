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
#include "lc_drafting_preview_bottom_bar.h"

#include "lc_setting.h"
#include "ui_lc_drafting_preview_bottom_bar.h"

namespace {
    const LC_SettingsGroupBase CFG_DraftingPreviewGroup("DraftingPreview");
    const LC_Setting<bool> o_ShowDraftingPreview(&CFG_DraftingPreviewGroup, "ShowDraftingPreview", true);
}

LC_DraftingPreviewBottomBar::LC_DraftingPreviewBottomBar(QWidget* parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::LC_DraftingPreviewBottomBar>()) {
    ui->setupUi(this);
    ui->cbShowPreview->setChecked(o_ShowDraftingPreview.get());

    connect(ui->cbShowPreview, &QCheckBox::toggled, this, [this](bool checked) {
        o_ShowDraftingPreview.set(checked);
        emit previewToggled(checked);
        if (m_visibilityCallback != nullptr) {
            m_visibilityCallback(checked);
        }
    });
}

LC_DraftingPreviewBottomBar::~LC_DraftingPreviewBottomBar() = default;

bool LC_DraftingPreviewBottomBar::isPreviewVisible() const {
    if (ui != nullptr && ui->cbShowPreview != nullptr) {
        return ui->cbShowPreview->isChecked();
    }
    return o_ShowDraftingPreview.get();
}

void LC_DraftingPreviewBottomBar::setOnPreviewVisibilityChanged(std::function<void(bool)> callback) {
    m_visibilityCallback = std::move(callback);
}

void LC_DraftingPreviewBottomBar::setPreviewChecked(bool checked) {
    if (ui != nullptr && ui->cbShowPreview != nullptr) {
        ui->cbShowPreview->setChecked(checked);
    }
    o_ShowDraftingPreview.set(checked);
}
