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

#ifndef RS_SELECTION_H
#define RS_SELECTION_H
#include <QList>
#include <QSet>

#include "lc_selectionoptions.h"
#include "lc_selectionpredicate.h"
#include "rs_graphicview.h"
#include "rs_layer.h"

class RS_Document;

namespace RS2
{
    enum EntityType : unsigned;
}

class QString;

class LC_GraphicViewport;
class RS_EntityContainer;
class RS_Graphic;
class RS_Vector;
class RS_Entity;

/**
 * API Class for selecting entities. 
 * There's no interaction handled in this class.
 * This class is connected to an entity container and
 * can be connected to a graphic view.
 *
 * @author Andrew Mustun
 */
class RS_Selection {
public:
    using FunEntityMatch  = std::function<bool(RS_Entity*)>;

    struct CurrentSelectionState {
        QSet<RS2::EntityType> documentEntityTypes; // fixme - rework to more efficient structure (like flags)
        QSet<RS2::EntityType> selectedEntityTypes;

        bool hasSelection() const {
            return !selectedEntityTypes.empty();
        }
    };

    explicit RS_Selection(RS_Document* entityContainer,
                 LC_GraphicViewport* graphicView=nullptr);
    explicit RS_Selection(RS_GraphicView* graphicView);
    static void unselectAllInDocument(RS_Document* document, LC_GraphicViewport* vp);
    static void selectEntitiesList(RS_Document* document, LC_GraphicViewport* vp, const QList<RS_Entity*>& entities, bool select);
    static void selectEntitiesVector(RS_Document* document, LC_GraphicViewport* vp, const std::vector<RS_Entity*>& entities, bool select);
    static void unselectLayer(RS_Document* document, LC_GraphicViewport* vp, RS_Layer* layer);
    void selectSingle(RS_Entity* e) const;
    void selectEntitiesList(const QList<RS_Entity*> &entities, bool select) const;
    void selectIfMatched(const QList<RS_Entity*> &entities, bool select, FunEntityMatch matchFun) const;
    void selectAll(bool select=true);
    void deselectAll() {selectAll(false);}
    void invertSelection();
    void selectWindow(enum RS2::EntityType typeToSelect, const RS_Vector& v1, const RS_Vector& v2,bool select=true, bool cross=false);
    void selectWindow(const QList<RS2::EntityType> &typesToSelect, const RS_Vector& v1, const RS_Vector& v2,bool select=true, bool cross=false);
    void selectIntersected(RS_Entity* entity, bool select);
    void deselectWindow(enum RS2::EntityType typeToSelect,const RS_Vector& v1, const RS_Vector& v2) {selectWindow(typeToSelect,v1, v2, false);}
    void selectIntersected(const RS_Vector& v1, const RS_Vector& v2,bool select=true);
    void deselectIntersected(const RS_Vector& v1, const RS_Vector& v2) {selectIntersected(v1, v2, false);}
    void selectContour(RS_Entity* e);
    void selectLayer(RS_Entity* e);
    void selectLayer(const RS_Layer* layer, bool select);
    void conditionalSelection(RS_Document* doc,const LC_SelectionOptions &options, const LC_SelectionPredicate& predicate, std::list<RS_Entity>& selectedEntities);
    void countSelectedEntities(QMap<RS2::EntityType, int> &entityTypeMaps) const;
    void collectCurrentSelectionState(CurrentSelectionState& selectionState) const;
    void performBulkSelection(std::function<void(RS_EntityContainer*, LC_GraphicViewport*, RS_Document*)> fun) const;
protected:
    RS_Document* m_document = nullptr;
    LC_GraphicViewport* m_viewPort = nullptr;
    bool m_additiveSelection = true;

    void doSelectEntitiesWithTypeInWindow(RS_EntityContainer* container, RS_Document* doc, RS2::EntityType typeToSelect, RS_Vector v1, RS_Vector v2, bool select, bool cross);
    void doSelectEntitiesWithTypesInWindow(RS_EntityContainer* container, RS_Document* doc, const QList<RS2::EntityType>& typesToSelect, RS_Vector v1, RS_Vector v2, bool select, bool cross);
    void doSelectIntersectedContainer(RS_Entity* entity, RS_Document* doc, bool select) const;
    void doSelectIntersectedAtomic(RS_Entity* entity, RS_Document* doc, bool select) const;
    void doUnselectAll(RS_EntityContainer* container, RS_Document* doc) const;
    void doUnselectAllIfNeeded(RS_EntityContainer* container, RS_Document* doc) const;
};

#endif
