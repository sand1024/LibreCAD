
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

#ifndef LC_DLG_CHECKLIST_SELECTION_H
#define LC_DLG_CHECKLIST_SELECTION_H

#include <QDialog>
#include <QList>
#include <QString>
#include <vector>

class QRadioButton;
class QGroupBox;
class QCheckBox;
class QDialogButtonBox;
class QLabel;
class QLineEdit;

struct LC_ChecklistChoice {
    QString id;
    QString label;
    QString tooltip;
    bool checked{true};
    bool enabled{true};
};

class LC_DlgChecklistSelection : public QDialog {
    Q_OBJECT
public:
    explicit LC_DlgChecklistSelection(QWidget* parent,
                                      const QString& title,
                                      const QString& message,
                                      QList<LC_ChecklistChoice>& choices);
    ~LC_DlgChecklistSelection() override = default;

    void setHeaderInput(const QString& label, const QString& defaultValue, const QString& tooltip = QString());
    QString headerInputText() const;

    static bool selectChoices(QWidget* parent,
                              const QString& title,
                              const QString& message,
                              QList<LC_ChecklistChoice>& choices);

    static bool selectChoicesWithInput(QWidget* parent,
                                       const QString& title,
                                       const QString& message,
                                       const QString& inputLabel,
                                       const QString& defaultInputText,
                                       QString& outInputText,
                                       QList<LC_ChecklistChoice>& choices);

    void setScopeSelection(const QString& groupTitle,
                         const QString& fullScopeText,
                         const QString& activeOnlyText,
                         bool defaultFullScope = true,
                         const QString& fullScopeTooltip = QString(),
                         const QString& activeOnlyTooltip = QString());

    bool isFullScopeSelected() const;

    static bool selectChoicesWithInputAndScope(QWidget* parent,
                                            const QString& title,
                                            const QString& message,
                                            const QString& inputLabel,
                                            const QString& defaultInputText,
                                            const QString& scopeTitle,
                                            const QString& fullScopeText,
                                            const QString& activeOnlyText,
                                            bool& outIsFullScope,
                                            QString& outInputText,
                                            QList<LC_ChecklistChoice>& choices);

private slots:
    void onSelectAllClicked();
    void onDeselectAllClicked();
    void onChoiceToggled();
    void onInputTextChanged(const QString& text);

private:
    void updateOkButtonState();

    QList<LC_ChecklistChoice>& m_choices;
    std::vector<QCheckBox*> m_checkBoxes;
    QDialogButtonBox* m_buttonBox{nullptr};
    QWidget* m_inputContainer{nullptr};
    QLabel* m_lblInput{nullptr};
    QLineEdit* m_leInput{nullptr};

    QGroupBox* m_gbScope{nullptr};
    QRadioButton* m_rbFullScope{nullptr};
    QRadioButton* m_rbActiveOnly{nullptr};
};

#endif
