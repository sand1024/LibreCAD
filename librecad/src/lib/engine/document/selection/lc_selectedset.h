#ifndef LC_SELECTEDSET_H
#define LC_SELECTEDSET_H
#include <QList>

#include "lc_selectedsetlistener.h"

class RS_Entity;

class LC_SelectedSet{
public:
    explicit LC_SelectedSet();
    virtual ~LC_SelectedSet();
    void clear();
    void add(RS_Entity* entity);
    void remove(RS_Entity* entity);
    void replaceBy(QList<RS_Entity*>& entities);

    bool isEmpty() const {return m_entitiesList.isEmpty();}

    RS_Entity* first() const {
        return m_entitiesList.first();
    }

    RS_Entity *last() const {
        return m_entitiesList.last();
    }

    QList<RS_Entity *>::const_iterator begin() const {
        return m_entitiesList.begin();
    }
    QList<RS_Entity *>::const_iterator end() const {
        return m_entitiesList.end();
    }

    void disableListeners();
    bool enableListeners();
    void addListener(LC_SelectedSetListener* listener);
    void removeListener(LC_SelectedSetListener* listener);
    void fireSelectionChanged();
    bool isSilent() const {return m_silentMode != 0;}
    void cleanup() ;
    bool collectSelectedEntities(QList<RS_Entity*>& list);
private:
    QList<RS_Entity*> m_entitiesList;
    QList<LC_SelectedSetListener*> m_listeners;
    int m_silentMode{false};
    bool m_changedInSilent{false};
};

#endif // LC_SELECTEDSET_H
