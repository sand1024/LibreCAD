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

#include "lc_skin_archetype_header_bar.h"
#include "ui_lc_skin_archetype_header_bar.h"

LC_SkinArchetypeHeaderBar::LC_SkinArchetypeHeaderBar(QWidget* parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::LC_SkinArchetypeHeaderBar>()) {
    ui->setupUi(this);

    setupComboboxes();

    connect(ui->cbStyleArchetype, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LC_SkinArchetypeHeaderBar::onArchetypeIndexChanged);
    connect(ui->cbBoxDecoration, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LC_SkinArchetypeHeaderBar::onDecorationIndexChanged);
}

LC_SkinArchetypeHeaderBar::~LC_SkinArchetypeHeaderBar() = default;

void LC_SkinArchetypeHeaderBar::setupComboboxes() {
    ui->cbStyleArchetype->clear();
    ui->cbStyleArchetype->addItem(tr("Classic Fusion (Native UI)"), static_cast<int>(StyleArchetype::ClassicFusion));
    ui->cbStyleArchetype->addItem(tr("Flat Modern"), static_cast<int>(StyleArchetype::FlatModern));
    ui->cbStyleArchetype->addItem(tr("Soft Satin"), static_cast<int>(StyleArchetype::SoftSatin));
    ui->cbStyleArchetype->addItem(tr("Glassy Gloss (Vista Style)"), static_cast<int>(StyleArchetype::GlassyGloss));
    ui->cbStyleArchetype->addItem(tr("Accent Outline"), static_cast<int>(StyleArchetype::AccentOutline));

    ui->cbBoxDecoration->clear();
    ui->cbBoxDecoration->addItem(tr("Frameless (Clean Flat)"), static_cast<int>(BoxDecoration::Frameless));
    ui->cbBoxDecoration->addItem(tr("Dividing Hairline"), static_cast<int>(BoxDecoration::DividingHairline));
    ui->cbBoxDecoration->addItem(tr("Box Outline Frame"), static_cast<int>(BoxDecoration::BoxOutline));
    ui->cbBoxDecoration->addItem(tr("Recessed Well"), static_cast<int>(BoxDecoration::RecessedWell));
    ui->cbBoxDecoration->addItem(tr("Active Left Sidebar (IDE Style)"), static_cast<int>(BoxDecoration::LeftAccentBar));
    ui->cbBoxDecoration->addItem(tr("Active Accent Frame"), static_cast<int>(BoxDecoration::ActiveAccentFrame));
}

void LC_SkinArchetypeHeaderBar::populateFromConfig(const ControlStyleConfig& config) {
    m_blockSignals = true;

    const int archIdx = ui->cbStyleArchetype->findData(static_cast<int>(config.styleArchetype));
    if (archIdx >= 0) {
        ui->cbStyleArchetype->setCurrentIndex(archIdx);
    }
    ui->lblStyleArchetypeDescription->setText(getArchetypeDescription(config.styleArchetype));

    const bool isClassic = (config.styleArchetype == StyleArchetype::ClassicFusion);

    // Archetype combobox is always enabled; only container decoration is disabled in Classic Fusion
    ui->cbStyleArchetype->setEnabled(true);
    ui->lblStyleArchetype->setEnabled(true);
    ui->cbBoxDecoration->setEnabled(!isClassic);
    ui->lblBoxDecoration->setEnabled(!isClassic);

    const int decIdx = ui->cbBoxDecoration->findData(static_cast<int>(config.boxDecoration));
    if (decIdx >= 0) {
        ui->cbBoxDecoration->setCurrentIndex(decIdx);
    }

    if (isClassic) {
        ui->lblCompositionDesc->setText(tr("<b>Container Decoration</b>: Not applicable in Classic Fusion (native 3D bevels and borders are used)."));
    }
    else {
        ui->lblCompositionDesc->setText(getDecorationDescription(config.boxDecoration));
    }

    m_blockSignals = false;
}

StyleArchetype LC_SkinArchetypeHeaderBar::styleArchetype() const {
    return static_cast<StyleArchetype>(ui->cbStyleArchetype->currentData().toInt());
}

BoxDecoration LC_SkinArchetypeHeaderBar::boxDecoration() const {
    return static_cast<BoxDecoration>(ui->cbBoxDecoration->currentData().toInt());
}

void LC_SkinArchetypeHeaderBar::onArchetypeIndexChanged(int index) {
    if (index < 0) {
        return;
    }
    const auto archetype = static_cast<StyleArchetype>(ui->cbStyleArchetype->currentData().toInt());
    ui->lblStyleArchetypeDescription->setText(getArchetypeDescription(archetype));

    const bool isClassic = (archetype == StyleArchetype::ClassicFusion);
    ui->cbBoxDecoration->setEnabled(!isClassic);
    ui->lblBoxDecoration->setEnabled(!isClassic);

    if (isClassic) {
        ui->lblCompositionDesc->setText(tr("<b>Container Decoration</b>: Not applicable in Classic Fusion (native 3D bevels and borders are used)."));
    }
    else {
        const auto decoration = static_cast<BoxDecoration>(ui->cbBoxDecoration->currentData().toInt());
        ui->lblCompositionDesc->setText(getDecorationDescription(decoration));
    }

    if (!m_blockSignals) {
        emit archetypeChanged(archetype);
    }
}

void LC_SkinArchetypeHeaderBar::onDecorationIndexChanged(int index) {
    if (index < 0) {
        return;
    }
    const auto decoration = static_cast<BoxDecoration>(ui->cbBoxDecoration->currentData().toInt());
    ui->lblCompositionDesc->setText(getDecorationDescription(decoration));
    if (!m_blockSignals) {
        emit decorationChanged(decoration);
    }
}

QString LC_SkinArchetypeHeaderBar::getArchetypeDescription(StyleArchetype archetype) const {
    switch (archetype) {
        case StyleArchetype::FlatModern:
            return tr("<b>Flat Modern</b>: Flat aesthetic with zero gradients, soft 1px outlines, and subtle responsive lighting.");
        case StyleArchetype::SoftSatin:
            return tr("<b>Soft Satin</b>: Gentle linear gradients and soft lighting transitions simulating elegant satin-like depth.");
        case StyleArchetype::GlassyGloss:
            return tr("<b>Glassy Gloss</b>: Implements soft, curved glossy reflections across control surfaces.");
        case StyleArchetype::AccentOutline:
            return tr("<b>Accent Outline</b>: Transparent body framed by low-contrast outlines, with vibrant highlights providing visual structure.");
        case StyleArchetype::ClassicFusion:
        default:
            return tr("<b>Classic Fusion</b>: Restores standard native Qt Fusion chiseled bevel geometries, 3D borders, and layout styles.");
    }
}

QString LC_SkinArchetypeHeaderBar::getDecorationDescription(BoxDecoration dec) const {
    switch (dec) {
        case BoxDecoration::Frameless:
            return tr("<b>Frameless</b>: Flat container panels with zero border lines or highlight separators.");
        case BoxDecoration::DividingHairline:
            return tr("<b>Dividing Hairline</b>: Draws a flat 1px line in flat/accent styles, and a chiseled 3D shadow in traditional styles.");
        case BoxDecoration::BoxOutline:
            return tr("<b>Box Outline Frame</b>: Complete flat 1px bounding outline frame wrapping around widget bounds.");
        case BoxDecoration::RecessedWell:
            return tr("<b>Recessed Well</b>: Simulates an inset groove. Draws a soft inner-offset frame in flat styles and a 3D shadow inset in traditional styles.");
        case BoxDecoration::LeftAccentBar:
            return tr("<b>Active Left Sidebar</b>: Left vertical 3px active accent indicator sidebar.");
        case BoxDecoration::ActiveAccentFrame:
            return tr("<b>Active Accent Frame</b>: Highlights the entire container panel with a 1px active highlight accent border.");
    }
    return QString();
}
