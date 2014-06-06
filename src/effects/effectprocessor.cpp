#include "effects/effectprocessor.h"
#include "effects/native/eqeffect.h"

template <typename T>
ControlObjectSlave* GroupEffectProcessor<T>::m_pSampleRate = NULL;

template <typename T>
int GroupEffectProcessor<T>::getSampleRate() {
    if (!m_pSampleRate) {
        m_pSampleRate = new ControlObjectSlave("[Master]", "samplerate");
    }
    return static_cast<int>(m_pSampleRate->get());
}

template class GroupEffectProcessor<EqEffectGroupState>;
