#ifndef NATIVEBACKEND_H
#define NATIVEBACKEND_H

#include "effects/effectsbackend.h"

class NativeBackend : public EffectsBackend {
  public:
    NativeBackend(QObject* pParent=NULL);
    virtual ~NativeBackend();

    const QList<EffectManifest> getAvailableEffects() const;
    EffectPointer instantiateEffect(const EffectManifest& manifest);
};

#endif /* NATIVEBACKEND_H */
