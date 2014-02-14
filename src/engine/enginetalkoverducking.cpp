#include "controlobjectslave.h"
#include "engine/enginetalkoverducking.h"

#define MIC_DUCK_THRESHOLD 0.1

EngineTalkoverDucking::EngineTalkoverDucking(
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

    m_pTalkoverDucking = new ControlPushButton(ConfigKey(m_group, "micDucking"));
    m_pTalkoverDucking->setButtonMode(ControlPushButton::TOGGLE);
    m_pTalkoverDucking->setStates(3);
    // Default to Auto ducking.
    m_pTalkoverDucking->set(
            m_pConfig->getValueString(ConfigKey(m_group, "duckMode"), "1").toDouble());
    connect(m_pTalkoverDucking, SIGNAL(valueChanged(double)),
            this, SLOT(slotDuckModeChanged(double)));
}

EngineTalkoverDucking::~EngineTalkoverDucking() {
    m_pConfig->set(ConfigKey(m_group, "duckStrength"), ConfigValue(m_pDuckStrength->get() * 100));
    m_pConfig->set(ConfigKey(m_group, "duckMode"), ConfigValue(m_pTalkoverDucking->get()));

    delete m_pDuckStrength;
    delete m_pTalkoverDucking;
}

void EngineTalkoverDucking::slotSampleRateChanged(double samplerate) {
    setParameters(
            MIC_DUCK_THRESHOLD, m_pDuckStrength->get(),
            samplerate / 2 * .1, samplerate / 2);
}

void EngineTalkoverDucking::slotDuckStrengthChanged(double strength) {
    setParameters(
            MIC_DUCK_THRESHOLD, strength,
            m_pMasterSampleRate->get() / 2 * .1, m_pMasterSampleRate->get() / 2);
    m_pConfig->set(ConfigKey(m_group, "duckStrength"), ConfigValue(strength * 100));
}

void EngineTalkoverDucking::slotDuckModeChanged(double mode) {
   m_pConfig->set(ConfigKey(m_group, "duckMode"), ConfigValue(mode));
}

CSAMPLE EngineTalkoverDucking::getGain(int numFrames) {
    // Apply microphone ducking.
    switch (getMode()) {
      case EngineTalkoverDucking::OFF:
        return 1.0;
      case EngineTalkoverDucking::AUTO:
        return calculateCompressedGain(numFrames);
      case EngineTalkoverDucking::MANUAL:
        return m_pDuckStrength->get();
    }
    qWarning() << "Invalid ducking mode, returning 1.0";
    return 1.0;
}
