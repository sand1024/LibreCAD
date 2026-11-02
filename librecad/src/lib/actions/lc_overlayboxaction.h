/*******************************************************************************
 *
 This file is part of the LibreCAD project, a 2D CAD program

 Copyright (C) 2025 LibreCAD.org
 Copyright (C) 2025 sand1024

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

#ifndef LC_OVERLAYBOXACTION_H
#define LC_OVERLAYBOXACTION_H

#include <QInputEvent>

#include "lc_lasso_region.h"
#include "lc_overlay_selection_lasso.h"
#include "lc_undoabledocumentmodificationaction.h"
#include "rs_previewactioninterface.h"


struct LC_SelectionBoxOptions;

class LC_OverlayBoxAction:public LC_UndoableDocumentModificationAction{
public:
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void init(const int status) override;
protected:
    enum class MassSelectionMode {
        NONE,
        BOX,
        LASSO
    };

    enum class MassSelectionStyle {
        Auto,
        Window,
        Crossing,
        Fence
    };

    explicit LC_OverlayBoxAction(const QString &name,LC_ActionContext *actionContext,RS2::ActionType actionType = RS2::ActionNone);
    ~LC_OverlayBoxAction() override;

    void initFromSettings() override;
    void drawOverlaySelectionBox(const RS_Vector &corner1, const RS_Vector &corner2, bool replace = false) const;
    void drawOverlaySelectionLasso(bool replace = false) const;
    void addPointToSelectionLasso(const RS_Vector& wcsPosition);

    void setMassSelectionMode(MassSelectionMode mode);
    MassSelectionMode getMassSelectionMode() const {return m_massSelectionMode;}

    void setMassSelectionStyle(MassSelectionStyle style);
    MassSelectionStyle getMassSelectionStyle() const {return m_massSelectionStyle;}

    bool isMassSelecting() const {
        return m_massSelectionMode == MassSelectionMode::BOX ||
               m_massSelectionMode == MassSelectionMode::LASSO;
    }

    virtual bool isAllowCycleMassSelectionStyle() const {
        return true;
    }

    bool handleMassSelectionKeyEvent(QKeyEvent* e, bool isPress);
    void cycleMassSelectionStyle();

    bool resolveCrossingMode(bool naturalCrossing, bool isControl) const;

    void processBoxSelectionMove(const RS_Vector& corner1, const RS_Vector& mouse, const LC_MouseEvent* e);
    void processLassoSelectionMove(const RS_Vector& mouse, const LC_MouseEvent* e);
    void updateSelectionInfoCursor(const RS_Vector& mouse, bool isLasso, bool deselect);

    void endBoxSelectionMode(const LC_MouseEvent* e);
    void endLassoSelectionMode(const LC_MouseEvent* e);
    virtual void invalidateSelectionCorner1() = 0;
    void finishBoxSelection(const LC_MouseEvent* e);
    void finishLassoSelection(const LC_MouseEvent* e);

    virtual RS_Vector getSelectionCorner1() = 0;
    virtual QList<RS2::EntityType> getEntityTypesToSelect() {return QList<RS2::EntityType>();};

    virtual void doUpdateActionPromptForSelectionStart() = 0;
    void updateActionPromptForSelectionMode();
    void refreshMassSelectionState(const QInputEvent* e);


    virtual bool isPerformSelect(bool isShift) const {
        return !isShift;
    }

    // Convenience overloads:
    bool isPerformSelect(const LC_MouseEvent* e) const {
        return isPerformSelect(e != nullptr && e->isShift);
    }

    bool isPerformSelect(const QInputEvent* e) const {
        return isPerformSelect(e != nullptr && (e->modifiers() & Qt::ShiftModifier) != 0);
    }

    std::unique_ptr<LC_SelectionBoxOptions> m_overlayBoxOptions;
    bool m_allowsLassoSelectionByPressedMouseButton = true;
    bool m_selectIntersecting = false;
    LC_LassoRegion m_lassoRegion;
    bool m_lmbPressed = false;

private:
    MassSelectionMode m_massSelectionMode = MassSelectionMode::NONE;
    MassSelectionStyle m_massSelectionStyle = MassSelectionStyle::Auto;
};

#endif
