#ifndef LC_PENWIZARD_H
#define LC_PENWIZARD_H

#include "lc_graphicviewawarewidget.h"

class ColorWizard;

class LC_PenWizard : public LC_GraphicViewAwareWidget{
    Q_OBJECT
public:
    explicit LC_PenWizard(QWidget* parent = nullptr);
    void setGraphicView(RS_GraphicView* gview) override;
protected slots:
    void setColorForSelected(QColor color) const;
    void selectByColor(QColor color) const;
    void setActivePenColor(QColor color) const;
private:
    RS_GraphicView* m_graphicView = nullptr;
    ColorWizard* m_colorWizard = nullptr;
};

#endif // LC_PENWIZARD_H
