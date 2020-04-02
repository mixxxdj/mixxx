#pragma once
#include <QDomElement>

#include "effects/defs.h"

class EffectPreset {
  public:
    EffectPreset();
    EffectPreset(const QDomElement& element);
    ~EffectPreset();

  private:
    QString m_id;
    QString m_version;
    double m_dMetaParameter;

    // QList <EffectParameterPresetPointer> m_effectParameterPresets;
};
