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

#include "lc_imagepropertieseditingwidget.h"

#include <QFileInfo>

#include "lc_graphicviewport.h"
#include "lc_guarded_signals_blocker.h"
#include "rs_dialogfactory.h"
#include "rs_dialogfactoryinterface.h"
#include "rs_image.h"
#include "rs_units.h"
#include "ui_lc_imagepropertieseditingwidget.h"

#define ALLOW_PICK_SCALE false

LC_ImagePropertiesEditingWidget::LC_ImagePropertiesEditingWidget(QWidget* parent)
    : LC_EntityPropertiesEditorWidget(parent), ui(new Ui::LC_ImagePropertiesEditingWidget) {
    ui->setupUi(this);
    connect(ui->leInsertX, &QLineEdit::editingFinished, this, &LC_ImagePropertiesEditingWidget::onInsertionPointEditingFinished);
    connect(ui->leInsertY, &QLineEdit::editingFinished, this, &LC_ImagePropertiesEditingWidget::onInsertionPointEditingFinished);
    connect(ui->leAngle, &QLineEdit::editingFinished, this, &LC_ImagePropertiesEditingWidget::onAngleEditingFinished);
    connect(ui->leWidth, &QLineEdit::editingFinished, this, &LC_ImagePropertiesEditingWidget::onWidthChanged);
    connect(ui->leHeight, &QLineEdit::editingFinished, this, &LC_ImagePropertiesEditingWidget::onHeightChanged);
    connect(ui->leScaleX, &QLineEdit::editingFinished, this, &LC_ImagePropertiesEditingWidget::onScaleXChanged);
    connect(ui->leScaleY, &QLineEdit::editingFinished, this, &LC_ImagePropertiesEditingWidget::onScaleYChanged);

    connect(ui->pbFile, &QPushButton::toggled, this, &LC_ImagePropertiesEditingWidget::onImageFileClick);
    connect(ui->lePath, &QLineEdit::textChanged, this, &LC_ImagePropertiesEditingWidget::onPathChanged);
    if (!ALLOW_PICK_SCALE) {
        ui->tbPickScaleX->setVisible(false);
        ui->tbPickScaleY->setVisible(false);
    }
}

LC_ImagePropertiesEditingWidget::~LC_ImagePropertiesEditingWidget() {
    delete ui;
}

void LC_ImagePropertiesEditingWidget::updateUIbyEntity() {
    LC_GuardedSignalsBlocker signalsBlocker({
        ui->leInsertX,
        ui->leInsertX,
        ui->leAngle,
        ui->leWidth,
        ui->leHeight,
        ui->leScaleX,
        ui->leScaleY,
        ui->lePath
    });

    const auto wcsInsertionPoint = m_entity->getInsertionPoint();
    const auto ucsInsertionPoint = m_viewport->toUCS(wcsInsertionPoint);
    toUI(ucsInsertionPoint, ui->leInsertX, ui->leInsertY);
    toUIAngleDeg(m_entity->getUVector().angle(), ui->leAngle);
    toUIValue(m_entity->getImageWidth(), ui->leWidth);
    toUIValue(m_entity->getImageHeight(), ui->leHeight);
    RS_Vector scale = m_entity->getScale();

    toUIValue(scale.x, ui->leScaleX);
    toUIValue(scale.y, ui->leScaleY);

    updateDPI(scale);
    updateSizeInPixels();
    ui->lePath->setText(m_entity->getFile());
}

void LC_ImagePropertiesEditingWidget::setEntity(RS_Entity* entity) {
    m_entity = static_cast<RS_Image*>(entity);
    updateUIbyEntity();
}

void LC_ImagePropertiesEditingWidget::onWidthChanged() {
    const RS_Vector originalSize = m_entity->getImageSize();

    double newWidthY;
    const bool meaningful = toDouble(ui->leWidth->text(), newWidthY, 0.0, true);
    newWidthY = meaningful ? newWidthY : originalSize.x;

    double scaleY = 1.0;
    double scaleX = newWidthY / originalSize.x;
    m_entity->scale(m_entity->getInsertionPoint(), RS_Vector(scaleX, scaleY));
    updateUIbyEntity();
}

void LC_ImagePropertiesEditingWidget::onHeightChanged() {
    const RS_Vector originalSize = m_entity->getImageSize();

    double newHeightY;
    const bool meaningful = toDouble(ui->leHeight->text(), newHeightY, 0.0, true);
    newHeightY = meaningful ? newHeightY : originalSize.y;

    double scaleX = 1.0;
    double scaleY = newHeightY / originalSize.y;
    m_entity->scale(m_entity->getInsertionPoint(), RS_Vector(scaleX, scaleY));
    updateUIbyEntity();
}

void LC_ImagePropertiesEditingWidget::onScaleXChanged() {
    RS_Vector scale = m_entity->getScale();

    double newScaleX;
    const bool meaningful = toDouble(ui->leScaleX->text(), newScaleX, 1.0, true);
    newScaleX = meaningful ? newScaleX : scale.getX();
    double scaleY = 1.0;
    double scaleX = newScaleX / scale.x;

    m_entity->scale(m_entity->getInsertionPoint(), RS_Vector(scaleX, scaleY));
    updateUIbyEntity();
}

void LC_ImagePropertiesEditingWidget::onScaleYChanged() {
    RS_Vector scale = m_entity->getScale();

    double newScaleY;
    const bool meaningful = toDouble(ui->leScaleY->text(), newScaleY, 1.0, true);
    newScaleY = meaningful ? newScaleY : scale.getY();
    double scaleX = 1.0;
    double scaleY = newScaleY / scale.y;

    m_entity->scale(m_entity->getInsertionPoint(), RS_Vector(scaleX, scaleY));
    updateUIbyEntity();
}

void LC_ImagePropertiesEditingWidget::onInsertionPointEditingFinished() const {
    m_entity->setInsertionPoint(toWCS(ui->leInsertX, ui->leInsertY, m_entity->getInsertionPoint()));
}

void LC_ImagePropertiesEditingWidget::onAngleEditingFinished() {
    const double orgScale = m_entity->getUVector().magnitude();
    m_scale /= orgScale;
    const double orgAngle = m_entity->getUVector().angle();
    const double angle = toWCSAngle(ui->leAngle, orgAngle);
    m_entity->scale(m_entity->getInsertionPoint(), RS_Vector(m_scale, m_scale));
    m_entity->rotate(m_entity->getInsertionPoint(), angle - orgAngle);
}

void LC_ImagePropertiesEditingWidget::onImageFileClick() const {
    ui->lePath->setText(RS_DIALOGFACTORY->requestImageOpenDialog()); // fixme - isn't it bad dependency?
}

void LC_ImagePropertiesEditingWidget::onPathChanged(const QString&) const {
    const auto text = ui->lePath->text().trimmed();
    if (QFileInfo(text).isFile()) {
        m_entity->setFile(text);
    }
}

void LC_ImagePropertiesEditingWidget::updateDPI(const RS_Vector& scale) const {
    const auto graphicUnit = m_entity->getGraphicUnit();
    const double dpiX = RS_Units::scaleToDpi(scale.x, graphicUnit);
    const double dpiY = RS_Units::scaleToDpi(scale.y, graphicUnit);
    const QString sX = asString(dpiX);
    const QString sY = asString(dpiY);
    const QString dpiStr = QString("[%1 x %2]").arg(sX, sY);
    ui->lblDPI->setText(dpiStr);
}

void LC_ImagePropertiesEditingWidget::updateSizeInPixels() {
    QString sizeX = QString::number(m_entity->getWidth());
    QString sizeY = QString::number(m_entity->getHeight());
    const QString sizeStr = QString("[%1 x %2] px").arg(sizeX, sizeY);
    ui->lblPictureSize->setText(sizeStr);
}

void LC_ImagePropertiesEditingWidget::setupInteractiveInputWidgets() {
    pickPointSetup(ui->wPickInsertionPoint, "insert", ui->leInsertX, ui->leInsertY);
    pickDistanceSetup(ui->tbPickWidth, "width", ui->leWidth);
    pickDistanceSetup(ui->tbPickHeight, "height", ui->leHeight);
    pickAngleSetup(ui->tbPickAngle, "angle", ui->leAngle);
    if (!ALLOW_PICK_SCALE) {
        pickDistanceSetup(ui->tbPickScaleX, "scalex", ui->leScaleX);
        pickDistanceSetup(ui->tbPickScaleY, "scaley", ui->leScaleY);
    }
}
