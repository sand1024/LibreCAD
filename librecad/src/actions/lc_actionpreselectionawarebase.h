#ifndef LC_ACTIONPRESELECTIONAWAREBASE_H
#define LC_ACTIONPRESELECTIONAWAREBASE_H

#include "rs_actionselectbase.h"

class LC_ActionPreSelectionAwareBase:public RS_ActionSelectBase
{
public:
    LC_ActionPreSelectionAwareBase(
        const char *name, RS_EntityContainer &container, RS_GraphicView &graphicView,
        const QList<RS2::EntityType> &entityTypeList = {}, bool countSelectionDeep = false);

    void mouseMoveEvent(QMouseEvent *event) override;
    void updateMouseButtonHints() override;
    void init(int status) override;
protected:
    bool selectionComplete = false;
    bool countDeep = false;
    void selectionFinishedByKey(QKeyEvent *e, bool escape) override;
    void mouseRightButtonReleaseEvent(int status, QMouseEvent *e) override;
    void mouseLeftButtonReleaseEvent(int status, QMouseEvent *e) override;
    virtual void selectionCompleted(bool singleEntity);
    virtual void mouseLeftButtonReleaseEventSelected(int status, QMouseEvent *pEvent);
    virtual void mouseRightButtonReleaseEventSelected(int status, QMouseEvent *pEvent);
    virtual void mouseMoveEventSelected(QMouseEvent *e);
    virtual void updateMouseButtonHintsForSelection() = 0;
    virtual void updateMouseButtonHintsForSelected(int status);

    RS2::CursorType doGetMouseCursor(int status) override;
    virtual RS2::CursorType doGetMouseCursorSelected(int status);

    unsigned int countSelectedEntities();
    void setSelectionComplete(bool allowEmptySelection);
    virtual bool isAllowTriggerOnEmptySelection(){return true;};
};

#endif // LC_ACTIONPRESELECTIONAWAREBASE_H