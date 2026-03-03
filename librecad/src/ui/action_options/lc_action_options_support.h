/*
 * ********************************************************************************
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
 * ********************************************************************************
 */

#ifndef LC_ACTIONOPTIONSSUPPORT_H
#define LC_ACTIONOPTIONSSUPPORT_H
#include "rs.h"
#include <QString>

#include "lc_action_options_editor.h"

class RS_ActionInterface;

class LC_ActionOptionsSupport {
public:
    virtual ~LC_ActionOptionsSupport() = default;
    void setAction(RS_ActionInterface * a, bool update = false);
    virtual void hideOptions() = 0;
    /**
     * Extension point. Method allows action to request update of UI (say, by enabling or hiding some parts of the widget).
     * Mode value is action-specific and should be processed by related option widget.
     * @param mode
     * @param value
     * @param value
     */
    virtual void updateUI([[maybe_unused]]int mode, const QVariant* value){}
protected:
    /**
 * Default workflow for saving settings values
 */
    virtual void saveSettings();
    /**
     * Extension point for actuall saving of settings
     */
    virtual void doSaveSettings(){}
    /**
     * Setter for corresponding action
     * @param a action
     * @param update true if options widget should be updated by action values, false - loading from settings
     */
    virtual void doSetAction(RS_ActionInterface* a, bool update) = 0;
    /**
     * Performs check that provided action type is accepted by options widget
     * @param actionType type of action
     * @return true if type is ok, false otherwise
     */
    virtual bool checkActionRttiValid(RS2::ActionType actionType);
    virtual void preSetupByAction(RS_ActionInterface* a) = 0;
    virtual void cleanup() = 0;

    /**
    * Default name for settings group name
    * @return name of group
    */
    virtual QString getSettingsGroupName(){return "Draw";}
    /**
     * Default name for prefix for settings. It assumes that all settings for the action starts with the same prefix.
     * @return  prefix to use.
     */
    virtual QString getSettingsOptionNamePrefix(){ return "";}

    // saving settings shortcut methods
    void save(const QString& name, const QString& value);
    void save(const QString& name, int value);
    void save(const QString& name, bool value);

    // loading settings shortcut methods
    QString load(const QString& name, const QString& defaultValue);
    int loadInt(const QString& name, int defaultValue);
    bool loadBool(const QString& name, bool defaultValue);

    // conversion utilities
    QString fromDouble(double value);
    bool toDouble(const QString &strValue, double &res, double notMeaningful, bool positiveOnly);
    bool toDoubleAngleDegrees(const QString &strValue, double &res, double notMeaningful, bool positiveOnly);
};

#endif
