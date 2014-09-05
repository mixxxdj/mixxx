#ifndef FILTEREFFECT_H
#define FILTEREFFECT_H

#include <QMap>

#include "effects/effect.h"
#include "effects/effectprocessor.h"
#include "engine/effects/engineeffect.h"
#include "engine/effects/engineeffectparameter.h"
#include "engine/enginefilterbutterworth8.h"
#include "sampleutil.h"
#include "util.h"
#include "util/defs.h"
#include "util/types.h"

struct FilterGroupState {
    FilterGroupState()
            // TODO(XXX) 44100 should be changed to real sample rate
            // https://bugs.launchpad.net/mixxx/+bug/1208816.
            : lowFilter(44100, 20),
              bandpassFilter(44100, 20, 200),
              highFilter(44100, 20),
              oldDepth(0),
              oldBandpassWidth(0),
              oldBandpassGain(0) {
        SampleUtil::applyGain(bandpassBuffer, 0, MAX_BUFFER_LEN);
        SampleUtil::applyGain(crossfadeBuffer, 0, MAX_BUFFER_LEN);
    }

    EngineFilterButterworth8Low lowFilter;
    EngineFilterButterworth8Band bandpassFilter;
    EngineFilterButterworth8High highFilter;

    CSAMPLE bandpassBuffer[MAX_BUFFER_LEN];
    CSAMPLE crossfadeBuffer[MAX_BUFFER_LEN];
    double oldDepth;
    double oldBandpassWidth;
    CSAMPLE oldBandpassGain;
};

class FilterEffect : public GroupEffectProcessor<FilterGroupState> {
  public:
    FilterEffect(EngineEffect* pEffect, const EffectManifest& manifest);
    virtual ~FilterEffect();

    static QString getId();
    static EffectManifest getManifest();

    // See effectprocessor.h
    void processGroup(const QString& group,
                      FilterGroupState* pState,
                      const CSAMPLE* pInput, CSAMPLE *pOutput,
                      const unsigned int numSamples,
                      const unsigned int sampleRate,
                      const GroupFeatureState& groupFeatures);

  private:
    QString debugString() const {
        return getId();
    }

    void applyFilters(FilterGroupState* pState,
                      const CSAMPLE* pIn, CSAMPLE* pOut, CSAMPLE* pTempBuffer,
                      const unsigned int numSamples, const unsigned int sampleRate,
                      double depth, CSAMPLE bandpassGain);

    EngineEffectParameter* m_pDepthParameter;
    EngineEffectParameter* m_pBandpassWidthParameter;
    EngineEffectParameter* m_pBandpassGainParameter;

    DISALLOW_COPY_AND_ASSIGN(FilterEffect);
};

#endif /* FILTEREFFECT_H */
