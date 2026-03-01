#include "engine/controls/vinylcontrolcontrol.h"

#include "control/controlpushbutton.h"
#include "moc_vinylcontrolcontrol.cpp"
#include "track/track.h"
#include "vinylcontrol/defs_vinylcontrol.h"

VinylControlControl::VinylControlControl(const QString& group, UserSettingsPointer pConfig)
        : EngineControl(group, pConfig),
          m_playEnabled(ConfigKey(group, QStringLiteral("play"))),
          m_inputConfigured(ConfigKey(group, QStringLiteral("input_configured"))),
          m_bSeekRequested(false) {
    m_pControlVinylStatus = std::make_unique<ControlObject>(
            ConfigKey(group, QStringLiteral("vinylcontrol_status")), this);
    m_pControlVinylSpeedType = std::make_unique<ControlObject>(
            ConfigKey(group, QStringLiteral("vinylcontrol_speed_type")), this);

    //Convert the ConfigKey's value into a double for the CO (for fast reads).
    const QString strVinylSpeedType = pConfig->getValueString(ConfigKey(group,
            QStringLiteral("vinylcontrol_speed_type")));
    if (strVinylSpeedType == MIXXX_VINYL_SPEED_33) {
        m_pControlVinylSpeedType->set(MIXXX_VINYL_SPEED_33_NUM);
    } else if (strVinylSpeedType == MIXXX_VINYL_SPEED_45) {
        m_pControlVinylSpeedType->set(MIXXX_VINYL_SPEED_45_NUM);
    } else {
        m_pControlVinylSpeedType->set(MIXXX_VINYL_SPEED_33_NUM);
    }

    m_pControlVinylSeek = std::make_unique<ControlObject>(
            ConfigKey(group, QStringLiteral("vinylcontrol_seek")), this);
    connect(m_pControlVinylSeek.get(),
            &ControlObject::valueChanged,
            this,
            &VinylControlControl::slotControlVinylSeek,
            Qt::DirectConnection);

    // Most of these controls are only created here so they are available
    // for ControlProxies in other engine units
    m_pControlVinylRate = std::make_unique<ControlObject>(
            ConfigKey(group, QStringLiteral("vinylcontrol_rate")), this);

    m_pControlVinylScratching = std::make_unique<ControlPushButton>(
            ConfigKey(group, QStringLiteral("vinylcontrol_scratching")), this);
    m_pControlVinylScratching->set(0);
    m_pControlVinylScratching->setButtonMode(mixxx::control::ButtonMode::Toggle);

    m_pControlVinylEnabled = std::make_unique<ControlPushButton>(
            ConfigKey(group, QStringLiteral("vinylcontrol_enabled")), this);
    m_pControlVinylEnabled->set(0);
    m_pControlVinylEnabled->setButtonMode(mixxx::control::ButtonMode::Toggle);
    m_pControlVinylEnabled->connectValueChangeRequest(
            this,
            &VinylControlControl::slotControlEnabledChangeRequest);

    m_pControlVinylWantEnabled = std::make_unique<ControlPushButton>(
            ConfigKey(group, QStringLiteral("vinylcontrol_wantenabled")), this);
    m_pControlVinylWantEnabled->set(0);
    m_pControlVinylWantEnabled->setButtonMode(mixxx::control::ButtonMode::Toggle);

    m_pControlVinylMode = std::make_unique<ControlPushButton>(
            ConfigKey(group, QStringLiteral("vinylcontrol_mode")), this);
    m_pControlVinylMode->setBehavior(mixxx::control::ButtonMode::Toggle, 3);

    m_pControlVinylCueing = std::make_unique<ControlPushButton>(
            ConfigKey(group, QStringLiteral("vinylcontrol_cueing")), this);
    m_pControlVinylCueing->setBehavior(mixxx::control::ButtonMode::Toggle, 3);

    m_pControlVinylSignalEnabled = std::make_unique<ControlPushButton>(
            ConfigKey(group, QStringLiteral("vinylcontrol_signal_enabled")),
            this);
    m_pControlVinylSignalEnabled->set(1);
    m_pControlVinylSignalEnabled->setButtonMode(mixxx::control::ButtonMode::Toggle);
}

void VinylControlControl::trackLoaded(TrackPointer pNewTrack) {
    m_pTrack = pNewTrack;
}

void VinylControlControl::slotControlEnabledChangeRequest(double v) {
    // Warn the user if they try to enable vinyl control on a player with no
    // configured input.
    if (v > 0 && !m_inputConfigured.toBool()) {
        emit noVinylControlInputConfigured();
    } else {
        m_pControlVinylEnabled->setAndConfirm(v);
    }
}

void VinylControlControl::notifySeekQueued() {
    // m_bSeekRequested is set and unset in a single execution path,
    // so there are no issues with signals/slots causing timing
    // issues.
    if (m_pControlVinylMode->get() == MIXXX_VCMODE_ABSOLUTE &&
            m_playEnabled.get() > 0.0 &&
            !m_bSeekRequested) {
        m_pControlVinylMode->set(MIXXX_VCMODE_RELATIVE);
    }
}

void VinylControlControl::slotControlVinylSeek(double fractionalPos) {
    // Prevent NaN's from sneaking into the engine.
    if (util_isnan(fractionalPos)) {
        return;
    }

    // Do nothing if no track is loaded.
    TrackPointer pTrack = m_pTrack;
    FrameInfo info = frameInfo();
    if (!pTrack || !info.trackEndPosition.isValid()) {
        return;
    }

    const auto newPlayPos = mixxx::audio::kStartFramePos +
            (info.trackEndPosition - mixxx::audio::kStartFramePos) * fractionalPos;

    if (m_pControlVinylEnabled->get() > 0.0 && m_pControlVinylMode->get() == MIXXX_VCMODE_RELATIVE) {
        int cuemode = (int)m_pControlVinylCueing->get();

        //if in preroll, always seek
        if (newPlayPos < mixxx::audio::kStartFramePos) {
            seekExact(newPlayPos);
            return;
        }

        switch (cuemode) {
        case MIXXX_RELATIVE_CUE_OFF:
            return; // If off, do nothing.
        case MIXXX_RELATIVE_CUE_ONECUE: {
            //if onecue, just seek to the regular cue
            const mixxx::audio::FramePos mainCuePosition = pTrack->getMainCuePosition();
            if (mainCuePosition.isValid()) {
                seekExact(mainCuePosition);
            }
            return;
        }
        case MIXXX_RELATIVE_CUE_HOTCUE:
            // Continue processing in this function.
            break;
        default:
            qWarning() << "Invalid vinyl cue setting";
            return;
        }

        mixxx::audio::FrameDiff_t shortestDistance = 0;
        mixxx::audio::FramePos nearestPlayPos;

        const QList<CuePointer> cuePoints(pTrack->getCuePoints());
        QListIterator<CuePointer> it(cuePoints);
        while (it.hasNext()) {
            CuePointer pCue(it.next());
            if (pCue->getType() != mixxx::CueType::HotCue || pCue->getHotCue() == -1) {
                continue;
            }

            const mixxx::audio::FramePos cuePosition = pCue->getPosition();
            if (!cuePosition.isValid()) {
                continue;
            }
            // pick cues closest to newPlayPos
            if (!nearestPlayPos.isValid() || (fabs(newPlayPos - cuePosition) < shortestDistance)) {
                nearestPlayPos = cuePosition;
                shortestDistance = fabs(newPlayPos - cuePosition);
            }
        }

        if (!nearestPlayPos.isValid()) {
            if (newPlayPos >= mixxx::audio::kStartFramePos) {
                //never found an appropriate cue, so don't seek?
                return;
            }
            //if negative, allow a seek by falling down to the bottom
        } else {
            m_bSeekRequested = true;
            seekExact(nearestPlayPos);
            m_bSeekRequested = false;
            return;
        }
    }

    // Just seek where it wanted to originally.
    m_bSeekRequested = true;
    seekExact(newPlayPos);
    m_bSeekRequested = false;
}

bool VinylControlControl::isEnabled() {
    return m_pControlVinylEnabled->toBool();
}

bool VinylControlControl::isScratching() {
    return m_pControlVinylScratching->toBool();
}
