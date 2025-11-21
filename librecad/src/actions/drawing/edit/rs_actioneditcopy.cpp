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

#include "rs_actioneditcopy.h"

#include "lc_copyutils.h"
#include "rs_clipboard.h"
#include "rs_graphic.h"
#include "rs_graphicview.h"
#include "rs_preview.h"
#include "rs_units.h"

/**
 * Constructor.
 *
 * @param undo true for undo and false for redo.
 */

RS_ActionEditCopyPaste::RS_ActionEditCopyPaste(LC_ActionContext *actionContext, RS2::ActionType actionType)
    :LC_ActionPreSelectionAwareBase("Edit Copy", actionContext, actionType)
    , m_referencePoint{new RS_Vector{}}{
}

RS_ActionEditCopyPaste::~RS_ActionEditCopyPaste() = default;

void RS_ActionEditCopyPaste::init(int status) {
    LC_ActionPreSelectionAwareBase::init(status);
    *m_referencePoint = RS_Vector(false);
    if (m_actionType  == RS2::ActionEditPaste) {
        m_selectionComplete  = true;
    }
    if (m_selectionComplete) {
        updateMouseButtonHints();
    }
}

void RS_ActionEditCopyPaste::onSelectionCompleted([[maybe_unused]] bool singleEntity, bool fromInit) {
    switch (m_actionType) {
        case RS2::ActionEditCutQuick:
        case RS2::ActionEditCopyQuick: {
            LC_ActionPreSelectionAwareBase::onSelectionCompleted(singleEntity, fromInit);
            break;
        }
        default: {
            setSelectionComplete(isAllowTriggerOnEmptySelection(), fromInit);
            updateMouseButtonHints();
            updateSelectionWidget();
        }
    }
}

bool RS_ActionEditCopyPaste::doTriggerModificationsPrepare(LC_DocumentModificationBatch& ctx) {
    switch (m_actionType){
        case  RS2::ActionEditCut:
        case  RS2::ActionEditCutQuick:
        case  RS2::ActionEditCopy:
        case  RS2::ActionEditCopyQuick:{
            QList<RS_Entity*> selection;
            m_document->getSelectedSet()->collectSelectedEntities(selection);
            if (!selection.empty()) {
                bool cut = m_actionType ==  RS2::ActionEditCut || m_actionType == RS2::ActionEditCutQuick;
                LC_CopyUtils::copy(*m_referencePoint, selection, m_graphic);
                unselectAll();
                if (cut) {
                    ctx-= selection;
                }

                if (m_invokedWithControl){
                    m_actionType = RS2::ActionEditPaste;
                    m_invokedWithControl = false;
                }
                else{
                    finish(false);
                }
            }
            break;
        }
        case RS2::ActionEditPaste: {
            LC_CopyUtils::RS_PasteData pasteData(*m_referencePoint);
            LC_CopyUtils::paste(pasteData, m_graphic, ctx);
            ctx.dontSetActiveLayerAndPen();
            if (!m_invokedWithControl) {
                finish(false);
            }
            break;
        }
        default:
            break;
    }
    return true;
}

void RS_ActionEditCopyPaste::doTriggerSelectionUpdate(bool keepSelected, const LC_DocumentModificationBatch& ctx) {
    LC_ActionPreSelectionAwareBase::doTriggerSelectionUpdate(keepSelected, ctx); // fixme - complete!
}

void RS_ActionEditCopyPaste::doTriggerCompletion(bool success) {
    LC_ActionPreSelectionAwareBase::doTriggerCompletion(success); // fixme - complete!
}

void RS_ActionEditCopyPaste::onMouseMoveEventSelected(int status, LC_MouseEvent *e) {
    if (status==SetReferencePoint) {
         switch (m_actionType) {
            case RS2::ActionEditCut:
            case RS2::ActionEditCopy:{
                break;
            }
            case RS2::ActionEditPaste:{
                *m_referencePoint = e->snapPoint;
                m_preview->addAllFrom(*RS_CLIPBOARD->getGraphic(), m_viewport);
                m_preview->move(*m_referencePoint);

                if (m_graphic) {
                    RS2::Unit sourceUnit = RS_CLIPBOARD->getGraphic()->getUnit();
                    RS2::Unit targetUnit = m_graphic->getUnit();
                    double const f = RS_Units::convert(1.0, sourceUnit, targetUnit);
                    m_preview->scale(*m_referencePoint, {f, f});
                }
                break;
            }
            default:
                break;
        }
    }
    else {
        deleteSnapper();
    }
}

void RS_ActionEditCopyPaste::onMouseLeftButtonReleaseSelected([[maybe_unused]]int status, LC_MouseEvent *e) {
    m_invokedWithControl = e->isControl;
    fireCoordinateEventForSnap(e);
}

void RS_ActionEditCopyPaste::onMouseRightButtonReleaseSelected(int status, [[maybe_unused]]LC_MouseEvent *e) {
    initPrevious(status);
}

void RS_ActionEditCopyPaste::onCoordinateEvent( [[maybe_unused]]int status, [[maybe_unused]]bool isZero, const RS_Vector &pos) {
    *m_referencePoint = pos;
    trigger();
}

void RS_ActionEditCopyPaste::updateMouseButtonHintsForSelection() {
   switch (m_actionType) {
       case RS2::ActionEditCut: {
           updateMouseWidgetTRCancel(tr("Select to cut") + getSelectionCompletionHintMsg(),
                                     MOD_SHIFT_AND_CTRL(tr("Select contour"), tr("Set point after selection")));
           break;
       }
       case RS2::ActionEditCutQuick: {
           updateMouseWidgetTRCancel(tr("Select to cut") + getSelectionCompletionHintMsg(),
                                     MOD_SHIFT_AND_CTRL(tr("Select contour"), tr("Cut right after selection")));
           break;
       }
       case RS2::ActionEditCopy: {
           updateMouseWidgetTRCancel(tr("Select to copy") + getSelectionCompletionHintMsg(),
                                     MOD_SHIFT_AND_CTRL(tr("Select contour"), tr("Set point after selection")));
           break;
       }
       case RS2::ActionEditCopyQuick: {
           updateMouseWidgetTRCancel(tr("Select to cut") + getSelectionCompletionHintMsg(),
                                     MOD_SHIFT_AND_CTRL(tr("Select contour"), tr("Copy right after selection")));
           break;
       }
       default:
           break;
   }
}

void RS_ActionEditCopyPaste::updateMouseButtonHintsForSelected(int status) {
    switch (status) {
        case SetReferencePoint: {
            switch (m_actionType) {
                case RS2::ActionEditCut:
                case RS2::ActionEditCopy: {
                    updateMouseWidgetTRCancel(tr("Specify reference point"), MOD_CTRL(tr("Paste Immediately")));
                    break;
                }
                case RS2::ActionEditPaste: {
                    updateMouseWidgetTRCancel(tr("Set paste reference point"), MOD_CTRL(tr("Paste Multiple")));
                    break;
                }
                default:
                    break;
            }
            break;
        default:
            updateMouseWidget();
            break;
        }
    }
}

RS2::CursorType RS_ActionEditCopyPaste::doGetMouseCursorSelected([[maybe_unused]]int status){
    return RS2::CadCursor;
}
