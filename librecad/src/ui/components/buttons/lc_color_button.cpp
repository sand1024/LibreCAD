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

#include "lc_color_button.h"

#include <QColorDialog>
#include <QEvent>
#include <QPainter>

#include "lc_action_draw_text.h"
#include "lc_settings_defaults.h"

LC_ColorButton::LC_ColorButton(const QColor& color, QWidget* parent)
    : QPushButton(parent), m_color(color) {
    connect(this, &QPushButton::clicked, this, &LC_ColorButton::chooseColor);
    updateSwatch();

}LC_ColorButton::LC_ColorButton(QWidget* parent)
    : QPushButton(parent), m_color(QColor()) {
    connect(this, &QPushButton::clicked, this, &LC_ColorButton::chooseColor);
    updateSwatch();
}

QColor LC_ColorButton::color() const {
    return m_color;
}

void LC_ColorButton::setColor(const QColor& color) {
    if (m_color != color) {
        m_color = color;
        m_toolTipDirty = true;
        updateSwatch();
    }
}

void LC_ColorButton::setLocked(bool locked) {
    if (m_locked != locked) {
        m_locked = locked;
        setCursor(locked ? Qt::ArrowCursor : Qt::PointingHandCursor);
        m_toolTipDirty = true;
        updateSwatch();
    }
}

void LC_ColorButton::setToolTip(const QString& text) {
    if (m_normalToolTip != text) {
        m_normalToolTip = text;
        m_toolTipDirty = true;
    }
}

void LC_ColorButton::setLockedToolTip(const QString& toolTip) {
    if (m_lockedToolTip != toolTip) {
        m_lockedToolTip = toolTip;
        m_toolTipDirty = true;
    }
}

void LC_ColorButton::setDialogTitle(const QString& title) {
    m_dialogTitle = title;
}

QString LC_ColorButton::buildToolTipText() const {
    QString baseText;
    if (m_locked) {
        baseText = !m_lockedToolTip.isEmpty() ? m_lockedToolTip : m_normalToolTip;
    }
    else {
        baseText = m_normalToolTip;
    }

    QStringList metaParts;
    if (m_color.isValid()) {
        const QString hexStr = (m_color.alpha() < 255)
                                   ? m_color.name(QColor::HexArgb).toUpper()
                                   : m_color.name(QColor::HexRgb).toUpper();

        const QString rgbStr = (m_color.alpha() < 255)
                                   ? QString("rgba(%1, %2, %3, %4)")
                                         .arg(QString::number(m_color.red()),
                                              QString::number(m_color.green()),
                                              QString::number(m_color.blue()),
                                              QString::number(m_color.alpha()))
                                   : QString("rgb(%1, %2, %3)")
                                         .arg(QString::number(m_color.red()),
                                              QString::number(m_color.green()),
                                              QString::number(m_color.blue()));

        metaParts.append(QString("%1: <b>%2</b>").arg(tr("Hex"), hexStr));
        metaParts.append(QString("%1: <b>%2</b>").arg(tr("RGB"), rgbStr));
    }

    if (baseText.isEmpty() && metaParts.isEmpty()) {
        return QString();
    }

    if (metaParts.isEmpty()) {
        return baseText;
    }

    const QPalette pal = palette();
    QColor metaColor = pal.color(QPalette::PlaceholderText);
    if (metaColor.value() == 0 && pal.color(QPalette::ToolTipText).value() == 0) {
        metaColor = QColor("gray");
    }

    const QString metaFooter = metaParts.join(" &nbsp;|&nbsp; ");

    QString html;
    if (!baseText.isEmpty()) {
        bool mightBeRichText = Qt::mightBeRichText(baseText);
        if (baseText.contains("\n\n") && !mightBeRichText) {
            const int splitIdx = baseText.indexOf("\n\n");
            if (splitIdx != -1) {
                QString title = baseText.left(splitIdx).toHtmlEscaped();
                QString body = baseText.mid(splitIdx + 2).toHtmlEscaped();
                body.replace("\n", "<br>");
                html = QString("<b>%1</b><hr>%2").arg(title, body);
            }
            else {
                const QString escapedText = mightBeRichText ? baseText : baseText.toHtmlEscaped();
                html += QString("<div>%1</div>").arg(escapedText);
            }
        }
        else {
            const QString escapedText = mightBeRichText ? baseText : baseText.toHtmlEscaped();
            html += QString("<div>%1</div>").arg(escapedText);
        }
    }

    html += QString("<div style=\"color: %1; font-size: small;\"><br>%2</div>")
                .arg(metaColor.name(), metaFooter);

    return html;
}

void LC_ColorButton::updateToolTip() {
    if (m_toolTipDirty) {
        m_cachedToolTip = buildToolTipText();
        QPushButton::setToolTip(m_cachedToolTip);
        m_toolTipDirty = false;
    }
}


void LC_ColorButton::updateSwatch() {
    update();
}

bool LC_ColorButton::event(QEvent* event) {
    if (event != nullptr) {
        if (event->type() == QEvent::ToolTip) {
            if (m_toolTipDirty) {
                updateToolTip();
            }
        }
        else if (event->type() == QEvent::PaletteChange) {
            m_toolTipDirty = true;
        }
    }
    return QPushButton::event(event);
}

void LC_ColorButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QRectF rF = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);

    // 1. Draw the background
    if (m_color.isValid()) {
        // Draw the solid colored swatch background
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_color);
        painter.drawRoundedRect(rF, 4.0, 4.0);
    } else {
        // Draw a neutral off-white background with a soft red diagonal slash representing unset/inherited
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor("#f4f4f4"));
        painter.drawRoundedRect(rF, 4.0, 4.0);

        painter.setPen(QPen(QColor("#d9534f"), 1.5, Qt::SolidLine, Qt::RoundCap)); // Soft red slash
        painter.drawLine(rF.topLeft() + QPointF(3, 3), rF.bottomRight() - QPointF(3, 3));
    }

    // 2. Draw the border outline based on the lock status
    if (m_locked) {
        // Dashed outline indicating the swatch is currently auto-calculated
        QPen borderPen(QColor("#aaaaaa"), 1, Qt::DashLine);
        painter.setPen(borderPen);
    } else {
        // Solid outline for manually specified colors
        QPen borderPen(QColor("#777777"), 1, Qt::SolidLine);
        painter.setPen(borderPen);
    }

    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(rF, 4.0, 4.0);
}

void LC_ColorButton::chooseColor() {
    if (m_locked) {
        return; // Guard clause against clicks when locked
    }

    QColor initial = m_color.isValid() ? m_color : Qt::white;

    // Use the dynamic dialog title property if set, otherwise fallback to "Select Color"
    QString title = m_dialogTitle.isEmpty() ? tr("Select Color") : m_dialogTitle;

    if (CFG_Defaults::o_UseQtColorPickerDialog) {
        auto *dialog = new QColorDialog(initial, this);
        dialog->setOption(QColorDialog::DontUseNativeDialog, true);
        dialog->setOption(QColorDialog::ShowAlphaChannel, true);

        connect(dialog, &QColorDialog::currentColorChanged, [this](const QColor& color) {
            if (color.isValid()) {
                setColor(color);
                // emit colorSelectionChanged(color);
                emit colorChanged(color);
            }
        });

        int executionResult = dialog->exec();
        if (executionResult == QDialog::Accepted) {
            QColor finalColor = dialog->currentColor();
            setColor(finalColor);
            emit colorChanged(finalColor);
        }
        else {
            setColor(initial);
            emit colorChanged(initial);
        }
    }
    else {
        const QColor col = QColorDialog::getColor(initial, this, title, QColorDialog::ShowAlphaChannel);
        if (col.isValid()) {
            setColor(col);
            emit colorChanged(col);
        }
    }
}
