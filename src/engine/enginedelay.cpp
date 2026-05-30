#include "enginedelay.h"

#include "control/controlpotmeter.h"
#include "control/controlproxy.h"
#include "engine/engine.h"
#include "moc_enginedelay.cpp"
#include "util/assert.h"
#include "util/sample.h"

namespace {
constexpr double kdMaxDelayPot = 500;
const int kiMaxDelay = static_cast<int>((kdMaxDelayPot + 8) / 1000 *
        mixxx::audio::SampleRate::kValueMax * mixxx::kEngineChannelOutputCount);
const QString kAppGroup = QStringLiteral("[App]");
} // anonymous namespace

EngineDelay::EngineDelay(const ConfigKey& delayControl, bool bPersist)
        : m_delayBuffer(kiMaxDelay),
          m_iDelayPos(0),
          m_iDelay(0) {
    m_delayBuffer.clear();
    m_pDelayPot = new ControlPotmeter(delayControl, 0, kdMaxDelayPot, false, true, false, bPersist);
    m_pDelayPot->setDefaultValue(0);
    connect(m_pDelayPot,
            &ControlObject::valueChanged,
            this,
            &EngineDelay::slotDelayChanged,
            Qt::DirectConnection);

    m_pSampleRate = new ControlProxy(kAppGroup, QStringLiteral("samplerate"), this);
    m_pSampleRate->connectValueChanged(this, &EngineDelay::slotDelayChanged, Qt::DirectConnection);
}

EngineDelay::~EngineDelay() {
    delete m_pDelayPot;
}

void EngineDelay::slotDelayChanged() {
    double newDelay = m_pDelayPot->get();
    double sampleRate = m_pSampleRate->get();

    m_iDelay = (int)(sampleRate * newDelay / 1000);
    m_iDelay *= 2;
    if (m_iDelay > (kiMaxDelay - 2)) {
        m_iDelay = (kiMaxDelay - 2);
    }
    if (m_iDelay <= 0) {
        // We start bypassing, so clear buffer, to avoid noise in case of re-enable delay
        m_delayBuffer.clear();
    }
}

void EngineDelay::process(CSAMPLE* pInputOutput, const std::size_t bufferSize) {
    if (m_iDelay <= 0) {
        return;
    }

    // The "+ kiMaxDelay" addition ensures positive values for the modulo calculation.
    int iDelaySourcePos = (m_iDelayPos + kiMaxDelay - m_iDelay) % kiMaxDelay;

    VERIFY_OR_DEBUG_ASSERT(iDelaySourcePos >= 0 && iDelaySourcePos < kiMaxDelay) {
        return;
    }

    for (std::size_t i = 0; i < bufferSize; ++i) {
        // put sample into delay buffer:
        m_delayBuffer[m_iDelayPos] = pInputOutput[i];

        // Take delayed sample from delay buffer and copy it to dest buffer:
        pInputOutput[i] = m_delayBuffer[iDelaySourcePos];

        // Increment indices without using expensive modulo (%) in the inner loop
        m_iDelayPos++;
        if (m_iDelayPos >= kiMaxDelay) {
            m_iDelayPos = 0;
        }
        iDelaySourcePos++;
        if (iDelaySourcePos >= kiMaxDelay) {
            iDelaySourcePos = 0;
        }
    }
}

void EngineDelay::setDelay(double newDelay) {
    m_pDelayPot->set(newDelay);
}
