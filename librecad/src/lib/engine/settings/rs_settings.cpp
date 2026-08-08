/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software 
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!  
**
**********************************************************************/

// RVT_PORT changed QSettings s(QSettings::Ini) to QSettings s("./qcad.ini", QSettings::IniFormat);
#include "rs_settings.h"

#include <QSettings>

#include "rs_debug.h"
#include "rs_pen.h"


RS_Settings::GroupGuard::GroupGuard(const QString &group):m_group{group} {}

RS_Settings::GroupGuard::~GroupGuard(){
    try {
        const auto instance = RS_Settings::instance();
        instance ->endGroup();
        if (!m_group.isEmpty()) {
            RS_Settings::instance()-> beginGroup(m_group);
        }
    } catch (...) {
        LC_LOG(RS_Debug::D_CRITICAL) << "RS_SETTINGS cleanup error";
    }
}

bool RS_Settings::saveIsAllowed = true;


RS_Settings *RS_Settings::instance() {
    return INSTANCE;
}

/**
 * Initialisation.
 *
 * @param companyKey String that identifies the company. Must start
 *        with a "/". E.g. "/RibbonSoft"
 * @param appKey String that identifies the application. Must start
 *        with a "/". E.g. "/LibreCAD"
 */
void RS_Settings::init(const QString &companyKey,const QString &appKey) {
    auto* settings = new QSettings(companyKey, appKey);
    INSTANCE = new RS_Settings(settings);
}

RS_Settings::RS_Settings(QSettings *qsettings) {
    m_settings = qsettings;
    m_group.clear();
}

RS_Settings::~RS_Settings() {
    delete m_settings;
    m_cache.clear();
}


void RS_Settings::beginGroup(const QString& group)  {
    QString actualGroup = group;
    m_group = std::move(actualGroup);
}

void RS_Settings::endGroup()  {
    m_group.clear();
}

std::unique_ptr<RS_Settings::GroupGuard> RS_Settings::beginGroupGuard(QString group) {
    auto guard = std::make_unique<GroupGuard>(std::move(m_group));
    m_group = std::move(group);
    return guard;
}

bool RS_Settings::write(const QString& key, const QVariant& variant) {
    return writeEntrySingle(m_group, key, variant);
}

bool RS_Settings::write(const QString &key, const int value) {
    return writeSingle(m_group, key, value);
}
bool RS_Settings::writeColor(const QString &key, const int value) {
    return writeEntry(key, QVariant(value % 0x80000000));
}

bool RS_Settings::writeSingle(const QString& group, const QString &key, const int value) {
    return writeEntrySingle(group, key, QVariant(value));
}

bool RS_Settings::write(const QString &key, const QString &value) {
    return writeSingle(m_group, key, value);
}

bool RS_Settings::writeSingle(const QString& group, const QString &key, const QString &value) {
    return writeEntrySingle(group, key, QVariant(value));
}

bool RS_Settings::write(const QString &key, const double value) {
    return writeSingle(m_group, key, value);
}

bool RS_Settings::writeSingle(const QString & group, const QString &key, const double value) {
    return writeEntrySingle(group, key, QVariant(value));
}
bool RS_Settings::write(const QString &key, const bool value) {
    return writeSingle(m_group, key, value);
}

bool RS_Settings::writeSingle(const QString &group, const QString &key, const bool value) {
    return writeEntrySingle(group, key, QVariant(value));
}

bool RS_Settings::readBool(const QString &key, const bool defaultValue) {
    return readBoolSingle(m_group, key, defaultValue);
}

bool RS_Settings::readBoolSingle(const QString &group, const QString &key, const bool defaultValue) {
    const QString fullName = getFullName(group, key);
    QVariant value = readEntryCache(fullName);
    if (!value.isValid()) {
        value = m_settings->value(fullName, QVariant(defaultValue));
        writeEntryCache(fullName, value);
    }
    const int result = value.toBool();
    return result;
}

bool RS_Settings::writeEntry(const QString &key, const QVariant &value) {
    return writeEntrySingle(m_group, key, QVariant(value));
}

QString RS_Settings::getFullName(const QString &group, const QString &key) const {
    const QString &fullName = QString("/%1/%2").arg(group, key);
    return fullName;
}

void RS_Settings::emitOptionsChanged() {
    emit optionsChanged();
}

QString RS_Settings::readStr(const QString &key,const QString &def) {
    return readStrSingle(m_group, key, def);
}

#define DEBUG_SETTINGS_READ_WRITE_

bool RS_Settings::writeEntrySingle(const QString& group, const QString &key, const QVariant &value) {
    const QString fullName = getFullName(group, key);

    QVariant cachedValue = readEntryCache(fullName);

    const bool cashedValid = cachedValue.isValid();
    // Symmetrical On-Demand Cache Backup on first write
    if (m_inTransaction && m_transactionBackup.count(fullName) == 0) {
        if (!cashedValid) {
            cachedValue = m_settings->value(fullName);
        }
        m_transactionBackup[fullName] = cachedValue; // Caches the pre-dialog state
    }

    if (cashedValid){
        bool tmpCached = cachedValue.toBool();
        bool tmpValue = value.toBool();
        if (cachedValue == value) {
            return false;
        }
    }

#ifdef DEBUG_SETTINGS_READ_WRITE
     LC_ERR << "Write FullName: " << fullName << " | " << value.toString();
#endif
    if (m_inTransaction) {
        // Only write to the in-memory cache, bypassing disk/registry
        writeEntryCache(fullName, value);
    }
    else {
        // Standard non-transactional write (writes to disk/registry immediately)
        m_settings->setValue(fullName, value);
        writeEntryCache(fullName, value);
    }

    // basically, that's a shortcut that we put value from cache as old value (instead of actual reading of it).
    // however, in most cases, properties will be read before modification, so that's fine
    emit optionChanged(group, key, cachedValue, value);  // fixme - sand - review. What if we're in transaction?

    return true;
}

QString RS_Settings::readStrSingle(const QString& group, const QString &key,const QString &def) {
    const QString fullName = getFullName(group, key);
    QVariant value = readEntryCache(fullName);
    if (!value.isValid()) {
        value = m_settings->value(fullName, QVariant(def))/*.toString()*/;
        writeEntryCache(fullName, value);
    }
    auto result = value.toString();
#ifdef DEBUG_SETTINGS_READ_WRITE
    LC_ERR << "Read FullName: " << fullName << " | " << value.toString();
#endif
    return result;
}

int RS_Settings::readColor(const QString &key, const int def) {
    return readColorSingle(m_group, key, def);
}

QStringList RS_Settings::getAllKeys() const {
    return m_settings->allKeys();
}

QStringList RS_Settings::getChildKeys() const {
    const QString currentGroup = m_settings->group();
    m_settings->beginGroup(m_group);
    auto result = m_settings->childKeys();
    m_settings->endGroup();
    m_settings->beginGroup(currentGroup);
    return result;
}

void RS_Settings::remove(const QString& key) const {
    const QString fullName = getFullName(m_group, key);
    m_settings->remove(fullName);
}

int RS_Settings::readColorSingle(const QString& group, const QString &key, const int def) {
    const QString fullName = getFullName(group, key);
    QVariant value = readEntryCache(fullName);
    if (!value.isValid()) {
        value = m_settings->value(fullName, QVariant(def));
        writeEntryCache(fullName, value);
    }
    unsigned long long uValue = value.toULongLong();
    uValue = uValue % 0x80000000ULL;
    const int result = static_cast<int>(uValue);
    return result;
}

int RS_Settings::readInt(const QString &key, const int def) {
    return readIntSingle(m_group, key, def);
}

int RS_Settings::readIntSingle(const QString& group, const QString &key, const int def) {
    const QString fullName = getFullName(group, key);
    QVariant value = readEntryCache(fullName);
    if (!value.isValid()) {
        value = m_settings->value(fullName, QVariant(def));
        writeEntryCache(fullName, value);
    }
    const int result = value.toInt();
    return result;
}

QByteArray RS_Settings::readByteArray(const QString &key) const {
    return readByteArraySingle(m_group, key);
}

QByteArray RS_Settings::readByteArraySingle(const QString& group, const QString &key) const {
    const QString fullName = getFullName(group, key);
    return m_settings->value(fullName, "").toByteArray();
}


void RS_Settings::clearAll() {
    m_settings->clear();
    m_cache.clear();
    saveIsAllowed = false;
}

void RS_Settings::clearGeometry() {
    m_settings->remove("/Geometry");
    m_cache.clear();
    saveIsAllowed = false;
}

void RS_Settings::writePen(const QString& name, const RS_Pen&pen){
    LC_SET("pen" + name + "Color", pen.getColor().name());
    LC_SET("pen" + name + "LineType", pen.getLineType());
    LC_SET("pen" + name + "Width", pen.getWidth());
}

RS_Pen RS_Settings::readPen(const QString& name, const RS_Pen &defaultPen){
    const auto color = RS_Color(LC_GET_STR("pen" + name + "Color", defaultPen.getColor().name()));
    // FIXME - well, that's a bit ugly - if there is a mess in setting value, cast from int to short will be illegal. Need additional validation there?
    const auto lineType = static_cast<RS2::LineType> (LC_GET_INT("pen" + name + "LineType", defaultPen.getLineType()));
    const auto lineWidth = static_cast<RS2::LineWidth> (LC_GET_INT("pen" + name + "Width", defaultPen.getWidth()));

    auto result = RS_Pen(color, lineWidth, lineType);
    return result;
}

void RS_Settings::startTransaction() {
    m_transactionBackup.clear();
    m_inTransaction = true;
}

void RS_Settings::commitTransaction() {
    if (!m_inTransaction) return;
    m_inTransaction = false;

    // Bulk write only the modified keys permanently to QSettings
    for (auto it = m_transactionBackup.begin(); it != m_transactionBackup.end(); ++it) {
        QString fullName = it.key();
        m_settings->setValue(fullName, m_cache[fullName]);
    }
    m_transactionBackup.clear();
    emitOptionsChanged();
}

void RS_Settings::rollbackTransaction() {
    if (!m_inTransaction) return;
    m_inTransaction = false;

    // Restore the in-memory cache to the pre-dialog baseline
    for (auto it = m_transactionBackup.begin(); it != m_transactionBackup.end(); ++it) {
        QString fullName = it.key();
        QVariant originalVal = it.value();

        writeEntryCache(fullName, originalVal); // Restore in-memory cache

        QStringList parts = fullName.split('/', Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            emit optionChanged(parts[0], parts[1], QVariant(), originalVal);
        }
    }
    m_transactionBackup.clear();
    emitOptionsChanged();
}
