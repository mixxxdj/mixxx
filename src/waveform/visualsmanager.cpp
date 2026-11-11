#include "waveform/visualsmanager.h"

#include "waveform/visualplayposition.h"

DeckVisuals::DeckVisuals(const QString& group)
        : m_group(group),
          m_SlowTickCnt(0),
          m_trackLoaded(false),
          m_playButton(ConfigKey(m_group, QStringLiteral("play"))),
          m_loopEnabled(ConfigKey(m_group, QStringLiteral("loop_enabled"))),
          m_engineBpm(ConfigKey(m_group, QStringLiteral("bpm"))),
          m_visualBpm(ConfigKey(m_group, QStringLiteral("visual_bpm"))),
          m_engineKey(ConfigKey(m_group, QStringLiteral("key"))),
          m_visualKey(ConfigKey(m_group, QStringLiteral("visual_key"))),
          m_timeElapsed(ConfigKey(m_group, QStringLiteral("time_elapsed"))),
          m_timeRemaining(ConfigKey(m_group, QStringLiteral("time_remaining"))),
          m_endOfTrack(ConfigKey(m_group, QStringLiteral("end_of_track"))) {
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

    const double playPosSeconds = playPosition * tempoTrackSeconds;
    double timeRemaining = tempoTrackSeconds - playPosSeconds;
    m_timeRemaining.set(timeRemaining);
    m_timeElapsed.set(playPosSeconds);

    // get position of outro end and use it as remaining time
    // for end-of-track warning if it's valid
    const double outroEndPosSeconds = m_pVisualPlayPos->getTrackEndSeconds();
    if (outroEndPosSeconds > 0) {
        timeRemaining = outroEndPosSeconds - playPosSeconds;
    }

    if (!m_playButton.toBool() ||                               // not playing
            m_loopEnabled.toBool() ||                           // in loop
            tempoTrackSeconds <= remainingTimeTriggerSeconds || // track too short
            timeRemaining > remainingTimeTriggerSeconds) {      // before the trigger
        m_endOfTrack.set(0.0);
    } else {
        m_endOfTrack.set(1.0);
    }

    // Update the BPM even more slowly
    m_SlowTickCnt = (m_SlowTickCnt + 1) % kSlowUpdateDivider;
    if (m_SlowTickCnt == 0 || !trackLoaded) {
        m_visualBpm.set(m_engineBpm.get());
    }
    m_visualKey.set(m_engineKey.get());

    m_trackLoaded = trackLoaded;
}
