#include "library/autodj/autofadecontroller.h"

#include <algorithm>

#include "control/controlproxy.h"
#include "control/controlpushbutton.h"
#include "mixer/basetrackplayer.h"
#include "moc_autofadecontroller.cpp"

namespace {
double clamp_to_unit_interval(double v) {
    if (v <= 0.0) {
        return 0.0;
    }
    if (v >= 1.0) {
        return 1.0;
    }
    return v;
}
} // namespace

AutoFadeController::AutoFadeController(BaseTrackPlayer* pPlayer)
        : m_state(AutoFaderState::Idle),
          m_transitionProgress(0.0),
          m_transitionTime(0.0),
          m_fadeBeginPos(0.0),
          m_fadeEndPos(0.0),
          m_isVolumeChangedTriggeredByMe(false),
          m_isPlayingChangeTriggeredByMe(false),
          m_group(pPlayer->getGroup()),
          m_orientation(m_group, "orientation"),
          m_playPos(m_group, "playposition"),
          m_play(m_group, "play"),
          m_duration(m_group, "duration"),
          m_volume(m_group, "volume"),
          m_fadeTime("[AutoFader]", "fade_time"),
          m_pPlayer(pPlayer) {
    m_playPos.connectValueChanged(this, &AutoFadeController::playerPositionChanged);
    m_play.connectValueChanged(this, &AutoFadeController::playerPlayingChanged);
    m_volume.connectValueChanged(this, &AutoFadeController::playerVolumeChanged);

    m_pIsFading = new ControlObject(ConfigKey(m_group, "auto_fade_active"));
    m_pFadeNow = new ControlPushButton(ConfigKey(m_group, "fade_now"));
    m_pFadeOutNow = new ControlPushButton(ConfigKey(m_group, "fade_out_now"));
    m_pFadeInNow = new ControlPushButton(ConfigKey(m_group, "fade_in_now"));

    connect(m_pFadeNow, &ControlObject::valueChanged, this, &AutoFadeController::controlFadeNow);
    connect(m_pFadeOutNow,
            &ControlObject::valueChanged,
            this,
            &AutoFadeController::controlFadeOutNow);
    connect(m_pFadeInNow,
            &ControlObject::valueChanged,
            this,
            &AutoFadeController::controlFadeOutNow);
}

AutoFadeController::~AutoFadeController() {
    delete m_pIsFading;
    delete m_pFadeNow;
    delete m_pFadeOutNow;
    delete m_pFadeInNow;
}

void AutoFadeController::playerPositionChanged(double playPosition) {
    if (m_state == AutoFaderState::Idle) {
        return;
    }

    // Note: this can be a delayed call of playerPositionChanged() where
    // the track was playing, but is now stopped.
    const bool isPlaying = m_play.toBool();
    if (!isPlaying) {
        cancelAndResetFade();
        return;
    }

    // Has the fade been completed?
    double currentVolume = getVolumeFader();
    if (m_faderTarget == currentVolume) {
        m_transitionProgress = 1.0;

        if (m_state == AutoFaderState::FadingOut) {
            // Fade out finished, stop the player
            togglePlay(false);

            // Reset the volume so it doesn't stay at 0.0.
            // This avoids surprises when the volume fader is not
            // visible and the user wants to play another track.
            setVolumeFader(m_faderOriginal);
        }

        // Reset the internal state to Idle
        cancelAndResetFade();
    } else {
        // We are in Fading state.
        // Calculate the current transitionProgress, the place between begin
        // and end position and the step we have taken since the last call
        double transitionProgress = (playPosition - m_fadeBeginPos) /
                (m_fadeEndPos - m_fadeBeginPos);

        // Clamp to [0.0,1.0] to avoid overshooting due to rounding errors
        // and the fact that the playPosition changes in steps,
        // and may therefore skip over the m_fadeEndPos.
        transitionProgress = clamp_to_unit_interval(transitionProgress);

        double transitionStep = transitionProgress - m_transitionProgress;
        if (transitionStep > 0.0) {
            // We have made progress.
            // Backward seeks pause the transitions; forward seeks speed up
            // the transitions. If there has been a seek beyond endPos, end
            // the transition immediately.
            double remainingFader = m_faderTarget - currentVolume;
            double adjustment = remainingFader /
                    (1.0 - m_transitionProgress) * transitionStep;
            // we move the volume fader linearly with
            // movements in this track's play position.
            double newVolume = currentVolume + adjustment;
            // Clamp to [0.0,1.0] to avoid overshooting due to rounding errors
            setVolumeFader(clamp_to_unit_interval(newVolume));
        }
        m_transitionProgress = transitionProgress;
        // if we are at 1.0 here, we need an additional callback until the last
        // step is processed and we can stop the deck.
    }
}

void AutoFadeController::playerPlayingChanged(bool) {
    if (!m_isPlayingChangeTriggeredByMe) {
        // Automatically stop any in-progress fade for the
        // associated player deck when the user interacts
        // with the player controls in any way.
        setState(AutoFaderState::Idle);
    }
}

void AutoFadeController::togglePlay(bool play) {
    DEBUG_ASSERT(!m_isPlayingChangeTriggeredByMe);
    m_isPlayingChangeTriggeredByMe = true;
    m_play.set(play ? 1.0 : 0.0);
    m_isPlayingChangeTriggeredByMe = false;
}

void AutoFadeController::playerVolumeChanged(double volume) {
    Q_UNUSED(volume);
    if (!m_isVolumeChangedTriggeredByMe) {
        // Automatically stop any in-progress fade for the
        // associated player deck when the user interacts
        // with the player controls in any way.
        cancelAndResetFade();
    }
}

double AutoFadeController::getVolumeFader() {
    return m_volume.getParameter();
}

void AutoFadeController::setVolumeFader(double value) {
    DEBUG_ASSERT(!m_isVolumeChangedTriggeredByMe); // Avoid endless recursion
    DEBUG_ASSERT(value >= 0.0 && value <= 1.0);
    m_isVolumeChangedTriggeredByMe = true;
    m_volume.setParameter(value);
    m_isVolumeChangedTriggeredByMe = false;
}

void AutoFadeController::startFade(AutoFaderState direction) {
    const bool isPlaying = m_play.toBool();
    if (direction == AutoFaderState::Idle ||
            (direction == AutoFaderState::FadingOut && !isPlaying) ||
            (direction == AutoFaderState::FadingIn && !m_pPlayer->getLoadedTrack())) {
        cancelAndResetFade();
    } else {
        m_transitionProgress = 0.0;
        // m_transitionTime = fabs(m_fadeTime.get());
        m_transitionTime = 5.0;
        m_fadeBeginPos = m_playPos.get();
        m_fadeEndPos = m_fadeBeginPos + m_transitionTime / m_duration.get();
        m_faderTarget = direction == AutoFaderState::FadingIn ? 1.0 : 0.0;
        m_faderOriginal = 1.0;

        if (direction == AutoFaderState::FadingIn && !isPlaying) {
            // Fade in requested, track is not playing yet
            togglePlay(true);
        }

        setState(direction);
    }
}

void AutoFadeController::cancelAndResetFade() {
    m_transitionProgress = 0.0;
    m_transitionTime = 0.0;
    m_fadeBeginPos = 0.0;
    m_fadeEndPos = 0.0;
    m_faderTarget = 0.0;
    m_faderOriginal = 0.0;
    setState(AutoFaderState::Idle);
}

void AutoFadeController::setState(AutoFaderState newState) {
    if (m_state == newState) {
        // Ignore no-op
        return;
    }

    // Update current state
    m_state = newState;

    // Allow skins to display the fader state
    m_pIsFading->setAndConfirm(newState == AutoFaderState::Idle ? 0.0 : 1.0);
}

void AutoFadeController::controlFadeNow(double value) {
    if (value > 0.0) {
        if (m_state == AutoFaderState::Idle) {
            const bool isPlaying = m_play.toBool();
            startFade(isPlaying ? AutoFaderState::FadingOut : AutoFaderState::FadingIn);
        } else if (m_state == AutoFaderState::FadingOut) {
            cancelAndResetFade();
            startFade(AutoFaderState::FadingIn);
        } else {
            cancelAndResetFade();
            startFade(AutoFaderState::FadingOut);
        }
    }
}

void AutoFadeController::controlFadeOutNow(double value) {
    if (value > 0.0) {
        const bool isPlaying = m_play.toBool();
        if (m_state == AutoFaderState::FadingOut) {
            cancelAndResetFade();
        } else if (isPlaying) {
            startFade(AutoFaderState::FadingOut);
        }
    }
}

void AutoFadeController::controlFadeInNow(double value) {
    if (value > 0.0) {
        const bool isPlaying = m_play.toBool();
        if (m_state == AutoFaderState::FadingIn) {
            cancelAndResetFade();
        } else if (isPlaying) {
            startFade(AutoFaderState::FadingIn);
        }
    }
}
