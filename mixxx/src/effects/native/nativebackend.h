#ifndef NATIVEBACKEND_H
#define NATIVEBACKEND_H

#include "effects/effectsbackend.h"

class NativeBackend : public EffectsBackend {
    Q_OBJECT
  public:
    NativeBackend(QObject* pParent=NULL);
    virtual ~NativeBackend();

    const QList<EffectManifest> getAvailableEffects() const;
    EffectPointer instantiateEffect(const EffectManifest& manifest);
  private:
    QList<EffectManifest> m_effectManifests;
};

#endif /* NATIVEBACKEND_H */
