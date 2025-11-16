/*
 * ********************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2025 LibreCAD.org
 * Copyright (C) 2025 sand1024
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

#ifndef LC_UNDOABLEDOCUMENTMODIFICATIONACTION_H
#define LC_UNDOABLEDOCUMENTMODIFICATIONACTION_H

#include "rs_previewactioninterface.h"


class LC_UndoableDocumentModificationAction: public RS_PreviewActionInterface {
protected:
    LC_UndoableDocumentModificationAction(const char *name,LC_ActionContext *actionContext,RS2::ActionType actionType = RS2::ActionNone):
     RS_PreviewActionInterface(name, actionContext, actionType){};
    ~LC_UndoableDocumentModificationAction() override = default;
    void setupAndUndoableAdd(LC_DocumentModificationBatch& ctx);
    void previewEntitiesToAdd(LC_DocumentModificationBatch& ctx) const;
    virtual bool doTriggerModificationsPrepare(LC_DocumentModificationBatch& ctx)  = 0;
    virtual void doTriggerCompletion(bool success) {};
    virtual void clearDocumentModificationContext(LC_DocumentModificationBatch& ctx);
    virtual void prepareDocumentModificationContext(LC_DocumentModificationBatch& ctx) {}
    void setupAndUndoableAdd(const QList<RS_Entity*>& entitiesToInsert, bool setActiveLayer, bool setActivePen); ;
    bool triggerModifyDocumentContent();
    void doTrigger() override;
    bool isTriggerUndoable() override {return true;}
};


class LC_SingleEntityCreationAction: public LC_UndoableDocumentModificationAction {
protected:
    LC_SingleEntityCreationAction(const char *name,LC_ActionContext *actionContext,RS2::ActionType actionType = RS2::ActionNone):
    LC_UndoableDocumentModificationAction(name, actionContext, actionType){};
    ~LC_SingleEntityCreationAction() override = default;
    bool doTriggerModificationsPrepare(LC_DocumentModificationBatch& modificationData) override;
    virtual RS_Entity* doTriggerCreateEntity() = 0;
};
#endif
