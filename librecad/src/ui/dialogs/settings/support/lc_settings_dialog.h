
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

#ifndef LC_SETTINGS_DIALOG_H
#define LC_SETTINGS_DIALOG_H

#include <QDialog>
#include <QTreeView>
#include <QLineEdit>
#include <QStackedWidget>
#include <QLabel>
#include <QScrollArea>
#include <QDialogButtonBox>
#include <QStandardItemModel>
#include <QMap>
#include <vector>
#include <map>
#include <memory>
#include <qnetworkreply.h>

#include "lc_dialog.h"
#include "lc_preset_manager_interface.h"
#include "lc_settings_page_interface.h"
#include "lc_settings_filter_model.h"

class QSplitter;
class LC_PresetManagementBar;
class QVBoxLayout;

namespace Ui {
    class LC_SettingsDialog;
}

class LC_SettingsDialog : public LC_Dialog {
    Q_OBJECT
public:
    explicit LC_SettingsDialog(QWidget* parent, const QString& dialogId);
    ~LC_SettingsDialog() override;
    void registerPage(std::unique_ptr<LC_SettingsPageInterface> pageRef);
    void registerPresetManager(const QString& groupPathId, std::unique_ptr<LC_PresetManagerInterface> manager);
    bool selectPage(const QString& pageId);
    void finalizeInitialization();
    QList<QPair<QString, QString>> gatherChildLinks(const QString& parentPageId) const;
    LC_PresetManagerInterface* getPresetManager(const QString& groupPathId) const;
    void forEachPresetManager(const std::function<void(LC_PresetManagerInterface*)>& callback) const;
    void accept() override;
    void reject() override;
signals:
   void restartRequired();

protected:
    void closeEvent(QCloseEvent* event) override;
    void saveInnerDialogData(LC_SettingsGroupDialog& group, bool savePositions) const override;
    void loadInnerDialogData(LC_SettingsGroupDialog& group, bool savePositions) override;
    void onPresetSelected(const QString& key);
    void doUpdatePageLivePreview(LC_SettingsPageInterface* page) const;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    void updateGatingState();

    bool isPageInitialized(const QString& pageId) const {
        return m_initializedPages.contains(pageId);
    }

    bool isPageInitialized(LC_SettingsPageInterface* page) const {
        return m_initializedPages.contains(page->id());
    }

private slots:
    void onSearchTextChanged(const QString& text);
    void onSearchReturnPressed();
    QModelIndex findFirstVisibleIndex(const QModelIndex& parent = QModelIndex()) const; 
    void onCategorySelected(const QModelIndex& index);
    void onNavigateBack();
    void onNavigateForward();

private:
    void buildCategoryTree();
    void updateBreadcrumbs(const QModelIndex& index) const; // Updated to use ModelIndex [4.2]
    void highlightPageContent(LC_SettingsPageInterface* page, const QString& filterText) const;
    LC_PresetManagerInterface* getPresetManagerForPage(const QString& pageId) const;

    void updateHistoryButtons() const;
    void navigateToHistoryIndex(int index);
    void restoreTreeExpandedState() const;
    bool isPresetManagerScopeDirty(LC_PresetManagerInterface* manager) const; 

    QStandardItemModel* m_treeModel = nullptr;
    LC_SettingsFilterModel* m_filterModel = nullptr;

    std::vector<std::unique_ptr<LC_SettingsPageInterface>> m_pages;
    std::map<QString, LC_SettingsPageInterface*> m_pageMapByPageId;
    std::map<QString, QStandardItem*> m_treeItemMapByPageId;
    QMap<QString, QStringList> m_searchIndex;

    std::map<QString, std::unique_ptr<LC_PresetManagerInterface>> m_presetManagers;

    std::vector<QString> m_history;
    int m_historyIndex = -1;
    bool m_isNavigatingHistory = false;

    QSet<QString> m_initializedPages;
    LC_SettingsPageInterface* m_activePage = nullptr;

    std::unique_ptr<Ui::LC_SettingsDialog> ui;

    QString m_dialogId;
};
#endif
