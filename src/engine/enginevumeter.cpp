#include "engine/enginevumeter.h"

#include "control/controlpotmeter.h"
#include "control/controlproxy.h"
#include "moc_enginevumeter.cpp"
#include "util/math.h"
#include "util/sample.h"

namespace {

// Rate at which the vumeter is updated (using a sample rate of 44100 Hz):
constexpr int kVuUpdateRate = 30;  // in 1/s, fits to display frame rate
constexpr int kPeakDuration = 500; // in ms

// Smoothing Factors
// Must be from 0-1 the lower the factor, the more smoothing that is applied
constexpr CSAMPLE kAttackSmoothing = 1.0f; // .85
constexpr CSAMPLE kDecaySmoothing = 0.1f;  //.16//.4

} // namespace

EngineVuMeter::EngineVuMeter(const QString& group) {
    // The VUmeter widget is controlled via a controlpotmeter, which means
    // that it should react on the setValue(int) signal.
    m_ctrlVuMeter = new ControlPotmeter(ConfigKey(group, "VuMeter"), 0., 1.);
    // left channel VU meter
    m_ctrlVuMeterL = new ControlPotmeter(ConfigKey(group, "VuMeterL"), 0., 1.);
    // right channel VU meter
    m_ctrlVuMeterR = new ControlPotmeter(ConfigKey(group, "VuMeterR"), 0., 1.);

    // Used controlpotmeter as the example used it :/ perhaps someone with more
    // knowledge could use something more suitable...
    m_ctrlPeakIndicator = new ControlPotmeter(ConfigKey(group, "PeakIndicator"),
                                              0., 1.);
    m_ctrlPeakIndicatorL = new ControlPotmeter(ConfigKey(group, "PeakIndicatorL"),
                                              0., 1.);
    m_ctrlPeakIndicatorR = new ControlPotmeter(ConfigKey(group, "PeakIndicatorR"),
                                              0., 1.);

    m_pSampleRate = new ControlProxy("[Master]", "samplerate", this);

    // Initialize the calculation:
    reset();
}

EngineVuMeter::~EngineVuMeter()
{
    delete m_ctrlVuMeter;
    delete m_ctrlVuMeterL;
    delete m_ctrlVuMeterR;
    delete m_ctrlPeakIndicator;
    delete m_ctrlPeakIndicatorL;
    delete m_ctrlPeakIndicatorR;
}

void EngineVuMeter::process(CSAMPLE* pIn, const int iBufferSize) {
    CSAMPLE fVolSumL, fVolSumR;

    int sampleRate = (int)m_pSampleRate->get();

    SampleUtil::CLIP_STATUS clipped = SampleUtil::sumAbsPerChannel(&fVolSumL,
            &fVolSumR, pIn, iBufferSize);
    m_fRMSvolumeSumL += fVolSumL;
    m_fRMSvolumeSumR += fVolSumR;

    m_iSamplesCalculated += iBufferSize / 2;

    // Are we ready to update the VU meter?:
    if (m_iSamplesCalculated > (sampleRate / kVuUpdateRate)) {
        doSmooth(m_fRMSvolumeL,
                log10(SHRT_MAX * m_fRMSvolumeSumL
                                / (m_iSamplesCalculated * 1000) + 1));
        doSmooth(m_fRMSvolumeR,
                log10(SHRT_MAX * m_fRMSvolumeSumR
                                / (m_iSamplesCalculated * 1000) + 1));

        const double epsilon = .0001;

        // Since VU meters are a rolling sum of audio, the no-op checks in
        // ControlObject will not prevent us from causing tons of extra
        // work. Because of this, we use an epsilon here to be gentle on the GUI
        // and MIDI controllers.
        if (fabs(m_fRMSvolumeL - m_ctrlVuMeterL->get()) > epsilon) {
            m_ctrlVuMeterL->set(m_fRMSvolumeL);
        }
        if (fabs(m_fRMSvolumeR - m_ctrlVuMeterR->get()) > epsilon) {
            m_ctrlVuMeterR->set(m_fRMSvolumeR);
        }

        double fRMSvolume = (m_fRMSvolumeL + m_fRMSvolumeR) / 2.0;
        if (fabs(fRMSvolume - m_ctrlVuMeter->get()) > epsilon) {
            m_ctrlVuMeter->set(fRMSvolume);
        }

        // Reset calculation:
        m_iSamplesCalculated = 0;
        m_fRMSvolumeSumL = 0;
        m_fRMSvolumeSumR = 0;
    }

    if (clipped & SampleUtil::CLIPPING_LEFT) {
        m_ctrlPeakIndicatorL->set(1.);
        m_peakDurationL = kPeakDuration * sampleRate / iBufferSize / 2000;
    } else if (m_peakDurationL <= 0) {
        m_ctrlPeakIndicatorL->set(0.);
    } else {
        --m_peakDurationL;
    }

    if (clipped & SampleUtil::CLIPPING_RIGHT) {
        m_ctrlPeakIndicatorR->set(1.);
        m_peakDurationR = kPeakDuration * sampleRate / iBufferSize / 2000;
    } else if (m_peakDurationR <= 0) {
        m_ctrlPeakIndicatorR->set(0.);
    } else {
        --m_peakDurationR;
    }

    m_ctrlPeakIndicator->set(
            (m_ctrlPeakIndicatorR->toBool() || m_ctrlPeakIndicatorL->toBool())
                    ? 1.0
                    : 0.0);
}

void EngineVuMeter::doSmooth(CSAMPLE &currentVolume, CSAMPLE newVolume)
{
    if (currentVolume > newVolume) {
        currentVolume -= kDecaySmoothing * (currentVolume - newVolume);
    } else {
        currentVolume += kAttackSmoothing * (newVolume - currentVolume);
    }
    if (currentVolume < 0) {
        currentVolume=0;
    }
    if (currentVolume > 1.0) {
        currentVolume=1.0;
    }
}

void EngineVuMeter::reset() {
    m_ctrlVuMeter->set(0);
    m_ctrlVuMeterL->set(0);
    m_ctrlVuMeterR->set(0);
    m_ctrlPeakIndicator->set(0);
    m_ctrlPeakIndicatorL->set(0);
    m_ctrlPeakIndicatorR->set(0);

    m_iSamplesCalculated = 0;
    m_fRMSvolumeL = 0;
    m_fRMSvolumeSumL = 0;
    m_fRMSvolumeR = 0;
    m_fRMSvolumeSumR = 0;
    m_peakDurationL = 0;
    m_peakDurationR = 0;
}
