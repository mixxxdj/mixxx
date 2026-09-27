#pragma once

#include <QObject>
#include <QAtomicInt>
#include <QVector>
#include <QString>

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "preferences/usersettings.h"
#include "util/samplebuffer.h"
#include "util/types.h"

#ifdef __STEM_SEPARATOR__

// High-level engine from mixxx-stems-engine
#include <StemEngine.h>

// -------------------------------------------------------------------
// EngineStemSeparator — Per-deck AI stem separation engine
//
// Wraps the StemEngine::MultiDeckManager from mixxx-stems-engine.
// Each Mixxx deck maps to one DeckId (A or B).
//
// The MultiDeckManager handles internally:
//   - Ring buffers (lock-free SPSC)
//   - Background worker thread for ONNX inference
//   - STFT / iSTFT with overlap-add
//   - Model loading / pruning
//   - Pre-roll buffering
//
// feedAudio() and readStem() are callable from the audio callback
// (SCHED_FIFO, zero-allocations, lock-free).
//
// CO interface exposed to the skin:
//   [Channel<N>],stem_separator_enabled   — 0/1 toggle
//   [Channel<N>],stem_separator_status    — 0=off, 1=buffering, 2=active
//   [Channel<N>],stem_separator_model     — model file path
// -------------------------------------------------------------------
class EngineStemSeparator : public QObject {
    Q_OBJECT
  public:
    /// @param group  Mixxx group like "Channel1" or "Channel2"
    /// @param pConfig  User settings
    /// @param parent  QObject parent
    EngineStemSeparator(const QString& group,
            UserSettingsPointer pConfig,
            QObject* parent = nullptr);
    ~EngineStemSeparator() override;

    /// Called from audio callback (EngineDeck::process).
    /// Feeds `bufferSize` stereo samples and writes separated stems.
    /// Internally calls StemEngine::MultiDeckManager::processAudioBlock().
    /// Returns true when separated stem data is ready.
    bool feedAudio(const CSAMPLE* pBuffer, SINT bufferSize);

    /// Called from audio callback to read one stem channel.
    /// Must be called AFTER feedAudio() in the same callback cycle.
    /// @param stemIdx 0–3 (Vocals, Drums, Bass, Other)
    /// @param pOutput destination buffer (bufferSize stereo samples)
    /// @param bufferSize number of stereo samples (must match feedAudio)
    void readStem(int stemIdx, CSAMPLE* pOutput, SINT bufferSize) const;

    /// Returns true when separation is active and stems are flowing.
    bool isReady() const;

    /// Enable/disable AI separation.
    void setEnabled(bool enabled);
    bool isEnabled() const;

    /// Load/reload the ONNX model from path.
    /// Called automatically when the model CO changes.
    bool loadModel(const QString& modelPath);

  signals:
    void statusChanged(int status);

  private slots:
    void slotEnabledChanged(double v);

  private:
    // Internamente el modelo ONNX produce 4 stems (drums, bass, other, vocals)
    // Pero la UI expone solo 3: Vocals, Other, Percussion (drums+bass merged)
    static constexpr int kNumStemsInternal = 4;
    static constexpr int kNumStemsExposed = 3;

    QString m_group;
    UserSettingsPointer m_pConfig;
    bool m_enabled;

    // Control objects for GUI/skin interaction
    std::unique_ptr<ControlPushButton> m_pEnabled;
    std::unique_ptr<ControlObject> m_pStatus;
    std::unique_ptr<ControlObject> m_pModel;

    // The actual engine from mixxx-stems-engine (shared across decks)
    static StemEngine::MultiDeckManager s_engine;
    static int s_instanceCount;  // reference count for init/shutdown

    // Output buffers per stem (pre-allocated, reused each callback)
    // These hold the separated stereo data written by processAudioBlock
    // and read by readStem. Size = internal stems (4).
    QVector<CSAMPLE> m_stemOutputs[kNumStemsInternal];

    // Status
    QAtomicInt m_status; // 0=off, 1=buffering, 2=active
};

#endif // __STEM_SEPARATOR__
