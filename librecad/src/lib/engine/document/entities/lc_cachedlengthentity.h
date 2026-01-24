#ifndef LC_CACHEDLENGTHENTITY_H
#define LC_CACHEDLENGTHENTITY_H

#include "rs_atomicentity.h"

class LC_CachedLengthEntity:public RS_AtomicEntity{
public:
    explicit LC_CachedLengthEntity(RS_EntityContainer* parent = nullptr);

  LC_CachedLengthEntity(const LC_CachedLengthEntity& entity)
      : RS_AtomicEntity{entity}
        , m_cachedLength{entity.m_cachedLength}{
  }

  LC_CachedLengthEntity& operator = (const LC_CachedLengthEntity& other)
  {
    if (this != &other) {
      RS_AtomicEntity::operator=(other);
      m_cachedLength = other.m_cachedLength;
    }
    return *this;
  }

  LC_CachedLengthEntity(LC_CachedLengthEntity&& entity) noexcept
      : RS_AtomicEntity{std::move(entity)}
      , m_cachedLength{entity.m_cachedLength}
  {
  }

  LC_CachedLengthEntity& operator = (LC_CachedLengthEntity&& other) noexcept {
    if (this != &other) {
      RS_AtomicEntity::operator=(std::move(other));
      m_cachedLength = other.m_cachedLength;
    }
    return *this;
  }

  double getLength() const override{
    return m_cachedLength;
  }

protected:
    // cached length for painting speedup
    double m_cachedLength = 0.0;
    virtual void updateLength() = 0;
};

#endif
