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

#ifndef LC_SKIN_ARCHETYPE_HEADER_BAR_H
#define LC_SKIN_ARCHETYPE_HEADER_BAR_H

#include <QWidget>
#include <memory>
#include "lc_palette_editor_shared.h"

namespace Ui {
    class LC_SkinArchetypeHeaderBar;
}

class LC_SkinArchetypeHeaderBar : public QWidget {
    Q_OBJECT
public:
    explicit LC_SkinArchetypeHeaderBar(QWidget* parent = nullptr);
    ~LC_SkinArchetypeHeaderBar() override;

    void populateFromConfig(const SkinConfig& config);
    StyleArchetype styleArchetype() const;
    BoxDecoration boxDecoration() const;

signals:
    void archetypeChanged(StyleArchetype archetype);
    void decorationChanged(BoxDecoration decoration);

private slots:
    void onArchetypeIndexChanged(int index);
    void onDecorationIndexChanged(int index);

private:
    void setupComboboxes();
    QString getArchetypeDescription(StyleArchetype archetype) const;
    QString getDecorationDescription(BoxDecoration dec) const;

    std::unique_ptr<Ui::LC_SkinArchetypeHeaderBar> ui;
    bool m_blockSignals = false;
};

#endif
