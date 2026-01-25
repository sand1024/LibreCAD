#ifndef ACTIONLIST_H
#define ACTIONLIST_H

#include <QListWidget>

class ActionList : public QListWidget{
    Q_OBJECT
public:
    explicit ActionList(QWidget* parent);
    void addActionItem(const QAction* action);
    void fromActionList(const QList<QAction*>& actionList);
    void fromActionMap(QMap<QString, QAction*>& actionMap);
public slots:
    void activateAction(const QListWidgetItem*);
protected:
    QList<QAction*> m_actionList;
};

#endif
