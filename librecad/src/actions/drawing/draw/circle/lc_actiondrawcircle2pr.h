/****************************************************************************
**
 * Draw circle, given its radius and two points on circle

Copyright (C) 2014-2015 Dongxu Li (dongxuli2011 at gmail.com)

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
**********************************************************************/
#ifndef LC_ACTIONDRAWCIRCLE2PR_H
#define LC_ACTIONDRAWCIRCLE2PR_H

#include "rs_actiondrawcirclecr.h"

/**
 * This action class can handle user events to draw a
 * circle with radius and two points on the circle
 *
 * @author Dongxu Li
 */
class LC_ActionDrawCircle2PR : public RS_ActionDrawCircleCR {
    Q_OBJECT
public:
    explicit LC_ActionDrawCircle2PR(LC_ActionContext* actionContext);
    ~LC_ActionDrawCircle2PR() override;
    void init(int status) override;
    QStringList getAvailableCommands() override;

protected:
    /**
 * Action States.
 */
    enum Status {
        SetPoint1 = InitialActionStatus, /**< Setting the 1st point. */
        SetPoint2, /**< Setting the 2nd point. */
        SelectCenter /**< select center out of two possibilities. */
    };

    struct Points;
    std::unique_ptr<Points> m_actionData;
    bool preparePreview(const RS_Vector& mouse, RS_Vector& altCenter) const;
    void reset() override;
    void onMouseLeftButtonRelease(int status, const LC_MouseEvent* e) override;
    void onMouseRightButtonRelease(int status, const LC_MouseEvent* e) override;
    bool doProcessCommand(int status, const QString& c) override;
    void onCoordinateEvent(int status, bool isZero, const RS_Vector& coord) override;
    void updateMouseButtonHints() override;
    void doTriggerCompletion(bool success) override;
    RS_Entity* doTriggerCreateEntity() override;
    void onMouseMoveEvent(int status, const LC_MouseEvent* e) override;
};
#endif
