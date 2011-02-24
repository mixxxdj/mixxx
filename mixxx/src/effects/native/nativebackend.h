#ifndef NATIVEBACKEND_H
#define NATIVEBACKEND_H

#include "effects/effectsbackend.h"

class NativeBackend : public EffectsBackend {
  public:
    NativeBackend();
    virtual ~NativeBackend();

    const QList<EffectManifest> getAvailableEffects() const;
    EffectPointer instantiateEffect(const EffectManifest& manifest) const;
};

#endif /* NATIVEBACKEND_H */
