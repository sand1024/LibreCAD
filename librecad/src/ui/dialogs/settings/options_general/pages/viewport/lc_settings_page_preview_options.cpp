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

#include "lc_settings_page_preview_options.h"

#include <QButtonGroup>

#include "dxf_format.h"
#include "lc_settings_appearance.h"
#include "ui_lc_settings_page_preview_options.h"
#include "lc_settings_backend.h"
#include "lc_settings_colors.h"
#include "rs_settings.h"

LC_SettingsPagePreviewOptions::LC_SettingsPagePreviewOptions(QObject* parent)
    : LC_SettingsPageBase(tr("Preview Entities"), std::make_unique<LC_LibreCADSettingsBackend>(CFG_Appearance::Group), parent)
    , ui(std::make_unique<Ui::LC_SettingsPagePreviewOptions>()) {
}

LC_SettingsPagePreviewOptions::~LC_SettingsPagePreviewOptions() = default;

void LC_SettingsPagePreviewOptions::setupUi() {
    ui->setupUi(m_widget);

    m_buttonGroup = new QButtonGroup(m_widget);
    m_buttonGroup->setExclusive(true);

    m_buttonGroup->addButton(ui->bDot, DXF_FORMAT_PDMode_CentreDot);
    m_buttonGroup->addButton(ui->bBlank, DXF_FORMAT_PDMode_CentreBlank);
    m_buttonGroup->addButton(ui->bPlus, DXF_FORMAT_PDMode_CentrePlus);
    m_buttonGroup->addButton(ui->bCross, DXF_FORMAT_PDMode_CentreCross);
    m_buttonGroup->addButton(ui->bTick, DXF_FORMAT_PDMode_CentreTick);

    m_buttonGroup->addButton(ui->bDotCircle, DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreDot));
    m_buttonGroup->addButton(ui->bBlankCircle, DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreBlank));
    m_buttonGroup->addButton(ui->bPlusCircle, DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentrePlus));
    m_buttonGroup->addButton(ui->bCrossCircle, DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreCross));
    m_buttonGroup->addButton(ui->bTickCircle, DXF_FORMAT_PDMode_EncloseCircle(DXF_FORMAT_PDMode_CentreTick));

    m_buttonGroup->addButton(ui->bDotSquare, DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreDot));
    m_buttonGroup->addButton(ui->bBlankSquare, DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreBlank));
    m_buttonGroup->addButton(ui->bPlusSquare, DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentrePlus));
    m_buttonGroup->addButton(ui->bCrossSquare, DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreCross));
    m_buttonGroup->addButton(ui->bTickSquare, DXF_FORMAT_PDMode_EncloseSquare(DXF_FORMAT_PDMode_CentreTick));

    m_buttonGroup->addButton(ui->bDotCircleSquare, DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreDot));
    m_buttonGroup->addButton(ui->bBlankCircleSquare, DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreBlank));
    m_buttonGroup->addButton(ui->bPlusCircleSquare, DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentrePlus));
    m_buttonGroup->addButton(ui->bCrossCircleSquare, DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreCross));
    m_buttonGroup->addButton(ui->bTickCircleSquare, DXF_FORMAT_PDMode_EncloseCircleSquare(DXF_FORMAT_PDMode_CentreTick));
}

void LC_SettingsPagePreviewOptions::setupBindings() {
    using namespace CFG_Appearance;
    using namespace CFG_Colors;

    bindBoolean({
        { ui->cbDisplayRefPoints, o_VisualizePreviewRefPoints }
    });

    bind(ui->sbMaxPreview, o_MaxPreview, false);

    bindColor({
        { ui->cbPreviewEntitiesColor, ui->pb_previewEntitiesColor, o_PreviewEntitiesColor, tr("Select Preview Entity Color") },
        { ui->cbPreviewRefColor, ui->pb_previewRefColor, o_PreviewReferencesColor, tr("Select Preview Explanatory Entity Color") },
        { ui->cbPreviewRefHighlightColor, ui->pb_previewRefHighlightColor, o_PreviewReferencesHighlightColor, tr("Select Active Explanatory Entity Color") }
    });

    bindString({
       { ui->lePointSize, o_RefPointSize }
   });

    // fixme - sand - preferences - MODIFICATION connect() for modification flag!

    // Custom non-flat size/metric relative to absolute radio group
    bindCustom<QWidget, QString>(m_widget, o_RefPointSize, false,
        [this](QWidget*) {
            bool ok;
            double val = ui->lePointSize->text().toDouble(&ok);
            if (!ok) val = 2.0;
            if (ui->rbRelSize->isChecked()) {
                val = -std::abs(val);
            } else {
                val = std::abs(val);
            }
            return QString::number(val);
        },
        [this](QWidget*, const QString& v) {
            bool ok;
            double val = v.toDouble(&ok);
            if (!ok) val = 2.0;
            ui->rbRelSize->setChecked(val <= 0.0);
            ui->rbAbsSize->setChecked(val > 0.0);
            ui->lePointSize->setText(QString::number(std::abs(val), 'g', 6));
        });

    bindCustom<QButtonGroup, int>(m_buttonGroup, o_RefPointType, false,
        &QButtonGroup::buttonToggled,
        [](QButtonGroup* w) { return w->checkedId(); },
        [](QButtonGroup* w, int id) {
            if (auto* btn = w->button(id)) {
                btn->setChecked(true);
            }
        });
}
