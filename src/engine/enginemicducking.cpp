#include "controlobjectslave.h"
#include "engine/enginemicducking.h"

#define MIC_DUCK_THRESHOLD 0.1

EngineMicDucking::EngineMicDucking(
        ConfigObject<ConfigValue>* pConfig, const char* group)
    : EngineSideChainCompressor(pConfig, group),
      m_pConfig(pConfig),
      m_group(group) {
    m_pMasterSampleRate = new ControlObjectSlave(m_group, "samplerate");
    m_pMasterSampleRate->connectValueChanged(this, SLOT(slotSampleRateChanged(double)));

    // Set compressor threshold to .5 of full volume, strength .75, and .1
    // second attack and 1 sec decay.
    m_pDuckStrength = new ControlPotmeter(ConfigKey(m_group, "duckStrength"), 0.0, 1.0);
    m_pDuckStrength->set(
            m_pConfig->getValueString(ConfigKey(m_group, "duckStrength"), "90").toDouble() / 100);
    connect(m_pDuckStrength, SIGNAL(valueChanged(double)),
            this, SLOT(slotDuckStrengthChanged(double)));
    setParameters(
            MIC_DUCK_THRESHOLD,
            m_pDuckStrength->get(),
            m_pMasterSampleRate->get() / 2 * .1,
            m_pMasterSampleRate->get() / 2);

    m_pMicDucking = new ControlPushButton(ConfigKey(m_group, "micDucking"));
    m_pMicDucking->setButtonMode(ControlPushButton::TOGGLE);
    m_pMicDucking->setStates(3);
    // Default to Auto ducking.
    m_pMicDucking->set(
            m_pConfig->getValueString(ConfigKey(m_group, "duckMode"), "1").toDouble());
    connect(m_pMicDucking, SIGNAL(valueChanged(double)),
            this, SLOT(slotDuckModeChanged(double)));
}

EngineMicDucking::~EngineMicDucking() {
    m_pConfig->set(ConfigKey(m_group, "duckStrength"), ConfigValue(m_pDuckStrength->get() * 100));
    m_pConfig->set(ConfigKey(m_group, "duckMode"), ConfigValue(m_pMicDucking->get()));

    delete m_pDuckStrength;
    delete m_pMicDucking;
}

void EngineMicDucking::slotSampleRateChanged(double samplerate) {
    setParameters(
            MIC_DUCK_THRESHOLD, m_pDuckStrength->get(),
            samplerate / 2 * .1, samplerate / 2);
}

void EngineMicDucking::slotDuckStrengthChanged(double strength) {
    setParameters(
            MIC_DUCK_THRESHOLD, strength,
            m_pMasterSampleRate->get() / 2 * .1, m_pMasterSampleRate->get() / 2);
    m_pConfig->set(ConfigKey(m_group, "duckStrength"), ConfigValue(strength * 100));
}

void EngineMicDucking::slotDuckModeChanged(double mode) {
   m_pConfig->set(ConfigKey(m_group, "duckMode"), ConfigValue(mode));
}

CSAMPLE EngineMicDucking::getGain(int numFrames) {
    // Apply microphone ducking.
    switch (getMode()) {
      case EngineMicDucking::OFF:
        return 1.0;
      case EngineMicDucking::AUTO:
        return calculateCompressedGain(numFrames);
      case EngineMicDucking::MANUAL:
        return m_pDuckStrength->get();
    }
    qWarning() << "Invalid ducking mode, returning 1.0";
    return 1.0;
}
