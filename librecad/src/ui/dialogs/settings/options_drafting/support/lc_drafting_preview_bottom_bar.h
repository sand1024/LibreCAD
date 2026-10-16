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

#ifndef LC_VIEWPORT_PREVIEW_BOTTOM_BAR_H
#define LC_VIEWPORT_PREVIEW_BOTTOM_BAR_H

#include <QWidget>

#include "lc_preview_visibility_controller.h"

class QCheckBox;
class QHBoxLayout;

namespace Ui {
    class LC_DraftingPreviewBottomBar;
}

class LC_DraftingPreviewBottomBar : public QWidget, public LC_PreviewVisibilityController {
    Q_OBJECT
public:
    explicit LC_DraftingPreviewBottomBar(QWidget* parent = nullptr);
    ~LC_DraftingPreviewBottomBar() override;

    bool isPreviewVisible() const override;
    void setOnPreviewVisibilityChanged(std::function<void(bool)> callback) override;

    void setPreviewChecked(bool checked) const;

    signals:
        void previewToggled(bool checked);

private:
    std::unique_ptr<Ui::LC_DraftingPreviewBottomBar> ui;
    std::function<void(bool)> m_visibilityCallback;
};

#endif
