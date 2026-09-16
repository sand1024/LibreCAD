
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

#ifndef LC_MENU_TRIGGER_INSPECTOR_H
#define LC_MENU_TRIGGER_INSPECTOR_H

#include <QWidget>
#include <QElapsedTimer>
#include <memory>
#include "lc_menu_activator.h"

namespace Ui {
    class LC_MenuTriggerInspector;
}

class LC_MenuTriggerInspector : public QWidget {
    Q_OBJECT
public:
    explicit LC_MenuTriggerInspector(QWidget* parent = nullptr);
    ~LC_MenuTriggerInspector() override;

    void setActivator(const QString& menuName, const QString& shortcutCode);
    QString getActivatorCode() const;
    void setExistingActivators(const QList<LC_MenuActivator*>& list);
    void setReadOnly(bool readOnly);

signals:
        void triggerChanged();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onManualControlToggled();
    void onEntityContextChanged(int index);
    void onUnassignClicked();

private:
    void initEntityCombobox();
    void syncUiFromActivator();
    void updateActivatorFromUi();
    void validateTrigger();

    std::unique_ptr<Ui::LC_MenuTriggerInspector> ui;
    std::unique_ptr<LC_MenuActivator> m_currentActivator;
    QList<LC_MenuActivator*> m_existingActivators;
    QElapsedTimer m_clickTimer;
    bool m_blockSignals = false;
    bool m_isReadOnly{false};
};

#endif
