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

#ifndef LC_LAZY_FONT_COMBO_BOX_H
#define LC_LAZY_FONT_COMBO_BOX_H

#include <QComboBox>
#include <QFontDatabase>

class LC_LazyFontComboBox : public QComboBox {
    Q_OBJECT
public:
    explicit LC_LazyFontComboBox(QWidget* parent = nullptr)
        : QComboBox(parent) {
    }

    void setCurrentFontName(const QString& fontName) {
        if (fontName.isEmpty()) {
            return;
        }
        const int idx = findText(fontName);
        if (idx >= 0) {
            setCurrentIndex(idx);
        }
        else {
            insertItem(0, fontName);
            setCurrentIndex(0);
        }
    }

    void showPopup() override {
        ensureFontsPopulated();
        QComboBox::showPopup();
    }

private:
    void ensureFontsPopulated() {
        if (m_fontsPopulated) {
            return;
        }
        m_fontsPopulated = true;

        const QString currentFont = currentText();
        const QStringList families = QFontDatabase::families();

        blockSignals(true);
        clear();
        addItems(families);

        if (!currentFont.isEmpty()) {
            const int idx = findText(currentFont);
            if (idx >= 0) {
                setCurrentIndex(idx);
            }
            else {
                insertItem(0, currentFont);
                setCurrentIndex(0);
            }
        }
        blockSignals(false);
    }

    bool m_fontsPopulated = false;
};

#endif 
