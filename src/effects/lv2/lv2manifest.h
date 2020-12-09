#pragma once

#include "effects/effectmanifest.h"
#include "effects/defs.h"
#include <lilv-0/lilv/lilv.h>

class LV2Manifest {
  public:
    enum Status {
        AVAILABLE,
        IO_NOT_STEREO,
        HAS_REQUIRED_FEATURES
    };

    LV2Manifest(const LilvPlugin* plug, QHash<QString, LilvNode*>& properties);
    ~LV2Manifest();
    EffectManifestPointer getEffectManifest() const;
    QList<int> getAudioPortIndices();
    QList<int> getControlPortIndices();
    const LilvPlugin* getPlugin();
    bool isValid();
    Status getStatus();

  private:
    void buildEnumerationOptions(const LilvPort* port,
                                 EffectManifestParameterPointer param);
    const LilvPlugin* m_pLV2plugin;
    EffectManifestPointer m_pEffectManifest;

    // This list contains:
    // position 0 -> input_left port index
    // position 1 -> input_right port index
    // position 2 -> output_left port index
    // position 3 -> output_right port index
    QList<int> audioPortIndices;
    // This list contains the control port indices
    QList<int> controlPortIndices;

    // Arrays used for storing minimum, maximum and default parameter values
    float* m_minimum;
    float* m_maximum;
    float* m_default;
    Status m_status;
};
