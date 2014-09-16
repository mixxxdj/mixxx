#include <QMutexLocker>

#include "vinylcontrol/vinylcontrolprocessor.h"

#include "vinylcontrol/defs_vinylcontrol.h"
#include "vinylcontrol/vinylcontrol.h"
#include "vinylcontrol/vinylcontrolxwax.h"
#include "util/defs.h"
#include "controlpushbutton.h"
#include "util/timer.h"
#include "util/event.h"
#include "sampleutil.h"

#define SIGNAL_QUALITY_FIFO_SIZE 256
#define SAMPLE_PIPE_FIFO_SIZE 65536

VinylControlProcessor::VinylControlProcessor(QObject* pParent, ConfigObject<ConfigValue> *pConfig)
        : QObject(pParent),
          m_pConfig(pConfig),
          m_pToggle(new ControlPushButton(ConfigKey(VINYL_PREF_KEY, "Toggle"))),
          m_pWorkBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)),
          m_processorsLock(QMutex::Recursive),
          m_processors(kMaximumVinylControlInputs, NULL),
          m_signalQualityFifo(SIGNAL_QUALITY_FIFO_SIZE),
          m_bReportSignalQuality(false) {
    connect(m_pToggle, SIGNAL(valueChanged(double)),
            this, SLOT(toggleDeck(double)),
            Qt::DirectConnection);
    m_pGuiTick = new ControlObjectSlave("[Master]", "guiTickTime");
    m_pGuiTick->connectValueChanged(this, SLOT(slotGuiTick(double)));
}

VinylControlProcessor::~VinylControlProcessor() {
    wait();

    delete m_pToggle;
    SampleUtil::free(m_pWorkBuffer);

    {
        QMutexLocker locker(&m_processorsLock);
        for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
            VinylControl* pProcessor = m_processors.at(i);
            m_processors[i] = NULL;
            delete pProcessor;
        }
    }

    // xwax has a global LUT that we need to free after we've shut down our
    // vinyl control threads because it's not thread-safe.
    VinylControlXwax::freeLUTs();
}

void VinylControlProcessor::setSignalQualityReporting(bool enable) {
    m_bReportSignalQuality = enable;
}

void VinylControlProcessor::requestReloadConfig() {
    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        VinylControl* pCurrent = NULL;
        {
            QMutexLocker locker(&m_processorsLock);
            pCurrent = m_processors[i];

            if (pCurrent == NULL) {
                continue;
            }

            VinylControl *pNew = new VinylControlXwax(m_pConfig, kVCGroup.arg(i + 1));
            m_processors.replace(i, pNew);
            // Delete outside of the critical section to avoid deadlocks.
            delete pCurrent;
        }
    }
}

void VinylControlProcessor::onInputConfigured(AudioInput input) {
    if (input.getType() != AudioInput::VINYLCONTROL) {
        qDebug() << "WARNING: AudioInput type is not VINYLCONTROL. Ignoring.";
        return;
    }
    unsigned char index = input.getIndex();

    if (index >= kMaximumVinylControlInputs) {
        // Should not be possible.
        qWarning() << "VinylControlProcessor::onInputConnected got invalid index:" << index;
        return;
    }

    VinylControl *pNew = new VinylControlXwax(m_pConfig, kVCGroup.arg(index + 1));

    m_processorsLock.lock();
    VinylControl* pCurrent = m_processors.at(index);
    m_processors.replace(index, pNew);
    m_processorsLock.unlock();
    // Delete outside of the critical section to avoid deadlocks.
    delete pCurrent;
}

void VinylControlProcessor::onInputUnconfigured(AudioInput input) {
    if (input.getType() != AudioInput::VINYLCONTROL) {
        qDebug() << "WARNING: AudioInput type is not VINYLCONTROL. Ignoring.";
        return;
    }

    unsigned char index = input.getIndex();

    if (index >= kMaximumVinylControlInputs) {
        // Should not be possible.
        qWarning() << "VinylControlProcessor::onInputDisconnected got invalid index:" << index;
        return;
    }

    m_processorsLock.lock();
    VinylControl* pVC = m_processors.at(index);
    m_processors.replace(index, NULL);
    m_processorsLock.unlock();
    // Delete outside of the critical section to avoid deadlocks.
    delete pVC;
}

bool VinylControlProcessor::deckConfigured(int index) const {
    return m_processors[index] != NULL;
}

void VinylControlProcessor::receiveBuffer(AudioInput input,
                                          const CSAMPLE* pBuffer,
                                          unsigned int nFrames) {
    ScopedTimer t("VinylControlProcessor::receiveBuffer");
    if (input.getType() != AudioInput::VINYLCONTROL) {
        qDebug() << "WARNING: AudioInput type is not VINYLCONTROL. Ignoring incoming buffer.";
        return;
    }

    unsigned char vcIndex = input.getIndex();

    if (vcIndex >= kMaximumVinylControlInputs) {
        // Should not be possible.
        return;
    }

    const int kChannels = 2;
    const int nSamples = nFrames * kChannels;

    // The consequences for not processing samples are low, so just don't even
    // risk causing dropouts.
    if (!m_processorsLock.tryLock()) {
        qWarning() << "VinylControlProcessor thread locked, skipping processing.";
        return;
    }
    VinylControl* pProcessor = m_processors[vcIndex];

    int framesRead = nSamples / 2;
    if (pProcessor) {
        pProcessor->analyzeSamples(pBuffer, framesRead);
    } else {
        // Samples are being written to a non-existent processor. Warning?
        qWarning() << "Samples written to non-existent VinylControl processor:" << vcIndex;
    }

    m_processorsLock.unlock();
}

void VinylControlProcessor::slotGuiTick(double v) {
    Q_UNUSED(v)

    if (m_bReportSignalQuality) {
        for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
            VinylControl* pProcessor = m_processors[i];
            if (!pProcessor) {
                continue;
            }
            VinylSignalQualityReport report;
            // It's not really a big deal if the quality report bitmap is being
            // written while we're reading it, so no need to hold the lock.
            if (pProcessor->writeQualityReport(&report)) {
                report.processor = i;
                if (m_signalQualityFifo.write(&report, 1) != 1) {
                    qWarning() << "VinylControlProcessor could not write signal quality report for VC index:" << i;
                }
            }
        }
    }
}

void VinylControlProcessor::toggleDeck(double value) {
    if (!value)
        return;

    /** few different cases here:
     * 1. No decks have vinyl control enabled.
     * 2. One deck has vinyl control enabled.
     * 3. Many decks have vinyl control enabled.
     *
     * For case 1, we'll just enable vinyl control on the first deck. Case 2
     * is the most common one, we'll just turn off the vinyl control on the
     * deck currently using it and turn it on on the next one (sequentially,
     * wrapping as needed). Behavior in case 3 is totally non-obvious and
     * will be ignored.
     */

    // -1 means we haven't found a proxy that's enabled
    int enabled = -1;

    QMutexLocker locker(&m_processorsLock);

    for (int i = 0; i < m_processors.size(); ++i) {
        VinylControl* pProcessor = m_processors.at(i);
        if (pProcessor && pProcessor->isEnabled()) {
            if (enabled > -1) {
                return; // case 3
            }
            enabled = i;
        }
    }

    if (enabled > -1 && m_processors.size() > 1) {
        // handle case 2

        int nextProxy = (enabled + 1) % m_processors.size();
        while (!m_processors[nextProxy]) {
            nextProxy = (nextProxy + 1) % m_processors.size();
        } // guaranteed to terminate as there's at least 1 non-null proxy

        if (nextProxy == enabled) {
            return;
        }

        VinylControl* pEnabled = m_processors[enabled];
        VinylControl* pNextProxy = m_processors[nextProxy];
        locker.unlock();
        pEnabled->toggleVinylControl(false);
        pNextProxy->toggleVinylControl(true);
    } else if (enabled == -1) {
        // handle case 1, or we just don't have any processors
        foreach (VinylControl* pProcessor, m_processors) {
            if (pProcessor) {
                locker.unlock();
                pProcessor->toggleVinylControl(true);
                return;
            }
        }
    }
}
