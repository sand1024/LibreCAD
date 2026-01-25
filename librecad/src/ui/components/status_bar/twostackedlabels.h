#ifndef TWOSTACKEDLABELS_H
#define TWOSTACKEDLABELS_H

#include <QFrame>

class QLabel;

class TwoStackedLabels : public QFrame{
    Q_OBJECT
protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
public:
    explicit TwoStackedLabels(QWidget* parent);
    void setTopLabel(const QString& status) const;
    void setTopLabelToolTip(const QString& tooltip) const;
    void setBottomLabel(const QString& status) const;
    void setBottomLabelToolTips(const QString& tooltip) const;
signals:
    void clicked();
private:
    QLabel* m_topLabel = nullptr;
    QLabel* m_bottomLabel = nullptr;
};

#endif
