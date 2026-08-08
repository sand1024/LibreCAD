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

/*******************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2026 LibreCAD.org
 ******************************************************************************/

#include "lc_settings_page_drawing_variables.h"
#include "ui_lc_settings_page_drawing_variables.h"
#include "lc_document_settings_backend.h"
#include "rs_graphic.h"
#include "rs_filterdxfrw.h"

LC_SettingsPageDrawingVariables::LC_SettingsPageDrawingVariables(RS_Graphic* graphic, QObject* parent)
    : LC_SettingsPageBase(tr("Variables"), std::make_unique<LC_DocumentSettingsBackend>(graphic), parent)
    , m_graphic(graphic) {
    setSortWeight(100); // Usually the last page
}

LC_SettingsPageDrawingVariables::~LC_SettingsPageDrawingVariables() = default;

void LC_SettingsPageDrawingVariables::setupUi() {
    ui = std::make_unique<Ui::LC_SettingsPageDrawingVariables>();
    ui->setupUi(m_widget);

    QStringList headers = { tr("Name"), tr("Code"), tr("Type"), tr("Value") };
    ui->tabVariables->setColumnCount(headers.size());
    ui->tabVariables->setHorizontalHeaderLabels(headers);

    ui->tabVariables->horizontalHeader()->setStretchLastSection(true);
    ui->tabVariables->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tabVariables->setEditTriggers(QAbstractItemView::NoEditTriggers);
}


void LC_SettingsPageDrawingVariables::loadSettings() {
    if (!m_graphic) return;

    ui->tabVariables->setSortingEnabled(false); // Disable while loading
    ui->tabVariables->setRowCount(0);

    QHash<QString, RS_Variable> vars = m_graphic->getVariableDict();
    int row = 0;

    for (auto it = vars.begin(); it != vars.end(); ++it) {
        ui->tabVariables->insertRow(row);

        QString name = it.key();
        if (name.startsWith("$")) name = name.mid(1);

        // 1. Name
        ui->tabVariables->setItem(row, 0, new QTableWidgetItem(name));

        // 2. DXF Group Code
        ui->tabVariables->setItem(row, 1, new QTableWidgetItem(QString::number(it.value().getCode())));

        // 3. Type & Value
        QString typeStr;
        QString valueStr;

        switch (it.value().getType()) {
            case RS2::VariableInt:
                typeStr = tr("INT");
                valueStr = QString::number(it.value().getInt());
                break;
            case RS2::VariableDouble:
                typeStr = tr("DOUBLE");
                valueStr = QString::number(it.value().getDouble());
                break;
            case RS2::VariableString:
                typeStr = tr("STRING");
                valueStr = it.value().getString();
                break;
            case RS2::VariableVector: {
                typeStr = tr("VECTOR");
                RS_Vector v = it.value().getVector();
                if (RS_FilterDXFRW::isVariableTwoDimensional(it.key()))
                    valueStr = QString("%1 / %2").arg(v.x).arg(v.y);
                else
                    valueStr = QString("%1 / %2 / %3").arg(v.x).arg(v.y).arg(v.z);
                break;
            }
            default:
                typeStr = tr("VOID");
                break;
        }

        ui->tabVariables->setItem(row, 2, new QTableWidgetItem(typeStr));
        ui->tabVariables->setItem(row, 3, new QTableWidgetItem(valueStr));
        row++;
    }

    ui->tabVariables->setSortingEnabled(true);
    ui->tabVariables->sortByColumn(0, Qt::AscendingOrder);
}
