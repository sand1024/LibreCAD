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
#include "lc_viewport_preview_bottom_bar.h"

#include "lc_setting.h"
#include "ui_lc_viewport_preview_bottom_bar.h"

namespace {
    const LC_SettingsGroupBase CFG_ViewportPreviewGroup("ViewportPreview");
    const LC_Setting<bool> o_ShowViewportPreview(&CFG_ViewportPreviewGroup, "ShowViewportPreview", true);
}

LC_ViewportPreviewBottomBar::LC_ViewportPreviewBottomBar(QWidget* parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::LC_ViewportPreviewBottomBar>()) {
    ui->setupUi(this);
    ui->cbShowPreview->setChecked(o_ShowViewportPreview.get());

    connect(ui->cbShowPreview, &QCheckBox::toggled, this, [this](bool checked) {
        o_ShowViewportPreview.set(checked);
        emit previewToggled(checked);
        if (m_visibilityCallback != nullptr) {
            m_visibilityCallback(checked);
        }
    });
}

LC_ViewportPreviewBottomBar::~LC_ViewportPreviewBottomBar() = default;

bool LC_ViewportPreviewBottomBar::isPreviewVisible() const {
    if (ui != nullptr && ui->cbShowPreview != nullptr) {
        return ui->cbShowPreview->isChecked();
    }
    return o_ShowViewportPreview.get();
}

void LC_ViewportPreviewBottomBar::setOnPreviewVisibilityChanged(std::function<void(bool)> callback) {
    m_visibilityCallback = std::move(callback);
}

void LC_ViewportPreviewBottomBar::setPreviewChecked(bool checked) {
    if (ui != nullptr && ui->cbShowPreview != nullptr) {
        ui->cbShowPreview->setChecked(checked);
    }
    o_ShowViewportPreview.set(checked);
}
