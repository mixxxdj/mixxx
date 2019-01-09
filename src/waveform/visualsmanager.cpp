#include "waveform/visualsmanager.h"

#include "waveform/waveformwidgetfactory.h"
#include "control/controlobject.h"


DeckVisuals::DeckVisuals(const QString& group)
        : m_group(group),
          m_SlowTickCnt(0),
          m_trackLoaded(false),
          playButton(ConfigKey(group, "play")),
          loopEnabled(ConfigKey(group, "loop_enabled")),
          engineBpm(ConfigKey(group, "bpm")),
          engineKey(ConfigKey(group, "key")) {
    m_pTimeElapsed = std::make_unique<ControlObject>(ConfigKey(m_group, "time_elapsed"));
    m_pTimeRemaining = std::make_unique<ControlObject>(ConfigKey(m_group, "time_remaining"));
    m_pEndOfTrack = std::make_unique<ControlObject>(ConfigKey(group, "end_of_track"));
    m_pVisualBpm = std::make_unique<ControlObject>(ConfigKey(m_group, "visual_bpm"));
    m_pVisualKey = std::make_unique<ControlObject>(ConfigKey(m_group, "visual_key"));

    // Control used to communicate ratio playpos to GUI thread
    m_pVisualPlayPos = VisualPlayPosition::getVisualPlayPosition(m_group);
}

// this is called from WaveformWidgetFactory::render in the main thread with the
// configured waveform frame rate
void DeckVisuals::process(double remainingTimeTriggerSeconds) {
    double playPosition;
    double tempoTrackSeconds;
    m_pVisualPlayPos->getTrackTime(&playPosition, &tempoTrackSeconds);
    bool trackLoaded = (tempoTrackSeconds != 0.0);
    if (!m_trackLoaded && !trackLoaded) {
        return;
    }

    double timeRemaining = (1.0 - playPosition) * tempoTrackSeconds;
    m_pTimeRemaining->set(timeRemaining);
    m_pTimeElapsed->set(tempoTrackSeconds - timeRemaining);

    if (!playButton.toBool() || // not playing
            loopEnabled.toBool() || // in loop
            tempoTrackSeconds <= remainingTimeTriggerSeconds || // track too short
            timeRemaining > remainingTimeTriggerSeconds // before the trigger
            ) {
        m_pEndOfTrack->set(0.0);
    } else {
        m_pEndOfTrack->set(1.0);
    }

    // Update the BPM even more slowly
    m_SlowTickCnt = (m_SlowTickCnt + 1) % kSlowUpdateDivider;
    if (m_SlowTickCnt == 0 || !trackLoaded) {
        m_pVisualBpm->set(engineBpm.get());
    }
    m_pVisualKey->set(engineKey.get());

    m_trackLoaded = trackLoaded;
}
