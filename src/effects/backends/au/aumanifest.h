#pragma once

#include "effects/backends/effectmanifest.h"
#include "effects/defs.h"

class AUManifest : public EffectManifest {
  public:
    AUManifest(const QString& id, const QString& name);
};
