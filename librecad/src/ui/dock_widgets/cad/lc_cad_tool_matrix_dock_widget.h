
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

#ifndef LC_CADToolMatrixDockWidget_H
#define LC_CADToolMatrixDockWidget_H
#include "lc_caddockwidget.h"

class LC_CADToolMatrixDockWidget : public LC_CADDockWidget {
    Q_OBJECT
public:
    // Strictly enforces the "allTools" layout configuration for the CAD matrix
    explicit LC_CADToolMatrixDockWidget(QWidget* parent, bool scrollContent = true);
    ~LC_CADToolMatrixDockWidget() override = default;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void changeEvent(QEvent* event) override;
protected:

    void onBeforeAddActions() override;
    bool shouldCreateButtonForAction(QAction* action) const override;
    void handleIgnoredAction(QAction* action) override;
    void configureButton(QToolButton* toolButton, QAction* action) override;
    void onLayoutUpdated() override;
    void doSetupGridLayout(QGridLayout* newGridLayout) override;

private:
    void updateSegmentedButtonsMask() const;
    int m_currentGroupId{1};
};


#endif
