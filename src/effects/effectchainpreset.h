#ifndef EFFECTCHAINPRESET_H
#define EFFECTCHAINPRESET_H

#include <QDomElement>

#include "effects/defs.h"
#include "effects/effectpreset.h"


class EffectChainPreset {
  public:
    EffectChainPreset();
    EffectChainPreset(const QDomElement& element);
    ~EffectChainPreset();

    QString name() const {
        return m_name;
    }

  private:
    QString m_id;
    QString m_name;
    QString m_description;
    double m_dSuper;
    EffectChainMixMode m_mixMode;
    QList <EffectPresetPointer> m_effectPresets;
};

#endif /* EFFECTCHAINPRESET_H */