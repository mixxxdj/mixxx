#ifndef BUILTINBACKEND_H
#define BUILTINBACKEND_H

#include "effects/defs.h"
#include "effects/effectsbackend.h"

class BuiltInBackend : public EffectsBackend {
  public:
    BuiltInBackend();
    virtual ~BuiltInBackend();

  private:
    QString debugString() const {
        return "BuiltInBackend";
    }
};

#endif /* BUILTINBACKEND_H */
