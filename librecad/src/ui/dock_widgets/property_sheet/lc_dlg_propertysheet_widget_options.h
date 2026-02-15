#ifndef LC_DLGPROPERTYSHEETWIDGETOPTIONS_H
#define LC_DLGPROPERTYSHEETWIDGETOPTIONS_H

#include "lc_dialog.h"
#include "lc_propertysheet_widget_options.h"

namespace Ui {
    class LC_DlgPropertySheetWidgetOptions;
}

class LC_DlgPropertySheetWidgetOptions : public LC_Dialog {
    Q_OBJECT public:
    LC_DlgPropertySheetWidgetOptions(QWidget* parent, LC_PropertySheetWidgetOptions* options);
    ~LC_DlgPropertySheetWidgetOptions() override;
    void accept() override;
private:
    Ui::LC_DlgPropertySheetWidgetOptions* ui;
    LC_PropertySheetWidgetOptions* m_options;
};

#endif
