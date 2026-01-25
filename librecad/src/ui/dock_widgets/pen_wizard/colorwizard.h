#ifndef COLORWIZARD_H
#define COLORWIZARD_H

#include <QFrame>
#include <memory>

class QListWidgetItem;

namespace Ui
{
    class ColorWizard;
}

class ColorWizard : public QFrame{
    Q_OBJECT
public:
    explicit ColorWizard(QWidget* parent = nullptr);
    ~ColorWizard() override;
    QStringList getFavList() const;
    void addFavorite(const QString& color) const;
private:
    std::unique_ptr<Ui::ColorWizard> ui;
signals:
    void requestingColorChange(QColor);
    void requestingSelection(QColor);
    void colorDoubleClicked(QColor);
protected slots:
    void requestColorChange();
    void requestSelection();
    void invokeColorDialog();
    void addOrRemove() const;
    void removeFavorite() const;
    void handleDoubleClick(const QListWidgetItem* item);
};

#endif
