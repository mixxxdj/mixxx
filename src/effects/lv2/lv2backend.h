#ifndef LV2BACKEND_H
#define LV2BACKEND_H

#include "effects/effectsbackend.h"

class LV2Backend : public EffectsBackend {
    Q_OBJECT
  public:
    LV2Backend(QObject* pParent=NULL);
    virtual ~LV2Backend();

  private:
    QString debugString() const {
        return "LV2Backend";
    }
};

#endif // LV2BACKEND_H
