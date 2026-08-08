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

#ifndef LC_DOCUMENT_SETTINGS_BACKEND_H
#define LC_DOCUMENT_SETTINGS_BACKEND_H

#include "lc_settings_backend.h"
#include "rs_graphic.h"
#include <QVariant>
#include <QHash>
#include <QString>

/**
 * @brief The LC_DocumentSettingsBackend class
 * Proxies LC_Setting bindings to the internal DXF variable system of an RS_Graphic.
 * Supports transactional changes (flushed only on sync()).
 */
class LC_DocumentSettingsBackend : public LC_SettingsBackend {
public:
    explicit LC_DocumentSettingsBackend(RS_Graphic* graphic) : m_graphic(graphic) {}

    /**
     * @brief Reads a value from the graphic or the local transactional cache.
     */
    QVariant value(const QString& key, const QVariant& defaultValue) const override {
        if (!m_graphic) return defaultValue;

        // 1. Check the local cache first (uncommitted changes)
        if (m_cache.contains(key)) {
            return m_cache[key];
        }

        // 2. Map logical UI key to physical DXF variable
        QString dxfKey = mapToDxf(key);

        // 3. Query the graphic based on the expected type
        if (defaultValue.typeId() == QMetaType::Int) {
            return m_graphic->getVariableInt(dxfKey, defaultValue.toInt());
        }
        else if (defaultValue.typeId() == QMetaType::Double) {
            return m_graphic->getVariableDouble(dxfKey, defaultValue.toDouble());
        }
        else if (defaultValue.typeId() == QMetaType::Bool) {
            // DXF uses integers (0/1) for booleans
            return m_graphic->getVariableInt(dxfKey, defaultValue.toBool() ? 1 : 0) != 0;
        }

        return m_graphic->getVariableString(dxfKey, defaultValue.toString());
    }

    /**
     * @brief Writes a value to the local transactional cache.
     */
    void setValue(const QString& key, const QVariant& val) override {
        m_cache[key] = val;
    }

    /**
     * @brief Commits all cached changes back to the actual RS_Graphic variables.
     */
    bool sync() override {
        if (!m_graphic) {
            return false;
        }
        if (m_cache.isEmpty()) {
            return true;
        }

        for (auto it = m_cache.begin(); it != m_cache.end(); ++it) {
            QString dxfKey = mapToDxf(it.key());
            QVariant val = it.value();

            // Determine appropriate DXF Group Code based on Variant type
            // Standard codes: 1 = String, 40 = Double, 70 = Integer
            if (val.typeId() == QMetaType::Int) {
                m_graphic->addVariable(dxfKey, val.toInt(), 70);
            }
            else if (val.typeId() == QMetaType::Double) {
                m_graphic->addVariable(dxfKey, val.toDouble(), 40);
            }
            else if (val.typeId() == QMetaType::Bool) {
                m_graphic->addVariable(dxfKey, val.toBool() ? 1 : 0, 70);
            }
            else {
                m_graphic->addVariable(dxfKey, val.toString(), 1);
            }
        }

        m_graphic->setModified(true);
        m_cache.clear(); // Clear cache after successful flush
        return true;
    }

private:
    /**
     * @brief Translates UI-friendly logical paths into DXF Header variable names.
     */
    static QString mapToDxf(const QString& key) {
        // Units Tab
        if (key == "Units/Main")            return "$INSUNITS";
        if (key == "Units/LinearFormat")    return "$LUNITS";
        if (key == "Units/LinearPrec")      return "$LUPREC";
        if (key == "Units/AngleFormat")     return "$AUNITS";
        if (key == "Units/AnglePrec")       return "$AUPREC";

        // Metadata Tab
        if (key == "Meta/Title")            return "$TITLE";
        if (key == "Meta/Subject")          return "$SUBJECT";
        if (key == "Meta/Author")           return "$AUTHOR";
        if (key == "Meta/Keywords")         return "$KEYWORDS";
        if (key == "Meta/Comments")         return "$COMMENTS";

        // Geometry / Points / Splines
        if (key == "Geometry/PointMode")    return "$PDMODE";
        if (key == "Geometry/PointSize")    return "$PDSIZE";
        if (key == "Geometry/SplineSegs")   return "$SPLINESEGS";

        // Lines
        if (key == "Geometry/LineCaps")     return "$ENDCAPS";
        if (key == "Geometry/LineJoin")     return "$JOINSTYLE";

        if (key.startsWith("$USER")) return key;

        // Fallback: If key already starts with $, assume it's a direct DXF variable
        if (key.startsWith('$')) return key;

        // Auto-convert logical paths "Group/Key" to "$GROUP_KEY"
        QString fallback = key.toUpper().replace("/", "_");
        if (!fallback.startsWith('$')) fallback.prepend('$');
        return fallback;
    }

    RS_Graphic* m_graphic;
    mutable QHash<QString, QVariant> m_cache;
};

#endif
