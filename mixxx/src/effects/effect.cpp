
#include "effects/effect.h"

Effect::Effect(EffectsBackend* pBackend, EffectManifest* pManifest)
        : QOBject(pBackend),
          m_pEffectsBackend(pBackend),
          m_pEffectManifest(pManifest) {

}

Effect::~Effect() {


}


