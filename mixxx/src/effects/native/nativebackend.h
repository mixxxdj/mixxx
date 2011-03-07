#ifndef NATIVEBACKEND_H
#define NATIVEBACKEND_H

#include "effects/effectsbackend.h"

class NativeBackend : public EffectsBackend {
    Q_OBJECT
  public:
    NativeBackend(QObject* pParent=NULL);
    virtual ~NativeBackend();

    //const QList<EffectManifestPointer> getAvailableEffects() const;
    //EffectPointer instantiateEffect(EffectManifestPointer manifest);
  private:
    QString debugString() const {
        return "NativeBackend";
    }

    QList<EffectManifestPointer> m_effectManifests;
};

#endif /* NATIVEBACKEND_H */
