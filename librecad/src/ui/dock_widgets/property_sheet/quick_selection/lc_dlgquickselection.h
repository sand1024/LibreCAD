/*
 * ********************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2025 LibreCAD.org
 * Copyright (C) 2025 sand1024
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
 * ********************************************************************************
 */

#ifndef LC_DLGQUICKSELECTION_H
#define LC_DLGQUICKSELECTION_H

#include <qcombobox.h>

#include "lc_dialog.h"
#include "lc_graphicviewport.h"
#include "rs_entitycontainer.h"
#include "rs_selection.h"

namespace Ui {
    class LC_DlgQuickSelection;
}

class LC_DlgQuickSelection : public LC_Dialog {
    Q_OBJECT
public:
    LC_DlgQuickSelection(QWidget *parent, RS_Document* doc, LC_GraphicViewport* viewport);
    ~LC_DlgQuickSelection() override;
protected slots:
    void setupEntitiesTypesList(const QSet<RS2::EntityType>& set) const;
    void onApplyToCurrentIndexChanged(int index) const;
    void onEntityTypeIndexChanged(int index);
    void onPropertyListRowChanged(int currentRow);
    void onManualSelectionClicked(bool checked = false);
private:
    Ui::LC_DlgQuickSelection *ui;
    RS_Document *m_document = nullptr;
    LC_GraphicViewport* m_viewport = nullptr;
    RS_Selection::CurrentSelectionState m_selectionState;
};

#endif // LC_DLGQUICKSELECTION_H
