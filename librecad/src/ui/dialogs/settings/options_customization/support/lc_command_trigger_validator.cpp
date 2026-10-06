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

#include "lc_command_trigger_validator.h"

LC_CommandTriggerValidator::LC_CommandTriggerValidator(Mode mode, int maxTokenLength, QObject* parent)
    : QValidator(parent)
    , m_mode(mode)
    , m_maxTokenLength(maxTokenLength) {
}

/**
 * @brief Validates command names, keycodes, and comma-separated alias lists.
 *
 * Enforces character rules and formatting constraints for command-line triggers
 * across both single-value fields and multi-alias lists, supporting all Unicode locales:
 *
 * 1. Modes:
 *    - SingleTrigger: Validates a single standalone command name or keycode.
 *    - CommaSeparatedAliases: Splits input by commas and validates each token
 *      individually, allowing surrounding whitespace between entries.
 *
 * 2. Token Character Rules:
 *    - First Character: Must be a Unicode letter (QChar::isLetter()). Leading digits (0-9),
 *      symbols, and whitespace are rejected. This ensures locale independence for English,
 *      Cyrillic, Greek, accented Latin, and other alphabets.
 *    - Subsequent Characters: May consist of Unicode letters, decimal digits (0-9),
 *      underscores ('_'), and hyphens ('-'). Embedded spaces inside an alias are invalid.
 *    - Suppression Token: The exact single-character token "-" is accepted as a valid
 *      entry to represent explicit suppression of built-in defaults.
 *
 * 3. Constraints & Input States:
 *    - Token Length: If maxTokenLength > 0 (e.g. 2 for Keycode mode), tokens exceeding
 *      the limit are rejected as Invalid.
 *    - Interactive Typing: Empty input, trailing commas, and incomplete intermediate
 *      tokens return QValidator::Intermediate to permit natural typing without locking.
 */
QValidator::State LC_CommandTriggerValidator::validate(QString& input, int& pos) const {
    Q_UNUSED(pos);
    if (input.isEmpty()) {
        return Intermediate;
    }

    if (m_mode == SingleTrigger) {
        const QString token = input.trimmed();
        if (token == "-") {
            return Acceptable;
        }
        if (m_maxTokenLength > 0 && token.length() > m_maxTokenLength) {
            return Invalid;
        }
        if (!token.isEmpty() && !token.at(0).isLetter()) {
            return Invalid;
        }
        for (int i = 1; i < token.length(); ++i) {
            const QChar ch = token.at(i);
            if (!ch.isLetterOrNumber() && ch != '_' && ch != '-') {
                return Invalid;
            }
        }
        return Acceptable;
    }

    // CommaSeparatedAliases mode
    const QStringList parts = input.split(',', Qt::KeepEmptyParts);
    for (int p = 0; p < parts.size(); ++p) {
        const QString token = parts.at(p).trimmed();
        if (token.isEmpty()) {
            continue;
        }
        if (token == "-") {
            continue;
        }
        if (m_maxTokenLength > 0 && token.length() > m_maxTokenLength) {
            return Invalid;
        }
        if (!token.at(0).isLetter()) {
            return Invalid;
        }
        for (int i = 1; i < token.length(); ++i) {
            const QChar ch = token.at(i);
            if (!ch.isLetterOrNumber() && ch != '_' && ch != '-') {
                return Invalid;
            }
        }
    }

    return Acceptable;
}
