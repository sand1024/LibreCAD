#ifndef LC_ACTIONDRAWARC2POPTIONS_H
#define LC_ACTIONDRAWARC2POPTIONS_H

#include <QLabel>

#include "lc_action_options_widget.h"

namespace Ui {
    class LC_Arc2PointsOptionsWidget;
}

class RS_ActionInterface;
class LC_ActionDrawArc2PointsBase;

class LC_Arc2PointsOptionsWidget : public LC_ActionOptionsWidget{
    Q_OBJECT
public:
    explicit LC_Arc2PointsOptionsWidget(int actionType);
    ~LC_Arc2PointsOptionsWidget() override;
public slots:
    void onDirectionChanged(bool);
    void languageChange() override;
    void onValueChanged();
protected:
    void doUpdateByAction(RS_ActionInterface *a) override;
    void setReversedToActionAndView(bool reversed) const;
private:
    Ui::LC_Arc2PointsOptionsWidget *ui;
    LC_ActionDrawArc2PointsBase* m_action = nullptr;
    void updateTooltip( QLabel *label) const;
};

#endif
