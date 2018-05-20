#ifndef NATIVEBACKEND_H
#define NATIVEBACKEND_H

#include "effects/effectsbackend.h"

class NativeBackend : public EffectsBackend {
    Q_OBJECT
  public:
    NativeBackend(UserSettingsPointer pConfig, QObject* pParent = nullptr);
    virtual ~NativeBackend();

  private:
    QString debugString() const {
        return "NativeBackend";
    }
};

#endif /* NATIVEBACKEND_H */
