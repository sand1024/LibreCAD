/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2018 A. Stebich (librecad@mail.lordofbikes.de)
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


#ifndef RS_DOCUMENT_H
#define RS_DOCUMENT_H

#include "rs_entitycontainer.h"
#include "rs_pen.h"
#include "rs_undo.h"
#include "selection/lc_selectedset.h"

class LC_TextStyleList;
class LC_DimStylesList;
class RS_GraphicView;
class LC_UCSList;
class LC_ViewList;
class RS_BlockList;
class RS_LayerList;

struct LC_DocumentModificationBatch {
    bool success {false};
    QList<RS_Entity*> entitiesToAdd;
    QList<RS_Entity*> entitiesToDelete;

    bool m_setActiveLayer{true};
    bool m_setActivePen{true};

    ~LC_DocumentModificationBatch() = default;

    void dontSetActiveLayerAndPen(){m_setActiveLayer = false; m_setActivePen = false;}
    void setActiveLayerAndPen(bool setLayer, bool setPen){m_setActiveLayer = setLayer; m_setActivePen = setPen;}


    void add(RS_Entity* entity) {entitiesToAdd.append(entity);}
    void remove(QList<RS_Entity*>& list) {entitiesToDelete.append(list);}
    void remove(RS_Entity* e) {entitiesToDelete.append(e);}
    void replace(RS_Entity* original, RS_Entity* clone) {entitiesToDelete.append(original);entitiesToAdd.append(clone); }
    void clear() {entitiesToAdd.clear(); entitiesToDelete.clear();};
};

inline void operator +=(LC_DocumentModificationBatch &ctx, RS_Entity* e) {ctx.entitiesToAdd.append(e);}
inline void operator +=(LC_DocumentModificationBatch &ctx, QList<RS_Entity*> &list) {ctx.entitiesToAdd.append(list);}
inline void operator +=(LC_DocumentModificationBatch &ctx, const QList<RS_Entity*> &list) {ctx.entitiesToAdd.append(list);}
inline void operator -=(LC_DocumentModificationBatch &ctx, RS_Entity* e) {ctx.entitiesToDelete.append(e);}
inline void operator -=(LC_DocumentModificationBatch &ctx, QList<RS_Entity*> &list) {ctx.entitiesToDelete.append(list);}
inline void operator -=(LC_DocumentModificationBatch &ctx, const QList<RS_Entity*> &list) {ctx.entitiesToDelete.append(list);};


/**
 * Base class for documents. Documents can be either graphics or
 * blocks and are typically shown in graphic views. Documents hold
 * an active pen for drawing in the Document, a file name and they
 * know whether they have been modified or not.
 *
 * @author Andrew Mustun
 */
class RS_Document : public RS_EntityContainer, public RS_Undo {
public:
    explicit RS_Document(RS_EntityContainer* parent=nullptr);
    ~RS_Document() override = default;

    virtual RS_LayerList* getLayerList()= 0;
    virtual RS_BlockList* getBlockList() = 0;
    virtual LC_DimStylesList* getDimStyleList() = 0;
    virtual LC_ViewList* getViewList() { return nullptr;}
    virtual LC_UCSList* getUCSList() { return nullptr;}
    virtual LC_TextStyleList* getTextStyleList() = 0;

    virtual void newDoc() = 0;

    /**
     * @return true for all document entities (e.g. Graphics or Blocks).
     */
    bool isDocument() const override {return true;}

    void addEntity(RS_Entity* entity) override;

    void undoableAdd(RS_Entity* entity, bool undoable = true) {
        addEntity(entity);
        if (undoable) {
           addUndoable(entity);
        }
    }

    void undoableDelete(RS_Entity* e) {
        if (e->getFlag(RS2::FlagSelected)) {
            unselect(e);
        }
        e->setFlag(RS2::FlagDeleted);
        addUndoable(e);
    }

    void select(RS_Entity* entity, bool select = true) {
        entity->doSelectInDocument(select, this);
    }

    void unselect(RS_Entity* entity) {
        entity->doSelectInDocument(false, this);
    }

    /**
     * @return Currently active drawing pen.
     */
    RS_Pen getActivePen() const {return activePen;}

    /**
     * Sets the currently active drawing pen to p.
     */
    void setActivePen(const RS_Pen& p) {activePen = p;}
/**
 * Sets the documents modified status to 'm'.
 */
    virtual void setModified(bool m) {
        modified = m;
    }

	/**
	 * @retval true The document has been modified since it was last saved.
	 * @retval false The document has not been modified since it was last saved.
	 */
    virtual bool isModified() const {return modified;}

    /**
     * Overwritten to set modified flag when undo cycle finished with undoable(s).
     */
    void endUndoCycle() override;
    void startUndoCycle() override;
    void setGraphicView(RS_GraphicView * g) {gv = g;}
    RS_GraphicView* getGraphicView() const {return gv;} // fixme - sand -- REALLY BAD DEPENDANCE TO UI here, REWORK!

    LC_SelectedSet* getSelectedSet() const {return m_selectedSet.get();}

    void modify(LC_DocumentModificationBatch& batch);
protected:
    /** Flag set if the document was modified and not yet saved. */
    bool modified = false;
    /** Active pen. */
    RS_Pen activePen;

    //used to read/save current view
    RS_GraphicView * gv = nullptr; // fixme - sand -- REALLY BAD DEPENDANCE TO UI here, REWORK!

    std::unique_ptr<LC_SelectedSet> m_selectedSet;

    void startBulkUndoablesCleanup() override;
    void endBulkUndoablesCleanup() override;

    /**
     * Removes an entity from the entity container. Implementation
     * from RS_Undo.
     */
    void removeUndoable(RS_Undoable* u) override {
        if (u != nullptr && u->undoRtti()==RS2::UndoableEntity && u->isDeleted()) {
            removeEntity(static_cast<RS_Entity*>(u));
        }
    }

    bool m_inBulkUndoableCleanup = false;
    bool m_savedAutoUpdateBorders = false;

};
#endif
