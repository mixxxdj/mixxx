#include "vinylcontrol/vinylcontrolprocessor.h"

#include <QMutexLocker>

#include "control/controlpushbutton.h"
#include "moc_vinylcontrolprocessor.cpp"
#include "util/defs.h"
#include "util/performancetimer.h"
#include "util/sample.h"
#include "util/timer.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "vinylcontrol/vinylcontrol.h"
#include "vinylcontrol/vinylcontrolxwax.h"

constexpr int kSignalQualityFifoSize = 256;
constexpr int kSamplePipeFifoSize = 65536;

VinylControlProcessor::VinylControlProcessor(QObject* pParent, UserSettingsPointer pConfig)
        : QThread(pParent),
          m_pConfig(pConfig),
          m_pToggle(std::make_unique<ControlPushButton>(ConfigKey(kVinylPrefKey, "Toggle"))),
          m_processors(),
          m_pWorkBuffer(SampleUtil::alloc(MAX_BUFFER_LEN)),
          m_processorsLock(QRecursiveMutex()),
          m_signalQualityFifo(kSignalQualityFifoSize),
          m_bReportSignalQuality(false),
          m_bQuit(false),
          m_bReloadConfig(false) {
    connect(m_pToggle.get(),
            &ControlPushButton::valueChanged,
            this,
            &VinylControlProcessor::toggleDeck,
            Qt::DirectConnection);

    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        m_samplePipes[i] = std::make_unique<FIFO<CSAMPLE>>(kSamplePipeFifoSize);
    }

    start(QThread::HighPriority);
}

VinylControlProcessor::~VinylControlProcessor() {
    m_bQuit = true;
    m_samplesAvailableSignal.wakeAll();
    wait();

    SampleUtil::free(m_pWorkBuffer);

    {
        const auto locker = QMutexLocker(&m_processorsLock);
        for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
            m_processors[i].reset();
            m_samplePipes[i].reset();
        }
    }

    // xwax has a global LUT that we need to free after we've shut down our
    // vinyl control threads because it's not thread-safe.
    VinylControlXwax::freeLUTs();
}

void VinylControlProcessor::setSignalQualityReporting(bool enable) {
    m_bReportSignalQuality = enable;
}

void VinylControlProcessor::shutdown() {
    m_bQuit = true;
    m_samplesAvailableSignal.wakeAll();
}

void VinylControlProcessor::requestReloadConfig() {
    m_bReloadConfig = true;
    m_samplesAvailableSignal.wakeAll();
}

void VinylControlProcessor::run() {
    unsigned static id = 0; // the id of this thread, for debugging purposes
    // FIXME copypasta (should factor this out somehow), -kousu 2/2009
    QThread::currentThread()->setObjectName(QString("VinylControlProcessor %1").arg(++id));

    PerformanceTimer reportTimer;
    reportTimer.start();

    while (!m_bQuit) {
        if (m_bReloadConfig) {
            reloadConfig();
            m_bReloadConfig = false;
        }

        bool timeToWriteSignalQualityReport = m_bReportSignalQuality &&
                reportTimer.elapsed().toIntegerMillis() >=
                        kVinylScopeUpdatePeriodMs;
        if (timeToWriteSignalQualityReport) {
            reportTimer.restart();
        }

        for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
            auto locker = QMutexLocker(&m_processorsLock);
            auto* pProcessor = m_processors[i].get();
            locker.unlock();
            auto* pSamplePipe = m_samplePipes[i].get();

            if (pSamplePipe->readAvailable() > 0) {
                int samplesRead = pSamplePipe->read(m_pWorkBuffer, MAX_BUFFER_LEN);

                if (samplesRead % 2 != 0) {
                    qWarning() << "VinylControlProcessor received non-even number of samples via sample FIFO.";
                    samplesRead--;
                }
                int framesRead = samplesRead / 2;

                if (pProcessor) {
                    pProcessor->analyzeSamples(m_pWorkBuffer, framesRead);
                } else {
                    // Samples are being written to a non-existent processor. Warning?
                    qWarning() << "Samples written to non-existent VinylControl processor:" << i;
                }
            }

            // If enabled, update the report at the same rate we use to call
            // VinylControlManager::updateSignalQualityListeners()
            if (pProcessor && timeToWriteSignalQualityReport) {
                VinylSignalQualityReport report;
                if (pProcessor->writeQualityReport(&report)) {
                    report.processor = i;
                    if (m_signalQualityFifo.write(&report, 1) == 1) {
                    } else {
                        qWarning() << "VinylControlProcessor could not write signal quality report for VC index:" << i;
                    }
                }
                emit qualityReportWritten();
            }
        }

        if (m_bQuit) {
            break;
        }

        // Wait for a signal from the main thread or engine thread that we
        // should wake up and process input.
        m_waitForSampleMutex.lock();
        m_samplesAvailableSignal.wait(&m_waitForSampleMutex);
        m_waitForSampleMutex.unlock();
    }
}

void VinylControlProcessor::reloadConfig() {
    for (int i = 0; i < kMaximumVinylControlInputs; ++i) {
        auto locker = QMutexLocker(&m_processorsLock);
        auto* pCurrent = m_processors[i].get();

        if (pCurrent == nullptr) {
            continue;
        }

        auto pNew = std::make_unique<VinylControlXwax>(m_pConfig, kVCGroup.arg(i + 1));

        auto pVinylControlProcessor = std::exchange(m_processors[i], std::move(pNew));

        locker.unlock();

        pVinylControlProcessor.reset();
    }
}

void VinylControlProcessor::onInputConfigured(const AudioInput& input) {
    if (input.getType() != AudioPathType::VinylControl) {
        qDebug() << "WARNING: AudioInput type is not VINYLCONTROL. Ignoring.";
        return;
    }
    unsigned char index = input.getIndex();

    if (index >= kMaximumVinylControlInputs) {
        // Should not be possible.
        qWarning() << "VinylControlProcessor::onInputConnected got invalid index:" << index;
        return;
    }

    auto pNew = std::make_unique<VinylControlXwax>(
            m_pConfig, kVCGroup.arg(index + 1));

    auto locker = QMutexLocker(&m_processorsLock);
    auto pVinylControlProcessor = std::exchange(m_processors[index], std::move(pNew));

    locker.unlock();
    // Delete outside of the critical section to avoid deadlocks.
    pVinylControlProcessor.reset();
}

void VinylControlProcessor::onInputUnconfigured(const AudioInput& input) {
    if (input.getType() != AudioPathType::VinylControl) {
        qDebug() << "WARNING: AudioInput type is not VINYLCONTROL. Ignoring.";
        return;
    }

    unsigned char index = input.getIndex();

    if (index >= kMaximumVinylControlInputs) {
        // Should not be possible.
        qWarning() << "VinylControlProcessor::onInputDisconnected got invalid index:" << index;
        return;
    }

    auto locker = QMutexLocker(&m_processorsLock);
    auto pVinylControlProcessor = std::exchange(m_processors[index], nullptr);
    locker.unlock();

    // Delete outside of the critical section to avoid deadlocks.
    pVinylControlProcessor.reset();
}

bool VinylControlProcessor::deckConfigured(int index) const {
    return m_processors[index] != nullptr;
}

void VinylControlProcessor::receiveBuffer(const AudioInput& input,
        const CSAMPLE* pBuffer,
        unsigned int nFrames) {
    ScopedTimer t(QStringLiteral("VinylControlProcessor::receiveBuffer"));
    if (input.getType() != AudioPathType::VinylControl) {
        qDebug() << "WARNING: AudioInput type is not VINYLCONTROL. Ignoring incoming buffer.";
        return;
    }

    unsigned char vcIndex = input.getIndex();

    if (vcIndex >= kMaximumVinylControlInputs) {
        // Should not be possible.
        return;
    }

    auto* pSamplePipe = m_samplePipes[vcIndex].get();

    if (pSamplePipe == nullptr) {
        // Should not be possible.
        return;
    }

    constexpr int kChannels = 2;
    const int nSamples = nFrames * kChannels;
    int samplesWritten = pSamplePipe->write(pBuffer, nSamples);

    if (samplesWritten < nSamples) {
        qWarning() << "ERROR: Buffer overflow in VinylControlProcessor. Dropping samples on the floor."
                   << "VCIndex:" << vcIndex;
    }

    m_samplesAvailableSignal.wakeAll();
}

void VinylControlProcessor::toggleDeck(double value) {
    if (value == 0) {
        return;
    }

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

    auto locker = QMutexLocker(&m_processorsLock);

    for (int i = 0; i < std::ssize(m_processors); ++i) {
        auto* pVinylControlProcessor = m_processors[i].get();
        if (pVinylControlProcessor && pVinylControlProcessor->isEnabled()) {
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

        auto* pEnabled = m_processors[enabled].get();
        auto* pNextProxy = m_processors[nextProxy].get();
        locker.unlock();
        pEnabled->toggleVinylControl(false);
        pNextProxy->toggleVinylControl(true);
    } else if (enabled == -1) {
        // handle case 1, or we just don't have any processors
        for (const auto& pProcessor : m_processors) {
            if (pProcessor) {
                locker.unlock();
                pProcessor->toggleVinylControl(true);
                return;
            }
        }
    }
}
