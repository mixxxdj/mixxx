#pragma once

#include "effects/defs.h"
#include "effects/effectsbackend.h"

class BuiltInBackend : public EffectsBackend {
    Q_OBJECT
  public:
    BuiltInBackend(QObject* pParent);
    virtual ~BuiltInBackend();

  private:
    QString debugString() const {
        return "BuiltInBackend";
    }
};
