#pragma once
#include <QDomElement>

#include "effects/defs.h"
#include "effects/presets/effectparameterpreset.h"

class EffectPreset {
  public:
    EffectPreset();
    EffectPreset(const QDomElement& element);
    ~EffectPreset();

  private:
    QString m_id;
    QString m_version;
    double m_dMetaParameter;

    QList<EffectParameterPreset> m_effectParameterPresets;
};
