#include "engine/stems/enginestemseparator.h"

#ifdef __STEM_SEPARATOR__

#include "moc_enginestemseparator.cpp"
#include "engine/engine.h"
#include "util/logger.h"
#include "util/sample.h"

namespace {
const mixxx::Logger kLogger("EngineStemSeparator");

// Default model path — can be overridden via CO or settings
const QString kDefaultModelPath = QStringLiteral("/usr/local/share/stem-models/htdemucs_fp16weights.onnx");
} // anonymous namespace

// Static engine instance (shared across all decks)
StemEngine::MultiDeckManager EngineStemSeparator::s_engine;
int EngineStemSeparator::s_instanceCount = 0;

// ---------------------------------------------------------------------------
// Map Mixxx group to StemEngine::DeckId
// ---------------------------------------------------------------------------
static StemEngine::DeckId groupToDeckId(const QString& group) {
    // group is like "[Channel1]" or "[Channel2]"
    if (group.contains(QChar('1')) || group.contains(QStringLiteral("Channel1"))) {
        return StemEngine::DeckId::DECK_A;
    }
    return StemEngine::DeckId::DECK_B;
}

// ---------------------------------------------------------------------------
// Constructor / Destructor
// ---------------------------------------------------------------------------
EngineStemSeparator::EngineStemSeparator(
        const QString& group,
        UserSettingsPointer pConfig,
        QObject* parent)
        : QObject(parent),
          m_group(group),
          m_pConfig(pConfig),
          m_enabled(false) {

    // Create COs for GUI / skin interaction
    m_pEnabled = std::make_unique<ControlPushButton>(
            ConfigKey(group, "stem_separator_enabled"));
    m_pEnabled->setButtonMode(mixxx::control::ButtonMode::Toggle);
    connect(m_pEnabled.get(),
            &ControlPushButton::valueChanged,
            this,
            &EngineStemSeparator::slotEnabledChanged);

    m_pStatus = std::make_unique<ControlObject>(
            ConfigKey(group, "stem_separator_status"));
    m_pStatus->setReadOnly();
    m_pStatus->forceSet(0.0);

    m_pModel = std::make_unique<ControlObject>(
            ConfigKey(group, "stem_separator_model"));
    // Load default model path from config if available
    QString modelPath = m_pConfig
            ? m_pConfig->getValue(ConfigKey(group, "stem_separator_model_path"),
                    kDefaultModelPath)
            : kDefaultModelPath;
    m_pModel->forceSet(0.0); // placeholder, real path via settings

    // Pre-allocate output buffers for worst-case buffer size
    // Max Mixxx buffer is typically 8192 stereo samples
    constexpr SINT kMaxBufferSamples = 16384; // ~372ms @ 44.1kHz
    for (int i = 0; i < kNumStemsInternal; ++i) {
        m_stemOutputs[i].resize(kMaxBufferSamples);
        m_stemOutputs[i].fill(0.0f);
    }

    // Reference-counted engine init
    if (s_instanceCount == 0) {
        kLogger.info() << "Initialising MultiDeckManager (first instance)";
        bool ok = s_engine.init(modelPath.toStdString());
        if (!ok) {
            kLogger.warning()
                    << "MultiDeckManager init failed — model not found at"
                    << modelPath;
        }
    }
    s_instanceCount++;

    kLogger.info()
            << "Created separator for" << group
            << "deckId=" << static_cast<int>(groupToDeckId(group));
}

EngineStemSeparator::~EngineStemSeparator() {
    setEnabled(false);
    s_instanceCount--;
    if (s_instanceCount <= 0) {
        kLogger.info() << "Shutting down MultiDeckManager (last instance)";
        s_engine.shutdown();
        s_instanceCount = 0;
    }
    kLogger.info() << "Destroyed separator for" << m_group;
}

// ---------------------------------------------------------------------------
// feedAudio — called from audio callback (RT-safe)
// ---------------------------------------------------------------------------
bool EngineStemSeparator::feedAudio(const CSAMPLE* pBuffer, SINT bufferSize) {
    if (!m_enabled) {
        return false;
    }

    // bufferSize is the number of stereo samples (L,R,L,R,...)
    // processAudioBlock takes numFrames (stereo frames)
    const size_t numFrames = bufferSize / mixxx::kEngineChannelOutputCount;
    if (numFrames == 0) {
        return false;
    }

    // Prepare output stem pointers (internal: 4 stems for the engine)
    float* stemPtrs[kNumStemsInternal];
    for (int i = 0; i < kNumStemsInternal; ++i) {
        stemPtrs[i] = m_stemOutputs[i].data();
    }

    // Call the engine's RT-safe processing function.
    // This copies the input into an internal ring buffer and reads
    // pre-computed separated stems from the worker thread.
    s_engine.processAudioBlock(
            groupToDeckId(m_group),
            pBuffer,
            stemPtrs,
            numFrames);

    // Update status based on engine state
    StemEngine::DeckState state = s_engine.getState(groupToDeckId(m_group));
    int newStatus = 0; // off
    switch (state) {
    case StemEngine::DeckState::LOADING:
        newStatus = 1; // buffering
        break;
    case StemEngine::DeckState::READY:
    case StemEngine::DeckState::PLAYING:
        newStatus = 2; // active
        break;
    default:
        newStatus = 0; // off / error
        break;
    }

    if (newStatus != m_status.loadRelaxed()) {
        m_status.storeRelaxed(newStatus);
        m_pStatus->forceSet(static_cast<double>(newStatus));
        emit statusChanged(newStatus);
    }

    return (m_status.loadRelaxed() == 2);
}

// ---------------------------------------------------------------------------
// readStem — called from audio callback (RT-safe)
// Must be called AFTER feedAudio() in the same callback cycle.
// ---------------------------------------------------------------------------
// Mapping interno (ONNX) → UI expuesto (3 stems):
//   UI 0: Vocales       ← ONNX stem 3 (vocals)
//   UI 1: Instrumentos  ← ONNX stem 1 (bass) + stem 2 (other) MIX
//   UI 2: Percusión     ← ONNX stem 0 (drums)
void EngineStemSeparator::readStem(int stemIdx, CSAMPLE* pOutput,
        SINT bufferSize) const {
    DEBUG_ASSERT(stemIdx >= 0 && stemIdx < kNumStemsExposed);

    if (stemIdx == 0) {
        // Vocales → ONNX stem 3
        SampleUtil::copy(pOutput, m_stemOutputs[3].constData(), bufferSize);
    } else if (stemIdx == 1) {
        // Instrumentos → Bass (1) + Other (2) MIX
        const CSAMPLE* bassPtr = m_stemOutputs[1].constData();
        const CSAMPLE* otherPtr = m_stemOutputs[2].constData();
        for (SINT i = 0; i < bufferSize; ++i) {
            pOutput[i] = bassPtr[i] + otherPtr[i];
        }
    } else if (stemIdx == 2) {
        // Percusión → ONNX stem 0 (drums)
        SampleUtil::copy(pOutput, m_stemOutputs[0].constData(), bufferSize);
    }
}

// ---------------------------------------------------------------------------
// isReady / setEnabled / isEnabled
// ---------------------------------------------------------------------------
bool EngineStemSeparator::isReady() const {
    return m_enabled && (m_status.loadRelaxed() == 2);
}

void EngineStemSeparator::setEnabled(bool enabled) {
    if (m_enabled == enabled) return;
    m_enabled = enabled;

    if (enabled) {
        // Notify the engine this deck is active
        m_status.storeRelaxed(1); // buffering
        m_pStatus->forceSet(1.0);
        emit statusChanged(1);
    } else {
        m_status.storeRelaxed(0);
        m_pStatus->forceSet(0.0);
        emit statusChanged(0);
    }
}

bool EngineStemSeparator::isEnabled() const {
    return m_enabled;
}

// ---------------------------------------------------------------------------
// loadModel
// ---------------------------------------------------------------------------
bool EngineStemSeparator::loadModel(const QString& modelPath) {
    kLogger.info() << "Loading model from" << modelPath;
    // Shutdown and reinit with new model path
    s_engine.shutdown();
    bool ok = s_engine.init(modelPath.toStdString());
    if (!ok) {
        kLogger.warning() << "Failed to load model from" << modelPath;
    }
    if (m_pConfig) {
        m_pConfig->setValue(
                ConfigKey(m_group, "stem_separator_model_path"), modelPath);
    }
    return ok;
}

// ---------------------------------------------------------------------------
// slotEnabledChanged
// ---------------------------------------------------------------------------
void EngineStemSeparator::slotEnabledChanged(double v) {
    setEnabled(v > 0.0);
}

#endif // __STEM_SEPARATOR__
