#pragma once

#include <lilv/lilv.h>

#include <QSharedPointer>
#include <vector>

#include "effects/backends/effectmanifest.h"

/// Refer to EffectManifest for documentation
class LV2Manifest : public EffectManifest {
  public:
    enum Status {
        AVAILABLE,
        IO_NOT_STEREO,
        HAS_REQUIRED_FEATURES
    };

    LV2Manifest(LilvWorld* world, const LilvPlugin* plug, QHash<QString, LilvNode*>& properties);

    QList<int> getAudioPortIndices();
    QList<int> getControlPortIndices();
    const LilvPlugin* getPlugin();
    bool isValid();
    Status getStatus();

  private:
    void buildEnumerationOptions(const LilvPort* port,
            EffectManifestParameterPointer param);
    const LilvPlugin* m_pLV2plugin;

    // This list contains:
    // position 0 -> input_left port index
    // position 1 -> input_right port index
    // position 2 -> output_left port index
    // position 3 -> output_right port index
    QList<int> audioPortIndices;
    // This list contains the control port indices
    QList<int> controlPortIndices;

    // Arrays used for storing minimum, maximum and default parameter values
    std::vector<float> m_minimum;
    std::vector<float> m_maximum;
    std::vector<float> m_default;
    Status m_status;
};

typedef QSharedPointer<LV2Manifest> LV2EffectManifestPointer;
