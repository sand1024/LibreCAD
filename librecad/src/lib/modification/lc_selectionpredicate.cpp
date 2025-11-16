#include "lc_selectionpredicate.h"

#include "rs_entity.h"

LC_SelectionPredicate::LC_SelectionPredicate(RS2::EntityType entityType) {
    if (entityType == RS2::EntityType::EntityUnknown) {
        m_acceptEntityFunction = []([[maybe_unused]]RS_Entity* e)-> bool {return true;};
    }
    else {
        m_acceptEntityFunction = [this](RS_Entity* e)-> bool {return e->rtti() == m_entityType;};
    }
}

bool LC_SelectionPredicate::accept(RS_Entity* entity) const {
    return false;
}

bool LC_SelectionPredicate::acceptRtti(RS_Entity* entity) const {
    return m_acceptEntityFunction(entity);
}
