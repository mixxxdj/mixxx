
#include "effects/effect.h"
#include "effects/effectsbackend.h"

Effect::Effect(EffectsBackend* pBackend, EffectManifest& effectManifest)
        : QObject(pBackend),
          m_pEffectsBackend(pBackend),
          m_effectManifest(effectManifest) {

}

Effect::~Effect() {


}


