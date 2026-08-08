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

#include "lc_settings_page_drawing_user_data.h"
#include "ui_lc_settings_page_drawing_user_data.h"
#include "lc_document_settings_backend.h"
#include "lc_dlgnewcustomvariable.h"
#include "rs_math.h"
#include <QMessageBox>

LC_SettingsPageDrawingUserData::LC_SettingsPageDrawingUserData(RS_Graphic* graphic, QObject* parent)
    : LC_SettingsPageBase(tr("User Data"), nullptr, parent) // FIX: Pass nullptr safely
    , m_graphic(graphic) {
}

LC_SettingsPageDrawingUserData::~LC_SettingsPageDrawingUserData() = default;

void LC_SettingsPageDrawingUserData::setupUi() {
    ui = std::make_unique<Ui::LC_SettingsPageDrawingUserData>();
    ui->setupUi(m_widget);

    ui->twCustomVars->setColumnCount(2);
    ui->twCustomVars->setHorizontalHeaderLabels({tr("Name"), tr("Value")});
    ui->twCustomVars->horizontalHeader()->setStretchLastSection(true);
    ui->twCustomVars->verticalHeader()->hide();
}

void LC_SettingsPageDrawingUserData::setupBehavior() {
    connect(ui->pbCustomVarAdd, &QPushButton::clicked, this, &LC_SettingsPageDrawingUserData::onAddCustomProperty);
    connect(ui->pbCustomVarDelete, &QPushButton::clicked, this, &LC_SettingsPageDrawingUserData::onDeleteCustomProperty);

    connect(ui->twCustomVars, &QTableWidget::itemSelectionChanged, this, [this](){
        ui->pbCustomVarDelete->setEnabled(ui->twCustomVars->currentRow() >= 0);
    });
}

void LC_SettingsPageDrawingUserData::loadSettings() {
    if (!m_graphic) return;

    // 1. Load Fixed Integer Variables $USERI1 - $USERI5
    ui->sbUserI1->setValue(m_graphic->getVariableInt("$USERI1", 0));
    ui->sbUserI2->setValue(m_graphic->getVariableInt("$USERI2", 0));
    ui->sbUserI3->setValue(m_graphic->getVariableInt("$USERI3", 0));
    ui->sbUserI4->setValue(m_graphic->getVariableInt("$USERI4", 0));
    ui->sbUserI5->setValue(m_graphic->getVariableInt("$USERI5", 0));

    // 2. Load Fixed Real Variables $USERR1 - $USERR5
    auto setReal = [this](QLineEdit* le, const QString& key) {
        double v = m_graphic->getVariableDouble(key, 0.0);
        le->setText(QString::number(v, 'g', 12));
    };
    setReal(ui->leUserR1, "$USERR1");
    setReal(ui->leUserR2, "$USERR2");
    setReal(ui->leUserR3, "$USERR3");
    setReal(ui->leUserR4, "$USERR4");
    setReal(ui->leUserR5, "$USERR5");

    // 3. Load Custom Variables Table manually
    ui->twCustomVars->setRowCount(0);
    const auto customVars = m_graphic->getCustomProperties();
    QHashIterator<QString, RS_Variable> it(customVars);
    int row = 0;
    while (it.hasNext()) {
        it.next();
        ui->twCustomVars->insertRow(row);
        auto* nameItem = new QTableWidgetItem(it.key());
        nameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        ui->twCustomVars->setItem(row, 0, nameItem);
        ui->twCustomVars->setItem(row, 1, new QTableWidgetItem(it.value().getString()));
        row++;
    }
    ui->pbCustomVarDelete->setEnabled(false);
}

bool LC_SettingsPageDrawingUserData::saveSettings() {
    if (!m_graphic) return false;

    // Save Integer variables $USERI1 - $USERI5
    m_graphic->addVariable("$USERI1", ui->sbUserI1->value(), 70);
    m_graphic->addVariable("$USERI2", ui->sbUserI2->value(), 70);
    m_graphic->addVariable("$USERI3", ui->sbUserI3->value(), 70);
    m_graphic->addVariable("$USERI4", ui->sbUserI4->value(), 70);
    m_graphic->addVariable("$USERI5", ui->sbUserI5->value(), 70);

    // Save Real variables $USERR1 - $USERR5
    m_graphic->addVariable("$USERR1", RS_Math::eval(ui->leUserR1->text(), 0.0), 40);
    m_graphic->addVariable("$USERR2", RS_Math::eval(ui->leUserR2->text(), 0.0), 40);
    m_graphic->addVariable("$USERR3", RS_Math::eval(ui->leUserR3->text(), 0.0), 40);
    m_graphic->addVariable("$USERR4", RS_Math::eval(ui->leUserR4->text(), 0.0), 40);
    m_graphic->addVariable("$USERR5", RS_Math::eval(ui->leUserR5->text(), 0.0), 40);

    // Save Custom Variables Table manually
    QHash<QString, QString> newCustomVars;
    for (int i = 0; i < ui->twCustomVars->rowCount(); ++i) {
        auto* nameItem = ui->twCustomVars->item(i, 0);
        auto* valueItem = ui->twCustomVars->item(i, 1);

        // Defensive Null-Pointer Guard
        if (nameItem && valueItem) {
            newCustomVars.insert(nameItem->text().trimmed(), valueItem->text());
        }
    }
    m_graphic->replaceCustomVars(newCustomVars);
    return true;
}

bool LC_SettingsPageDrawingUserData::isModified() const {
    if (!m_graphic) return false;

    // 1. Compare Integer variables $USERI1 - $USERI5
    if (ui->sbUserI1->value() != m_graphic->getVariableInt("$USERI1", 0) ||
        ui->sbUserI2->value() != m_graphic->getVariableInt("$USERI2", 0) ||
        ui->sbUserI3->value() != m_graphic->getVariableInt("$USERI3", 0) ||
        ui->sbUserI4->value() != m_graphic->getVariableInt("$USERI4", 0) ||
        ui->sbUserI5->value() != m_graphic->getVariableInt("$USERI5", 0)) {
        return true;
    }

    // 2. Compare Real variables $USERR1 - $USERR5
    auto doubleDiff = [](double v1, double v2) {
        return std::abs(v1 - v2) > 1e-9;
    };
    if (doubleDiff(RS_Math::eval(ui->leUserR1->text()), m_graphic->getVariableDouble("$USERR1", 0.0)) ||
        doubleDiff(RS_Math::eval(ui->leUserR2->text()), m_graphic->getVariableDouble("$USERR2", 0.0)) ||
        doubleDiff(RS_Math::eval(ui->leUserR3->text()), m_graphic->getVariableDouble("$USERR3", 0.0)) ||
        doubleDiff(RS_Math::eval(ui->leUserR4->text()), m_graphic->getVariableDouble("$USERR4", 0.0)) ||
        doubleDiff(RS_Math::eval(ui->leUserR5->text()), m_graphic->getVariableDouble("$USERR5", 0.0))) {
        return true;
    }

    // 3. Compare Custom Variables Table
    const auto customVars = m_graphic->getCustomProperties();
    if (ui->twCustomVars->rowCount() != customVars.count()) {
        return true;
    }

    for (int i = 0; i < ui->twCustomVars->rowCount(); ++i) {
        auto* nameItem = ui->twCustomVars->item(i, 0);
        auto* valueItem = ui->twCustomVars->item(i, 1);

        // Defensive Null-Pointer Guard
        if (!nameItem || !valueItem) {
            return true;
        }

        QString name = nameItem->text().trimmed();
        QString value = valueItem->text();

        if (!customVars.contains(name) || customVars.value(name).getString() != value) {
            return true;
        }
    }

    return false;
}

void LC_SettingsPageDrawingUserData::onAddCustomProperty() {
    LC_DlgNewCustomVariable dlg(m_widget);
    QStringList existing;
    for (int i = 0; i < ui->twCustomVars->rowCount(); ++i) {
        auto* item = ui->twCustomVars->item(i, 0);
        if (item) existing << item->text();
    }
    dlg.setPropertyNames(&existing);

    if (dlg.exec() == QDialog::Accepted) {
        QString propName = dlg.getPropertyName().trimmed();
        if (propName.isEmpty()) return;

        int row = ui->twCustomVars->rowCount();
        ui->twCustomVars->insertRow(row);

        // FIX: Ensure newly added key is read-only (un-editable) just like loaded ones
        auto* nameItem = new QTableWidgetItem(propName);
        nameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        ui->twCustomVars->setItem(row, 0, nameItem);

        ui->twCustomVars->setItem(row, 1, new QTableWidgetItem(dlg.getPropertyValue()));
    }
}


void LC_SettingsPageDrawingUserData::onDeleteCustomProperty() {
    int row = ui->twCustomVars->currentRow();
    if (row < 0) return;

    QString propName = ui->twCustomVars->item(row, 0)->text();
    if (QMessageBox::warning(m_widget, tr("Delete Custom Property"),
        tr("Are you sure you want to delete property [%1]?").arg(propName),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
    {
        ui->twCustomVars->removeRow(row);
    }
}
