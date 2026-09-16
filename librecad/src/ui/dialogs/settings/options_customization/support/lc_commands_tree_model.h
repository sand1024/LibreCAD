
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

#ifndef LC_COMMANDS_TREE_MODEL_H
#define LC_COMMANDS_TREE_MODEL_H

#include <qaction.h>
#include <QMultiMap>

#include "lc_action_mapping_tree_model_base.h"
#include "lc_command_types.h"
#include "rs.h"

class LC_ActionGroupManager;
class LC_ActionTypeMapper;
class LC_CommandsTreeItem;

class LC_CommandsTreeModel : public LC_ActionMappingTreeModelBase {
    Q_OBJECT
public:
    enum Column {
       NAME = 0,
       KEYCODE,
       COMMAND,
       ALIAS,
       LAST
    };

    explicit LC_CommandsTreeModel(QObject* parent = nullptr,
                                  const QColor& filterColor = QColor(189, 99, 19),
                                  const QColor& conflictColor = QColor(218, 54, 51));
    ~LC_CommandsTreeModel() override = default;

    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    LC_CommandsTreeItem* getItemForIndex(const QModelIndex& index) const;

    void rebuildModel(const CommandsConfig& config, LC_ActionGroupManager* agm, LC_ActionTypeMapper* mapper);
    void updateItemTriggers(LC_CommandsTreeItem* item, const QString& mainCmd, const QString& keyCode, const QString& alias,
                                const QStringList& effectiveTriggers, bool isModified);

    void recalculateCollisions(const CommandsConfig& config,  LC_ActionGroupManager* agm, const LC_ActionTypeMapper* mapper);

    bool checkForCollisions(const CommandsConfig& config,
                             LC_ActionGroupManager* agm,
                            const LC_ActionTypeMapper* mapper,
                            QString* outFirstConflict = nullptr);

    static QStringList computeActionEffectiveTriggers(RS2::ActionType actionType, const CommandDefinition& def);
    static QStringList computeKeywordEffectiveTriggers(const QString& key, const KeywordDefinition& def);

private:
    void buildGlobalTriggerMaps(const CommandsConfig& config, LC_ActionGroupManager* agm, const LC_ActionTypeMapper* mapper,
                                const QMap<QString, CommandDefinition>& cmdMap, QMultiMap<QString, QString>& outActionTriggerMap,
                                QMultiMap<QString, QString>& outKeywordTriggerMap) const;
    bool matchesSearchFilter(const QStringList& searchableStrings) const;

    LC_CommandsTreeItem* createActionTreeItem(LC_CommandsTreeItem* parentGroup, const QAction* action, const CommandDefinition& cmdDef,
                                              const LC_ActionTypeMapper* mapper, const QMultiMap<QString, QString>& actionTriggerMap);

    void buildActionGroups(const QMap<QString, CommandDefinition>& cmdMap, LC_ActionGroupManager* agm, const LC_ActionTypeMapper* mapper,
                           const QMultiMap<QString, QString>& actionTriggerMap, LC_CommandsTreeItem* rootItem);

    LC_CommandsTreeItem* createKeywordTreeItem(LC_CommandsTreeItem* parentGroup, const KeywordDefinition& kwDef,
                                           const QMultiMap<QString, QString>& keywordTriggerMap);

    void buildKeywordsGroup(const CommandsConfig& config, const QMultiMap<QString, QString>& keywordTriggerMap,
                            LC_CommandsTreeItem* rootItem);

    void updateItemTriggers(LC_CommandsTreeItem* item, const QString& mainCmd, const QString& a1, const QString& a2, bool isModified);
    bool evaluateKeywordCollision(LC_CommandsTreeItem* item, const KeywordDefinition& kwDef) const;
    void recalculateGroupCollisions(LC_CommandsTreeItem* groupItem, int groupRow, const QMap<QString, CommandDefinition>& cmdMap,
                                    const QMap<QString, KeywordDefinition>& kwMap, const LC_ActionTypeMapper* mapper,
                                    const QMultiMap<QString, QString>& actionTriggers);



    bool evaluateActionCollision(LC_CommandsTreeItem* item, const CommandDefinition& cmdDef, const LC_ActionTypeMapper* mapper,
                                 const QMultiMap<QString, QString>& actionTriggers) const;

};

#endif
