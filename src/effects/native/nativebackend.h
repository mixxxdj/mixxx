#ifndef NATIVEBACKEND_H
#define NATIVEBACKEND_H

#include "effects/effectsbackend.h"

class NativeEffect {
  public:
    virtual ~NativeEffect() { }
    virtual QString getId() const = 0;
    virtual EffectManifest getManifest() const = 0;
};

class NativeBackend : public EffectsBackend {
    Q_OBJECT
  public:
    NativeBackend(QObject* pParent=NULL);
    virtual ~NativeBackend();

  private:
    QString debugString() const {
        return "NativeBackend";
    }
    QList<EffectManifest> m_effectManifests;
};

#endif /* NATIVEBACKEND_H */
