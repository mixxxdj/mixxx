#ifndef LV2MANIFEST_H
#define LV2MANIFEST_H

#include "effects/effectmanifest.h"
#include <lilv-0/lilv/lilv.h>

class LV2Manifest {
  public:
    LV2Manifest(const LilvPlugin* plug, QHash<QString, LilvNode*>& properties);
    ~LV2Manifest();

    EffectManifest getEffectManifest();
  private:
    const LilvPlugin* m_pLV2plugin;
    EffectManifest m_effectManifest;

    // Arrays used for storing minimum, maximum and default parameter values
    float* m_minimum;
    float* m_maximum;
    float* m_default;
};


#endif // LV2MANIFEST_H
