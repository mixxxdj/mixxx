#include "engine/vinylcontrolcontrol.h"
#include "mathstuff.h"
#include "vinylcontrol/vinylcontrol.h"
#include "library/dao/cue.h"

VinylControlControl::VinylControlControl(const char* pGroup, ConfigObject<ConfigValue>* pConfig)
        : EngineControl(pGroup, pConfig) {
    m_pControlVinylStatus = new ControlObject(ConfigKey(pGroup, "vinylcontrol_status"));
    m_pControlVinylSpeedType = new ControlObject(ConfigKey(pGroup, "vinylcontrol_speed_type"));

    //Convert the ConfigKey's value into a double for the CO (for fast reads).
    QString strVinylSpeedType = pConfig->getValueString(ConfigKey(pGroup,
                                                      "vinylcontrol_speed_type"));
    if (strVinylSpeedType == MIXXX_VINYL_SPEED_33) {
        m_pControlVinylSpeedType->set(MIXXX_VINYL_SPEED_33_NUM);
    } else if (strVinylSpeedType == MIXXX_VINYL_SPEED_45) {
        m_pControlVinylSpeedType->set(MIXXX_VINYL_SPEED_45_NUM);
    } else {
        m_pControlVinylSpeedType->set(MIXXX_VINYL_SPEED_33_NUM);
    }

    m_pControlVinylSeek = new ControlObject(ConfigKey(pGroup, "vinylcontrol_seek"));
    connect(m_pControlVinylSeek, SIGNAL(valueChanged(double)),
            this, SLOT(slotControlVinylSeek(double)),
            Qt::DirectConnection);

    m_pControlVinylScratching = new ControlPushButton(ConfigKey(pGroup, "vinylcontrol_scratching"));
    m_pControlVinylScratching->set(0);
    m_pControlVinylScratching->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlVinylEnabled = new ControlPushButton(ConfigKey(pGroup, "vinylcontrol_enabled"));
    m_pControlVinylEnabled->set(0);
    m_pControlVinylEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlVinylWantEnabled = new ControlPushButton(ConfigKey(pGroup, "vinylcontrol_wantenabled"));
    m_pControlVinylWantEnabled->set(0);
    m_pControlVinylWantEnabled->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlVinylMode = new ControlPushButton(ConfigKey(pGroup, "vinylcontrol_mode"));
    m_pControlVinylMode->setStates(3);
    m_pControlVinylMode->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlVinylCueing = new ControlPushButton(ConfigKey(pGroup, "vinylcontrol_cueing"));
    m_pControlVinylCueing->setStates(3);
    m_pControlVinylCueing->setButtonMode(ControlPushButton::TOGGLE);
    m_pControlVinylSignalEnabled = new ControlPushButton(ConfigKey(pGroup, "vinylcontrol_signal_enabled"));
    m_pControlVinylSignalEnabled->set(1);
    m_pControlVinylSignalEnabled->setButtonMode(ControlPushButton::TOGGLE);
}

VinylControlControl::~VinylControlControl() {
    delete m_pControlVinylSeek;
    delete m_pControlVinylStatus;
    delete m_pControlVinylSpeedType;
    delete m_pControlVinylEnabled;
    delete m_pControlVinylMode;
    delete m_pControlVinylCueing;
}

void VinylControlControl::trackLoaded(TrackPointer pTrack) {
    m_pCurrentTrack = pTrack;
}

void VinylControlControl::trackUnloaded(TrackPointer pTrack) {
    m_pCurrentTrack.clear();
}

void VinylControlControl::slotControlVinylSeek(double change) {
    if(isnan(change) || change > 1.14 || change < -1.14) {
        // This seek is ridiculous.
        return;
    }

    double total_samples = getTotalSamples();
    double new_playpos = round(change*total_samples);

    // Do nothing if no track is loaded.
    if (!m_pCurrentTrack) {
        return;
    }

    if (m_pControlVinylEnabled->get() > 0.0 && m_pControlVinylMode->get() == MIXXX_VCMODE_RELATIVE) {
        int cuemode = (int)m_pControlVinylCueing->get();

        //if in preroll, always seek
        if (new_playpos < 0) {
            emit(seek(change));
            return;
        } else if (cuemode == MIXXX_RELATIVE_CUE_OFF) {
            return;  //if off, do nothing
        } else if (cuemode == MIXXX_RELATIVE_CUE_ONECUE) {
            //if onecue, just seek to the regular cue
            emit(seekAbs(m_pCurrentTrack->getCuePoint()));
            return;
        }

        double distance = 0;
        int nearest_playpos = -1;

        QList<Cue*> cuePoints = m_pCurrentTrack->getCuePoints();
        QListIterator<Cue*> it(cuePoints);
        while (it.hasNext()) {
            Cue* pCue = it.next();
            if (pCue->getType() != Cue::CUE || pCue->getHotCue() == -1) {
                continue;
            }

            int cue_position = pCue->getPosition();
            //pick cues closest to new_playpos
            if ((nearest_playpos == -1) ||
                (fabs(new_playpos - cue_position) < distance)) {
                nearest_playpos = cue_position;
                distance = fabs(new_playpos - cue_position);
            }
        }

        if (nearest_playpos == -1) {
            if (new_playpos >= 0) {
                //never found an appropriate cue, so don't seek?
                return;
            }
            //if negative, allow a seek by falling down to the bottom
        } else {
            emit(seekAbs(nearest_playpos));
            return;
        }
    }

    // just seek where it wanted to originally
    emit(seek(change));
}

bool VinylControlControl::isEnabled()
{
    return m_pControlVinylEnabled->get();
}

bool VinylControlControl::isScratching()
{
    return m_pControlVinylScratching->get();
}
