#ifndef COMBOBOXOPTION_H
#define COMBOBOXOPTION_H
#include <QFrame>

namespace Ui {
    class ComboBoxOption;
}

class ComboBoxOption : public QFrame {
    Q_OBJECT
public:
    explicit ComboBoxOption(QWidget* parent);
    ~ComboBoxOption() override;
    void setTitle(const QString& title) const;
    void setOptionsList(const QStringList& options) const;
    void setCurrentOption(const QString& option);

protected:
    int m_lastSavedIndex;
    signals :
    void optionToSave(QString);

private
    slots :
    void saveIndexAndEmitOption();
    void setButtonState(int) const;

private:
    Ui::ComboBoxOption* ui;
};

#endif
