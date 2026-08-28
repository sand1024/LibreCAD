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

#ifndef LC_STYLING_PREVIEW_BOTTOM_BAR_H
#define LC_STYLING_PREVIEW_BOTTOM_BAR_H

#include <QWidget>
#include <memory>
#include "lc_palette_color_utils.h"

namespace Ui {
    class LC_StylingPreviewBottomBar;
}

class LC_StylingPreviewBottomBar : public QWidget {
    Q_OBJECT
public:
    explicit LC_StylingPreviewBottomBar(QWidget* parent = nullptr);
    ~LC_StylingPreviewBottomBar() override;

    void setCvdVisible(bool visible) const;
    void setPreviewControlsVisible(bool visible) const;
    void setPreviewChecked(bool checked);

    bool isPreviewChecked() const;
    bool isDisabledStateChecked() const;
    LC_PaletteColorUtils::CVDType activeCvdType() const;
signals:
    void previewToggled(bool checked);
    void disabledStateToggled(bool checked);
    void cvdChanged(LC_PaletteColorUtils::CVDType cvd);

private:
    std::unique_ptr<Ui::LC_StylingPreviewBottomBar> ui;
    bool m_blockSignals = false;
};

#endif
