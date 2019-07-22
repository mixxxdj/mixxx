// cuecontrol.cpp
// Created 11/5/2009 by RJ Ryan (rryan@mit.edu)

#include <QMutexLocker>
#include <QStringBuilder>

#include "engine/enginebuffer.h"
#include "engine/controls/cuecontrol.h"

#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "control/controlindicator.h"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "util/sample.h"
#include "util/color/color.h"

// TODO: Convert these doubles to a standard enum
// and convert elseif logic to switch statements
static const double CUE_MODE_MIXXX = 0.0;
static const double CUE_MODE_PIONEER = 1.0;
static const double CUE_MODE_DENON = 2.0;
static const double CUE_MODE_NUMARK = 3.0;
static const double CUE_MODE_MIXXX_NO_BLINK = 4.0;
static const double CUE_MODE_CUP = 5.0;

CueControl::CueControl(QString group,
                       UserSettingsPointer pConfig) :
        EngineControl(group, pConfig),
        m_bPreviewing(false),
        // m_pPlay->toBoo() -> engine play state
        // m_pPlay->set(1.0) -> emulate play button press
        m_pPlay(ControlObject::getControl(ConfigKey(group, "play"))),
        m_pStopButton(ControlObject::getControl(ConfigKey(group, "stop"))),
        m_iCurrentlyPreviewingHotcues(0),
        m_bypassCueSetByPlay(false),
        m_iNumHotCues(NUM_HOT_CUES),
        m_pLoadedTrack(),
        m_mutex(QMutex::Recursive) {
    // To silence a compiler warning about CUE_MODE_PIONEER.
    Q_UNUSED(CUE_MODE_PIONEER);
    createControls();

    m_pTrackSamples = ControlObject::getControl(ConfigKey(group, "track_samples"));

    m_pQuantizeEnabled = ControlObject::getControl(ConfigKey(group, "quantize"));
    connect(m_pQuantizeEnabled, &ControlObject::valueChanged,
            this, &CueControl::quantizeChanged,
            Qt::DirectConnection);

    m_pPrevBeat = ControlObject::getControl(ConfigKey(group, "beat_prev"));
    m_pNextBeat = ControlObject::getControl(ConfigKey(group, "beat_next"));
    m_pClosestBeat = ControlObject::getControl(ConfigKey(group, "beat_closest"));

    m_pCuePoint = new ControlObject(ConfigKey(group, "cue_point"));
    m_pCuePoint->set(-1.0);

    m_pCueMode = new ControlObject(ConfigKey(group, "cue_mode"));

    m_pSeekOnLoadMode = new ControlObject(ConfigKey(group, "seekonload_mode"));

    m_pCueSet = new ControlPushButton(ConfigKey(group, "cue_set"));
    m_pCueSet->setButtonMode(ControlPushButton::TRIGGER);
    connect(m_pCueSet, &ControlObject::valueChanged,
            this, &CueControl::cueSet,
            Qt::DirectConnection);

    m_pCueClear = new ControlPushButton(ConfigKey(group, "cue_clear"));
    m_pCueClear->setButtonMode(ControlPushButton::TRIGGER);
    connect(m_pCueClear, &ControlObject::valueChanged,
            this, &CueControl::cueClear,
            Qt::DirectConnection);

    m_pCueGoto = new ControlPushButton(ConfigKey(group, "cue_goto"));
    connect(m_pCueGoto, &ControlObject::valueChanged,
            this, &CueControl::cueGoto,
            Qt::DirectConnection);

    m_pCueGotoAndPlay =
            new ControlPushButton(ConfigKey(group, "cue_gotoandplay"));
    connect(m_pCueGotoAndPlay, &ControlObject::valueChanged,
            this, &CueControl::cueGotoAndPlay,
            Qt::DirectConnection);

    m_pCuePlay =
            new ControlPushButton(ConfigKey(group, "cue_play"));
    connect(m_pCuePlay, &ControlObject::valueChanged,
            this, &CueControl::cuePlay,
            Qt::DirectConnection);

    m_pCueGotoAndStop =
            new ControlPushButton(ConfigKey(group, "cue_gotoandstop"));
    connect(m_pCueGotoAndStop, &ControlObject::valueChanged,
            this, &CueControl::cueGotoAndStop,
            Qt::DirectConnection);

    m_pCuePreview = new ControlPushButton(ConfigKey(group, "cue_preview"));
    connect(m_pCuePreview, &ControlObject::valueChanged,
            this, &CueControl::cuePreview,
            Qt::DirectConnection);

    m_pCueCDJ = new ControlPushButton(ConfigKey(group, "cue_cdj"));
    connect(m_pCueCDJ, &ControlObject::valueChanged,
            this, &CueControl::cueCDJ,
            Qt::DirectConnection);

    m_pCueDefault = new ControlPushButton(ConfigKey(group, "cue_default"));
    connect(m_pCueDefault, &ControlObject::valueChanged,
            this, &CueControl::cueDefault,
            Qt::DirectConnection);

    m_pPlayStutter = new ControlPushButton(ConfigKey(group, "play_stutter"));
    connect(m_pPlayStutter, &ControlObject::valueChanged,
            this, &CueControl::playStutter,
            Qt::DirectConnection);

    m_pCueIndicator = new ControlIndicator(ConfigKey(group, "cue_indicator"));
    m_pPlayIndicator = new ControlIndicator(ConfigKey(group, "play_indicator"));

    m_pIntroStartPosition = new ControlObject(ConfigKey(group, "intro_start_position"));
    m_pIntroStartPosition->set(-1.0);

    m_pIntroStartEnabled = new ControlObject(ConfigKey(group, "intro_start_enabled"));
    m_pIntroStartEnabled->setReadOnly();

    m_pIntroStartSet = new ControlPushButton(ConfigKey(group, "intro_start_set"));
    connect(m_pIntroStartSet, &ControlObject::valueChanged,
            this, &CueControl::introStartSet,
            Qt::DirectConnection);

    m_pIntroStartClear = new ControlPushButton(ConfigKey(group, "intro_start_clear"));
    connect(m_pIntroStartClear, &ControlObject::valueChanged,
            this, &CueControl::introStartClear,
            Qt::DirectConnection);

    m_pIntroStartActivate = new ControlPushButton(ConfigKey(group, "intro_start_activate"));
    connect(m_pIntroStartActivate, &ControlObject::valueChanged,
            this, &CueControl::introStartActivate,
            Qt::DirectConnection);

    m_pIntroEndPosition = new ControlObject(ConfigKey(group, "intro_end_position"));
    m_pIntroEndPosition->set(-1.0);

    m_pIntroEndEnabled = new ControlObject(ConfigKey(group, "intro_end_enabled"));
    m_pIntroEndEnabled->setReadOnly();

    m_pIntroEndSet = new ControlPushButton(ConfigKey(group, "intro_end_set"));
    connect(m_pIntroEndSet, &ControlObject::valueChanged,
            this, &CueControl::introEndSet,
            Qt::DirectConnection);

    m_pIntroEndClear = new ControlPushButton(ConfigKey(group, "intro_end_clear"));
    connect(m_pIntroEndClear, &ControlObject::valueChanged,
            this, &CueControl::introEndClear,
            Qt::DirectConnection);

    m_pIntroEndActivate = new ControlPushButton(ConfigKey(group, "intro_end_activate"));
    connect(m_pIntroEndActivate, &ControlObject::valueChanged,
            this, &CueControl::introEndActivate,
            Qt::DirectConnection);

    m_pOutroStartPosition = new ControlObject(ConfigKey(group, "outro_start_position"));
    m_pOutroStartPosition->set(-1.0);

    m_pOutroStartEnabled = new ControlObject(ConfigKey(group, "outro_start_enabled"));
    m_pOutroStartEnabled->setReadOnly();

    m_pOutroStartSet = new ControlPushButton(ConfigKey(group, "outro_start_set"));
    connect(m_pOutroStartSet, &ControlObject::valueChanged,
            this, &CueControl::outroStartSet,
            Qt::DirectConnection);

    m_pOutroStartClear = new ControlPushButton(ConfigKey(group, "outro_start_clear"));
    connect(m_pOutroStartClear, &ControlObject::valueChanged,
            this, &CueControl::outroStartClear,
            Qt::DirectConnection);

    m_pOutroStartActivate = new ControlPushButton(ConfigKey(group, "outro_start_activate"));
    connect(m_pOutroStartActivate, &ControlObject::valueChanged,
            this, &CueControl::outroStartActivate,
            Qt::DirectConnection);

    m_pOutroEndPosition = new ControlObject(ConfigKey(group, "outro_end_position"));
    m_pOutroEndPosition->set(-1.0);

    m_pOutroEndEnabled = new ControlObject(ConfigKey(group, "outro_end_enabled"));
    m_pOutroEndEnabled->setReadOnly();

    m_pOutroEndSet = new ControlPushButton(ConfigKey(group, "outro_end_set"));
    connect(m_pOutroEndSet, &ControlObject::valueChanged,
            this, &CueControl::outroEndSet,
            Qt::DirectConnection);

    m_pOutroEndClear = new ControlPushButton(ConfigKey(group, "outro_end_clear"));
    connect(m_pOutroEndClear, &ControlObject::valueChanged,
            this, &CueControl::outroEndClear,
            Qt::DirectConnection);

    m_pOutroEndActivate = new ControlPushButton(ConfigKey(group, "outro_end_activate"));
    connect(m_pOutroEndActivate, &ControlObject::valueChanged,
            this, &CueControl::outroEndActivate,
            Qt::DirectConnection);

    m_pVinylControlEnabled = new ControlProxy(group, "vinylcontrol_enabled");
    m_pVinylControlMode = new ControlProxy(group, "vinylcontrol_mode");
}

CueControl::~CueControl() {
    delete m_pCuePoint;
    delete m_pCueMode;
    delete m_pSeekOnLoadMode;
    delete m_pCueSet;
    delete m_pCueClear;
    delete m_pCueGoto;
    delete m_pCueGotoAndPlay;
    delete m_pCuePlay;
    delete m_pCueGotoAndStop;
    delete m_pCuePreview;
    delete m_pCueCDJ;
    delete m_pCueDefault;
    delete m_pPlayStutter;
    delete m_pCueIndicator;
    delete m_pPlayIndicator;
    delete m_pIntroStartPosition;
    delete m_pIntroStartEnabled;
    delete m_pIntroStartSet;
    delete m_pIntroStartClear;
    delete m_pIntroStartActivate;
    delete m_pIntroEndPosition;
    delete m_pIntroEndEnabled;
    delete m_pIntroEndSet;
    delete m_pIntroEndClear;
    delete m_pIntroEndActivate;
    delete m_pOutroStartPosition;
    delete m_pOutroStartEnabled;
    delete m_pOutroStartSet;
    delete m_pOutroStartClear;
    delete m_pOutroStartActivate;
    delete m_pOutroEndPosition;
    delete m_pOutroEndEnabled;
    delete m_pOutroEndSet;
    delete m_pOutroEndClear;
    delete m_pOutroEndActivate;
    delete m_pVinylControlEnabled;
    delete m_pVinylControlMode;
    qDeleteAll(m_hotcueControls);
}

void CueControl::createControls() {
    for (int i = 0; i < m_iNumHotCues; ++i) {
        HotcueControl* pControl = new HotcueControl(getGroup(), i);

        connect(pControl, &HotcueControl::hotcuePositionChanged,
                this, &CueControl::hotcuePositionChanged,
                Qt::DirectConnection);
        connect(pControl, &HotcueControl::hotcueSet,
                this, &CueControl::hotcueSet,
                Qt::DirectConnection);
        connect(pControl, &HotcueControl::hotcueGoto,
                this, &CueControl::hotcueGoto,
                Qt::DirectConnection);
        connect(pControl, &HotcueControl::hotcueGotoAndPlay,
                this, &CueControl::hotcueGotoAndPlay,
                Qt::DirectConnection);
        connect(pControl, &HotcueControl::hotcueGotoAndStop,
                this, &CueControl::hotcueGotoAndStop,
                Qt::DirectConnection);
        connect(pControl, &HotcueControl::hotcueActivate,
                this, &CueControl::hotcueActivate,
                Qt::DirectConnection);
        connect(pControl, &HotcueControl::hotcueActivatePreview,
                this, &CueControl::hotcueActivatePreview,
                Qt::DirectConnection);
        connect(pControl, &HotcueControl::hotcueClear,
                this, &CueControl::hotcueClear,
                Qt::DirectConnection);

        m_hotcueControls.append(pControl);
    }
}

void CueControl::attachCue(CuePointer pCue, int hotCue) {
    HotcueControl* pControl = m_hotcueControls.value(hotCue, NULL);
    if (pControl == NULL) {
        return;
    }
    if (pControl->getCue() != NULL) {
        detachCue(pControl->getHotcueNumber());
    }
    connect(pCue.get(), &Cue::updated,
            this, &CueControl::cueUpdated,
            Qt::DirectConnection);

    pControl->setCue(pCue);

}

void CueControl::detachCue(int hotCue) {
    HotcueControl* pControl = m_hotcueControls.value(hotCue, NULL);
    if (pControl == NULL) {
        return;
    }
    CuePointer pCue(pControl->getCue());
    if (!pCue)
        return;
    disconnect(pCue.get(), 0, this, 0);
    pControl->resetCue();
}

void CueControl::trackLoaded(TrackPointer pNewTrack) {
    QMutexLocker lock(&m_mutex);
    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(), 0, this, 0);
        for (int i = 0; i < m_iNumHotCues; ++i) {
            detachCue(i);
        }

        m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
        m_pCuePoint->set(-1.0);
        m_pIntroStartPosition->set(-1.0);
        m_pIntroStartEnabled->forceSet(0.0);
        m_pIntroEndPosition->set(-1.0);
        m_pIntroEndEnabled->forceSet(0.0);
        m_pOutroStartPosition->set(-1.0);
        m_pOutroStartEnabled->forceSet(0.0);
        m_pOutroEndPosition->set(-1.0);
        m_pOutroEndEnabled->forceSet(0.0);
        m_pLoadedTrack.reset();
    }

    if (!pNewTrack) {
        return;
    }
    m_pLoadedTrack = pNewTrack;

    connect(m_pLoadedTrack.get(), &Track::cuesUpdated,
            this, &CueControl::trackCuesUpdated,
            Qt::DirectConnection);

    connect(m_pLoadedTrack.get(), &Track::beatsUpdated,
            this, &CueControl::trackBeatsUpdated,
            Qt::DirectConnection);

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    // Update COs with cues from track.
    loadCuesFromTrack();

    // Seek track according to SeekOnLoadMode.
    SeekOnLoadMode seekOnLoadMode = getSeekOnLoadMode();
    switch (seekOnLoadMode) {
    case SEEK_ON_LOAD_ZERO_POS:
        seekExact(0.0);
        break;
    case SEEK_ON_LOAD_MAIN_CUE:
        seekExact(m_pCuePoint->get());
        break;
    case SEEK_ON_LOAD_INTRO_CUE:
        seekExact(m_pIntroStartPosition->get());
        break;
    default:
        // Respect cue recall preference option.
        if (isCueRecallEnabled() && m_pCuePoint->get() != -1.0) {
            // If cue recall is ON and main cue point is set, seek to it.
            seekExact(m_pCuePoint->get());
        } else if (!(m_pVinylControlEnabled->get() &&
                     m_pVinylControlMode->get() == MIXXX_VCMODE_ABSOLUTE)) {
            // Otherwise, seek to zero unless vinylcontrol is on and
            // set to absolute. This allows users to load tracks and
            // have the needle-drop be maintained.
            seekExact(0.0);
        }
        break;
    }
}

void CueControl::cueUpdated() {
    //QMutexLocker lock(&m_mutex);
    // We should get a trackCuesUpdated call anyway, so do nothing.
}

void CueControl::loadCuesFromTrack() {
    QMutexLocker lock(&m_mutex);
    QSet<int> active_hotcues;
    CuePointer pLoadCue, pIntroCue, pOutroCue;

    if (!m_pLoadedTrack)
        return;

    for (const CuePointer& pCue: m_pLoadedTrack->getCuePoints()) {
        if (pCue->getType() == Cue::LOAD) {
            DEBUG_ASSERT(!pLoadCue);  // There should be only one LOAD cue
            pLoadCue = pCue;
        } else if (pCue->getType() == Cue::INTRO) {
            DEBUG_ASSERT(!pIntroCue);  // There should be only one INTRO cue
            pIntroCue = pCue;
        } else if (pCue->getType() == Cue::OUTRO) {
            DEBUG_ASSERT(!pOutroCue);  // There should be only one OUTRO cue
            pOutroCue = pCue;
        } else if (pCue->getType() == Cue::CUE && pCue->getHotCue() != -1) {
            int hotcue = pCue->getHotCue();
            HotcueControl* pControl = m_hotcueControls.value(hotcue, NULL);

            // Cue's hotcue doesn't have a hotcue control.
            if (pControl == NULL) {
                continue;
            }

            CuePointer pOldCue(pControl->getCue());

            // If the old hotcue is different than this one.
            if (pOldCue != pCue) {
                // If the old hotcue exists, detach it
                if (pOldCue) {
                    detachCue(hotcue);
                }
                attachCue(pCue, hotcue);
            } else {
                // If the old hotcue is the same, then we only need to update
                pControl->setPosition(pCue->getPosition());
                pControl->setColor(pCue->getColor());
            }
            // Add the hotcue to the list of active hotcues
            active_hotcues.insert(hotcue);
        }
    }

    if (pLoadCue) {
        double position = pLoadCue->getPosition();
        Cue::CueSource source = pLoadCue->getSource();

        m_pCuePoint->set(quantizeCuePoint(position, source, QuantizeMode::ClosestBeat));
    } else {
        m_pCuePoint->set(-1.0);
    }

    if (pIntroCue) {
        double startPosition = pIntroCue->getPosition();
        double endPosition = pIntroCue->getEndPosition();
        Cue::CueSource source = pIntroCue->getSource();

        m_pIntroStartPosition->set(quantizeCuePoint(startPosition, source, QuantizeMode::PreviousBeat));
        m_pIntroStartEnabled->forceSet(startPosition == -1.0 ? 0.0 : 1.0);
        m_pIntroEndPosition->set(quantizeCuePoint(endPosition, source, QuantizeMode::NextBeat));
        m_pIntroEndEnabled->forceSet(endPosition == -1.0 ? 0.0 : 1.0);
    } else {
        m_pIntroStartPosition->set(-1.0);
        m_pIntroStartEnabled->forceSet(0.0);
        m_pIntroEndPosition->set(-1.0);
        m_pIntroEndEnabled->forceSet(0.0);
    }

    if (pOutroCue) {
        double startPosition = pOutroCue->getPosition();
        double endPosition = pOutroCue->getEndPosition();
        Cue::CueSource source = pOutroCue->getSource();

        m_pOutroStartPosition->set(quantizeCuePoint(startPosition, source, QuantizeMode::PreviousBeat));
        m_pOutroStartEnabled->forceSet(startPosition == -1.0 ? 0.0 : 1.0);
        m_pOutroEndPosition->set(quantizeCuePoint(endPosition, source, QuantizeMode::NextBeat));
        m_pOutroEndEnabled->forceSet(endPosition == -1.0 ? 0.0 : 1.0);
    } else {
        m_pOutroStartPosition->set(-1.0);
        m_pOutroStartEnabled->forceSet(0.0);
        m_pOutroEndPosition->set(-1.0);
        m_pOutroEndEnabled->forceSet(0.0);
    }

    // Detach all hotcues that are no longer present
    for (int i = 0; i < m_iNumHotCues; ++i) {
        if (!active_hotcues.contains(i)) {
            detachCue(i);
        }
    }
}

void CueControl::reloadCuesFromTrack() {
    if (!m_pLoadedTrack)
        return;

    // Determine current playing position of the track.
    TrackAt trackAt = getTrackAt();
    bool wasTrackAtZeroPos = isTrackAtZeroPos();
    bool wasTrackAtIntroCue = isTrackAtIntroCue();

    // Update COs with cues from track.
    loadCuesFromTrack();

    // Retrieve current position of cues from COs.
    double cue = m_pCuePoint->get();
    double intro = m_pIntroStartPosition->get();

    // Make track follow the updated cues.
    SeekOnLoadMode seekOnLoadMode = getSeekOnLoadMode();
    if (seekOnLoadMode == SEEK_ON_LOAD_DEFAULT) {
        if ((trackAt == TrackAt::Cue || wasTrackAtZeroPos) && cue != -1.0 && isCueRecallEnabled()) {
            seekExact(cue);
        }
    } else if (seekOnLoadMode == SEEK_ON_LOAD_MAIN_CUE) {
        if ((trackAt == TrackAt::Cue || wasTrackAtZeroPos) && cue != -1.0) {
            seekExact(cue);
        }
    } else if (seekOnLoadMode == SEEK_ON_LOAD_INTRO_CUE) {
        if ((wasTrackAtIntroCue || wasTrackAtZeroPos) && intro != -1.0) {
            seekExact(intro);
        }
    }
}

void CueControl::trackCuesUpdated() {
    reloadCuesFromTrack();
}

void CueControl::trackBeatsUpdated() {
    reloadCuesFromTrack();
}

void CueControl::quantizeChanged(double v) {
    Q_UNUSED(v);

    reloadCuesFromTrack();
}

void CueControl::hotcueSet(HotcueControl* pControl, double v) {
    //qDebug() << "CueControl::hotcueSet" << v;

    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    int hotcue = pControl->getHotcueNumber();
    // Note: the cue is just detached from the hotcue control
    // It remains in the database for later use
    // TODO: find a rule, that allows us to delete the cue as well
    // https://bugs.launchpad.net/mixxx/+bug/1653276
    hotcueClear(pControl, v);

    CuePointer pCue(m_pLoadedTrack->createAndAddCue());
    double closestBeat = m_pClosestBeat->get();
    double cuePosition =
            (m_pQuantizeEnabled->toBool() && closestBeat != -1) ?
                    closestBeat : getSampleOfTrack().current;
    pCue->setPosition(cuePosition);
    pCue->setHotCue(hotcue);
    pCue->setLabel("");
    pCue->setType(Cue::CUE);
    pCue->setSource(Cue::MANUAL);
    // TODO(XXX) deal with spurious signals
    attachCue(pCue, hotcue);

    if (getConfig()->getValue(ConfigKey("[Controls]", "auto_hotcue_colors"), false)) {
        const QList<PredefinedColorPointer> predefinedColors = Color::kPredefinedColorsSet.allColors;
        pCue->setColor(predefinedColors.at((hotcue % (predefinedColors.count() - 1)) + 1));
    };

    // If quantize is enabled and we are not playing, jump to the cue point
    // since it's not necessarily where we currently are. TODO(XXX) is this
    // potentially invalid for vinyl control?
    bool playing = m_pPlay->toBool();
    if (!playing && m_pQuantizeEnabled->toBool()) {
        lock.unlock();  // prevent deadlock.
        // Enginebuffer will quantize more exactly than we can.
        seekAbs(cuePosition);
    }
}

void CueControl::hotcueGoto(HotcueControl* pControl, double v) {
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue(pControl->getCue());

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (pCue) {
        int position = pCue->getPosition();
        if (position != -1) {
            seekAbs(position);
        }
    }
}

void CueControl::hotcueGotoAndStop(HotcueControl* pControl, double v) {
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    CuePointer pCue(pControl->getCue());

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (pCue) {
        int position = pCue->getPosition();
        if (position != -1) {
            m_pPlay->set(0.0);
            seekExact(position);
        }
    }
}

void CueControl::hotcueGotoAndPlay(HotcueControl* pControl, double v) {
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue(pControl->getCue());

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (pCue) {
        int position = pCue->getPosition();
        if (position != -1) {
            seekAbs(position);
            if (!isPlayingByPlayButton()) {
                // cueGoto is processed asynchrony.
                // avoid a wrong cue set if seek by cueGoto is still pending
                m_bPreviewing = false;
                m_iCurrentlyPreviewingHotcues = 0;
                // don't move the cue point to the hot cue point in DENON mode
                m_bypassCueSetByPlay = true;
                m_pPlay->set(1.0);
            }
        }
    }
}

void CueControl::hotcueActivate(HotcueControl* pControl, double v) {
    //qDebug() << "CueControl::hotcueActivate" << v;

    QMutexLocker lock(&m_mutex);

    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue(pControl->getCue());

    lock.unlock();

    if (pCue) {
        if (v) {
            if (pCue->getPosition() == -1) {
                hotcueSet(pControl, v);
            } else {
                if (isPlayingByPlayButton()) {
                    hotcueGoto(pControl, v);
                } else {
                    hotcueActivatePreview(pControl, v);
                }
            }
        } else {
            if (pCue->getPosition() != -1) {
                hotcueActivatePreview(pControl, v);
            }
        }
    } else {
        if (v) {
            // just in case
            hotcueSet(pControl, v);
        } else if (m_iCurrentlyPreviewingHotcues) {
            // The cue is non-existent, yet we got a release for it and are
            // currently previewing a hotcue. This is indicative of a corner
            // case where the cue was detached while we were pressing it. Let
            // hotcueActivatePreview handle it.
            hotcueActivatePreview(pControl, v);
        }
    }
}

void CueControl::hotcueActivatePreview(HotcueControl* pControl, double v) {
    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack) {
        return;
    }
    CuePointer pCue(pControl->getCue());

    if (v) {
        if (pCue && pCue->getPosition() != -1) {
            m_iCurrentlyPreviewingHotcues++;
            double position = pCue->getPosition();
            m_bypassCueSetByPlay = true;
            m_pPlay->set(1.0);
            pControl->setPreviewing(true);
            pControl->setPreviewingPosition(position);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(position);
        }
    } else if (m_iCurrentlyPreviewingHotcues) {
        // This is a activate release and we are previewing at least one
        // hotcue. If this hotcue is previewing:
        if (pControl->isPreviewing()) {
            // Mark this hotcue as not previewing.
            double position = pControl->getPreviewingPosition();
            pControl->setPreviewing(false);
            pControl->setPreviewingPosition(-1);

            // If this is the last hotcue to leave preview.
            if (--m_iCurrentlyPreviewingHotcues == 0 && !m_bPreviewing) {
                m_pPlay->set(0.0);
                // Need to unlock before emitting any signals to prevent deadlock.
                lock.unlock();
                seekExact(position);
            }
        }
    }
}

void CueControl::hotcueClear(HotcueControl* pControl, double v) {
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue(pControl->getCue());
    detachCue(pControl->getHotcueNumber());
    m_pLoadedTrack->removeCue(pCue);
}

void CueControl::hotcuePositionChanged(HotcueControl* pControl, double newPosition) {
    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    CuePointer pCue(pControl->getCue());
    if (pCue) {
        // Setting the position to -1 is the same as calling hotcue_x_clear
        if (newPosition == -1) {
            pCue->setHotCue(-1);
            detachCue(pControl->getHotcueNumber());
        } else if (newPosition > 0 && newPosition < m_pTrackSamples->get()) {
            pCue->setPosition(newPosition);
        }
    }
}

void CueControl::hintReader(HintVector* pHintList) {
    Hint cue_hint;
    double cuePoint = m_pCuePoint->get();
    if (cuePoint >= 0) {
        cue_hint.frame = SampleUtil::floorPlayPosToFrame(m_pCuePoint->get());
        cue_hint.frameCount = Hint::kFrameCountForward;
        cue_hint.priority = 10;
        pHintList->append(cue_hint);
    }

    // this is called from the engine thread
    // it is no locking required, because m_hotcueControl is filled during the
    // constructor and getPosition()->get() is a ControlObject
    for (const auto& pControl: m_hotcueControls) {
        double position = pControl->getPosition();
        if (position != -1) {
            cue_hint.frame = SampleUtil::floorPlayPosToFrame(position);
            cue_hint.frameCount = Hint::kFrameCountForward;
            cue_hint.priority = 10;
            pHintList->append(cue_hint);
        }
    }
}

// Moves the cue point to current position or to closest beat in case
// quantize is enabled
void CueControl::cueSet(double v) {
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    double closestBeat = m_pClosestBeat->get();
    double cue = (m_pQuantizeEnabled->toBool() && closestBeat != -1) ?
            closestBeat : getSampleOfTrack().current;
    m_pCuePoint->set(cue);
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Store cue point in loaded track
    if (pLoadedTrack) {
        pLoadedTrack->setCuePoint(CuePosition(cue, Cue::MANUAL));
    }
}

void CueControl::cueClear(double v) {
    if (!v) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    m_pCuePoint->set(-1.0);
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        pLoadedTrack->setCuePoint(CuePosition());
    }
}

void CueControl::cueGoto(double v)
{
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    // Seek to cue point
    double cuePoint = m_pCuePoint->get();

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    seekAbs(cuePoint);
}

void CueControl::cueGotoAndPlay(double v)
{
    if (!v) return;
    cueGoto(v);
    QMutexLocker lock(&m_mutex);
    // Start playing if not already
    if (!isPlayingByPlayButton()) {
        // cueGoto is processed asynchrony.
        // avoid a wrong cue set if seek by cueGoto is still pending
        m_bPreviewing = false;
        m_iCurrentlyPreviewingHotcues = 0;
        m_bypassCueSetByPlay = true;
        m_pPlay->set(1.0);
    }
}

void CueControl::cueGotoAndStop(double v)
{
    if (!v)
        return;

    QMutexLocker lock(&m_mutex);
    m_pPlay->set(0.0);
    double cuePoint = m_pCuePoint->get();

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    seekExact(cuePoint);
}

void CueControl::cuePreview(double v)
{
    QMutexLocker lock(&m_mutex);

    if (v) {
        m_bPreviewing = true;
        m_bypassCueSetByPlay = true;
        m_pPlay->set(1.0);
    } else if (!v && m_bPreviewing) {
        m_bPreviewing = false;
        if (!m_iCurrentlyPreviewingHotcues) {
            m_pPlay->set(0.0);
        } else {
            return;
        }
    } else {
        return;
    }

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    seekAbs(m_pCuePoint->get());
}

void CueControl::cueCDJ(double v) {
    // This is how Pioneer cue buttons work:
    // If pressed while freely playing (i.e. playing and platter NOT being touched), stop playback and go to cue.
    // If pressed while NOT freely playing (i.e. stopped or playing but platter IS being touched), set new cue point.
    // If pressed while stopped and at cue, play while pressed.
    // If play is pressed while holding cue, the deck is now playing. (Handled in playFromCuePreview().)

    QMutexLocker lock(&m_mutex);
    const auto freely_playing = m_pPlay->toBool() && !getEngineBuffer()->getScratching();
    TrackAt trackAt = getTrackAt();

    if (v) {
        if (m_iCurrentlyPreviewingHotcues) {
            // we are already previewing by hotcues
            // just jump to cue point and continue previewing
            m_bPreviewing = true;
            lock.unlock();
            seekAbs(m_pCuePoint->get());
        } else if (freely_playing || trackAt == TrackAt::End) {
            // Jump to cue when playing or when at end position

            // Just in case.
            m_bPreviewing = false;
            m_pPlay->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        } else if (trackAt == TrackAt::Cue) {
            // pause at cue point
            m_bPreviewing = true;
            m_pPlay->set(1.0);
        } else {
            // Pause not at cue point and not at end position
            cueSet(v);
            // Just in case.
            m_bPreviewing = false;
            m_pPlay->set(0.0);

            // If quantize is enabled, jump to the cue point since it's not
            // necessarily where we currently are
            if (m_pQuantizeEnabled->toBool()) {
                lock.unlock();  // prevent deadlock.
                // Enginebuffer will quantize more exactly than we can.
                seekAbs(m_pCuePoint->get());
            }
        }
    } else if (m_bPreviewing) {
        m_bPreviewing = false;
        if (!m_iCurrentlyPreviewingHotcues) {
            m_pPlay->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        }
    }
    // indicator may flash because the delayed adoption of seekAbs
    // Correct the Indicator set via play
    if (m_pLoadedTrack && !freely_playing) {
        m_pCueIndicator->setBlinkValue(ControlIndicator::ON);
    } else {
        m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
    }
}

void CueControl::cueDenon(double v) {
    // This is how Denon DN-S 3700 cue buttons work:
    // If pressed go to cue and stop.
    // If pressed while stopped and at cue, play while pressed.
    // Cue Point is moved by play from pause

    QMutexLocker lock(&m_mutex);
    bool playing = (m_pPlay->toBool());
    TrackAt trackAt = getTrackAt();

    if (v) {
        if (m_iCurrentlyPreviewingHotcues) {
            // we are already previewing by hotcues
            // just jump to cue point and continue previewing
            m_bPreviewing = true;
            lock.unlock();
            seekAbs(m_pCuePoint->get());
        } else if (!playing && trackAt == TrackAt::Cue) {
            // pause at cue point
            m_bPreviewing = true;
            m_pPlay->set(1.0);
        } else {
            // Just in case.
            m_bPreviewing = false;
            m_pPlay->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        }
    } else if (m_bPreviewing) {
        m_bPreviewing = false;
        if (!m_iCurrentlyPreviewingHotcues) {
            m_pPlay->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        }
    }
}

void CueControl::cuePlay(double v) {
    // This is how CUP button works:
    // If freely playing (i.e. playing and platter NOT being touched), press to go to cue and stop.
    // If not freely playing (i.e. stopped or platter IS being touched), press to go to cue and stop.
    // On release, start playing from cue point.


    QMutexLocker lock(&m_mutex);
    const auto freely_playing = m_pPlay->toBool() && !getEngineBuffer()->getScratching();
    TrackAt trackAt = getTrackAt();

    // pressed
    if (v) {
        if (freely_playing) {
            m_bPreviewing = false;
            m_pPlay->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        } else if (trackAt == TrackAt::ElseWhere) {
            // Pause not at cue point and not at end position
            cueSet(v);
            // Just in case.
            m_bPreviewing = false;
            m_pPlay->set(0.0);
            // If quantize is enabled, jump to the cue point since it's not
            // necessarily where we currently are
            if (m_pQuantizeEnabled->toBool()) {
                lock.unlock();  // prevent deadlock.
                // Enginebuffer will quantize more exactly than we can.
                seekAbs(m_pCuePoint->get());
            }
        }
    } else if (trackAt == TrackAt::Cue){
        m_bPreviewing = false;
        m_pPlay->set(1.0);
        lock.unlock();
    }
}

void CueControl::cueDefault(double v) {
    double cueMode = m_pCueMode->get();
    // Decide which cue implementation to call based on the user preference
    if (cueMode == CUE_MODE_DENON || cueMode == CUE_MODE_NUMARK) {
        cueDenon(v);
    } else if (cueMode == CUE_MODE_CUP) {
        cuePlay(v);
    } else {
        // The modes CUE_MODE_PIONEER and CUE_MODE_MIXXX are similar
        // are handled inside cueCDJ(v)
        // default to Pioneer mode
        cueCDJ(v);
    }
}

void CueControl::pause(double v) {
    QMutexLocker lock(&m_mutex);
    //qDebug() << "CueControl::pause()" << v;
    if (v != 0.0) {
        m_pPlay->set(0.0);
    }
}

void CueControl::playStutter(double v) {
    QMutexLocker lock(&m_mutex);
    //qDebug() << "playStutter" << v;
    if (v != 0.0) {
        if (isPlayingByPlayButton()) {
            cueGoto(1.0);
        } else {
            m_pPlay->set(1.0);
        }
    }
}

void CueControl::introStartSet(double v) {
    if (!v) {
        return;
    }

    QMutexLocker lock(&m_mutex);

    // Quantize cue point to nearest beat before current position.
    // Fall back to nearest beat after or current position.
    double position = quantizeCurrentPosition(QuantizeMode::PreviousBeat);

    // Make sure user is not trying to place intro start cue on or after
    // other intro/outro cues.
    double introEnd = m_pIntroEndPosition->get();
    double outroStart = m_pOutroStartPosition->get();
    double outroEnd = m_pOutroEndPosition->get();
    if (introEnd != -1.0 && position >= introEnd) {
        qWarning() << "Trying to place intro start cue on or after intro end cue.";
        return;
    }
    if (outroStart != -1.0 && position >= outroStart) {
        qWarning() << "Trying to place intro start cue on or after outro start cue.";
        return;
    }
    if (outroEnd != -1.0 && position >= outroEnd) {
        qWarning() << "Trying to place intro start cue on or after outro end cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(Cue::INTRO);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue();
            pCue->setType(Cue::INTRO);
        }
        pCue->setSource(Cue::MANUAL);
        pCue->setPosition(position);
        pCue->setLength(introEnd != -1.0 ? introEnd - position : 0.0);
    }
}

void CueControl::introStartClear(double v) {
    if (!v) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    double introEnd = m_pIntroEndPosition->get();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(Cue::INTRO);
        if (introEnd != -1.0) {
            pCue->setPosition(-1.0);
            pCue->setLength(introEnd);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::introStartActivate(double v) {
    if (v) {
        QMutexLocker lock(&m_mutex);
        double introStart = m_pIntroStartPosition->get();
        lock.unlock();

        if (introStart == -1.0) {
            introStartSet(1.0);
        } else {
            seekAbs(introStart);
        }
    }
}

void CueControl::introEndSet(double v) {
    if (!v) {
        return;
    }

    QMutexLocker lock(&m_mutex);

    // Quantize cue point to nearest beat after current position.
    // Fall back to nearest beat before or current position.
    double position = quantizeCurrentPosition(QuantizeMode::NextBeat);

    // Make sure user is not trying to place intro end cue on or before
    // intro start cue, or on or after outro start/end cue.
    double introStart = m_pIntroStartPosition->get();
    double outroStart = m_pOutroStartPosition->get();
    double outroEnd = m_pOutroEndPosition->get();
    if (introStart != -1.0 && position <= introStart) {
        qWarning() << "Trying to place intro end cue on or before intro start cue.";
        return;
    }
    if (outroStart != -1.0 && position >= outroStart) {
        qWarning() << "Trying to place intro end cue on or after outro start cue.";
        return;
    }
    if (outroEnd != -1.0 && position >= outroEnd) {
        qWarning() << "Trying to place intro end cue on or after outro end cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(Cue::INTRO);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue();
            pCue->setType(Cue::INTRO);
        }
        pCue->setSource(Cue::MANUAL);
        if (introStart != -1.0) {
            pCue->setPosition(introStart);
            pCue->setLength(position - introStart);
        } else {
            pCue->setPosition(-1.0);
            pCue->setLength(position);
        }
    }
}

void CueControl::introEndClear(double v) {
    if (!v) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    double introStart = m_pIntroStartPosition->get();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(Cue::INTRO);
        if (introStart != -1.0) {
            pCue->setPosition(introStart);
            pCue->setLength(0.0);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::introEndActivate(double v) {
    if (v) {
        QMutexLocker lock(&m_mutex);
        double introEnd = m_pIntroEndPosition->get();
        lock.unlock();

        if (introEnd == -1.0) {
            introEndSet(1.0);
        } else {
            seekAbs(introEnd);
        }
    }
}

void CueControl::outroStartSet(double v) {
    if (!v) {
        return;
    }

    QMutexLocker lock(&m_mutex);

    // Quantize cue point to nearest beat before current position.
    // Fall back to nearest beat after or current position.
    double position = quantizeCurrentPosition(QuantizeMode::PreviousBeat);

    // Make sure user is not trying to place outro start cue on or before
    // intro end cue or on or after outro end cue.
    double introStart = m_pIntroStartPosition->get();
    double introEnd = m_pIntroEndPosition->get();
    double outroEnd = m_pOutroEndPosition->get();
    if (introStart != -1.0 && position <= introStart) {
        qWarning() << "Trying to place outro start cue on or before intro start cue.";
        return;
    }
    if (introEnd != -1.0 && position <= introEnd) {
        qWarning() << "Trying to place outro start cue on or before intro end cue.";
        return;
    }
    if (outroEnd != -1.0 && position >= outroEnd) {
        qWarning() << "Trying to place outro start cue on or after outro end cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(Cue::OUTRO);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue();
            pCue->setType(Cue::OUTRO);
        }
        pCue->setSource(Cue::MANUAL);
        pCue->setPosition(position);
        pCue->setLength(outroEnd != -1.0 ? outroEnd - position : 0.0);
    }
}

void CueControl::outroStartClear(double v) {
    if (!v) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    double outroEnd = m_pOutroEndPosition->get();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(Cue::OUTRO);
        if (outroEnd != -1.0) {
            pCue->setPosition(-1.0);
            pCue->setLength(outroEnd);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::outroStartActivate(double v) {
    if (v) {
        QMutexLocker lock(&m_mutex);
        double outroStart = m_pOutroStartPosition->get();
        lock.unlock();

        if (outroStart == -1.0) {
            outroStartSet(1.0);
        } else {
            seekAbs(outroStart);
        }
    }
}

void CueControl::outroEndSet(double v) {
    if (!v) {
        return;
    }

    QMutexLocker lock(&m_mutex);

    // Quantize cue point to nearest beat after current position.
    // Fall back to nearest beat before or current position.
    double position = quantizeCurrentPosition(QuantizeMode::NextBeat);

    // Make sure user is not trying to place outro end cue on or before
    // other intro/outro cues.
    double introStart = m_pIntroStartPosition->get();
    double introEnd = m_pIntroEndPosition->get();
    double outroStart = m_pOutroStartPosition->get();
    if (introStart != -1.0 && position <= introStart) {
        qWarning() << "Trying to place outro end cue on or before intro start cue.";
        return;
    }
    if (introEnd != -1.0 && position <= introEnd) {
        qWarning() << "Trying to place outro end cue on or before intro end cue.";
        return;
    }
    if (outroStart != -1.0 && position <= outroStart) {
        qWarning() << "Trying to place outro end cue on or before outro start cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(Cue::OUTRO);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue();
            pCue->setType(Cue::OUTRO);
        }
        pCue->setSource(Cue::MANUAL);
        if (outroStart != -1.0) {
            pCue->setPosition(outroStart);
            pCue->setLength(position - outroStart);
        } else {
            pCue->setPosition(-1.0);
            pCue->setLength(position);
        }
    }
}

void CueControl::outroEndClear(double v) {
    if (!v) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    double outroStart = m_pOutroStartPosition->get();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(Cue::OUTRO);
        if (outroStart != -1.0) {
            pCue->setPosition(outroStart);
            pCue->setLength(0.0);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::outroEndActivate(double v) {
    if (v) {
        QMutexLocker lock(&m_mutex);
        double outroEnd = m_pOutroEndPosition->get();
        lock.unlock();

        if (outroEnd == -1.0) {
            outroEndSet(1.0);
        } else {
            seekAbs(outroEnd);
        }
    }
}

bool CueControl::updateIndicatorsAndModifyPlay(bool newPlay, bool playPossible) {
    //qDebug() << "updateIndicatorsAndModifyPlay" << newPlay << playPossible
    //        << m_iCurrentlyPreviewingHotcues << m_bPreviewing;
    QMutexLocker lock(&m_mutex);
    double cueMode = m_pCueMode->get();
    if ((cueMode == CUE_MODE_DENON || cueMode == CUE_MODE_NUMARK) &&
        newPlay && playPossible &&
        !m_pPlay->toBool() &&
        !m_bypassCueSetByPlay) {
        // in Denon mode each play from pause moves the cue point
        // if not previewing
        cueSet(1.0);
    }
    m_bypassCueSetByPlay = false;

    // when previewing, "play" was set by cue button, a following toggle request
    // (play = 0.0) is used for latching play.
    bool previewing = false;
    if (m_bPreviewing || m_iCurrentlyPreviewingHotcues) {
        if (!newPlay) {
            // play latch request: stop previewing and go into normal play mode.
            m_bPreviewing = false;
            m_iCurrentlyPreviewingHotcues = 0;
            newPlay = true;
        } else {
            previewing = true;
        }
    }

    TrackAt trackAt = getTrackAt();

    if (!playPossible) {
        // play not possible
        newPlay = false;
        m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
        m_pStopButton->set(0.0);
    } else if (newPlay && !previewing) {
        // Play: Indicates a latched Play
        m_pPlayIndicator->setBlinkValue(ControlIndicator::ON);
        m_pStopButton->set(0.0);
    } else {
        // Pause:
        m_pStopButton->set(1.0);
        if (cueMode == CUE_MODE_DENON) {
            if (trackAt == TrackAt::Cue || previewing) {
                m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
            } else {
                // Flashing indicates that a following play would move cue point
                m_pPlayIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
            }
        } else if (cueMode == CUE_MODE_MIXXX || cueMode == CUE_MODE_MIXXX_NO_BLINK ||
                   cueMode == CUE_MODE_NUMARK) {
            m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
        } else {
            // Flashing indicates that play is possible in Pioneer mode
            m_pPlayIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
        }
    }

    if (cueMode != CUE_MODE_DENON && cueMode != CUE_MODE_NUMARK) {
        if (m_pCuePoint->get() != -1) {
            if (newPlay == 0.0 && trackAt == TrackAt::ElseWhere) {
                if (cueMode == CUE_MODE_MIXXX) {
                    // in Mixxx mode Cue Button is flashing slow if CUE will move Cue point
                    m_pCueIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
                } else if (cueMode == CUE_MODE_MIXXX_NO_BLINK) {
                    m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
                } else {
                    // in Pioneer mode Cue Button is flashing fast if CUE will move Cue point
                    m_pCueIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_250MS);
                }
            } else {
                m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
            }
        } else {
            m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
        }
    }
    m_pPlayStutter->set(newPlay ? 1.0 : 0.0);

    return newPlay;
}

// called from the engine thread
void CueControl::updateIndicators() {
    // No need for mutex lock because we are only touching COs.
    double cueMode = m_pCueMode->get();
    TrackAt trackAt = getTrackAt();

    if (cueMode == CUE_MODE_DENON || cueMode == CUE_MODE_NUMARK) {
        // Cue button is only lit at cue point
        bool playing = m_pPlay->toBool();
        if (trackAt == TrackAt::Cue) {
            // at cue point
            if (!playing) {
                m_pCueIndicator->setBlinkValue(ControlIndicator::ON);
                m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
            }
        } else {
            m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
            if (!playing) {
                if (trackAt != TrackAt::End && cueMode != CUE_MODE_NUMARK) {
                    // Play will move cue point
                    m_pPlayIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
                } else {
                    // At track end
                    m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
                }
            }
        }
    } else {
        // Here we have CUE_MODE_PIONEER or CUE_MODE_MIXXX
        // default to Pioneer mode
        if (!m_bPreviewing) {
            const auto freely_playing = m_pPlay->toBool() && !getEngineBuffer()->getScratching();
            if (!freely_playing) {
                switch (trackAt) {
                case TrackAt::ElseWhere:
                    if (cueMode == CUE_MODE_MIXXX) {
                        // in Mixxx mode Cue Button is flashing slow if CUE will move Cue point
                        m_pCueIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
                    } else if (cueMode == CUE_MODE_MIXXX_NO_BLINK) {
                        m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
                    } else {
                        // in Pioneer mode Cue Button is flashing fast if CUE will move Cue point
                        m_pCueIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_250MS);
                    }
                    break;
                case TrackAt::End:
                    // At track end
                    m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
                    break;
                case TrackAt::Cue:
                    // Next Press is preview
                    m_pCueIndicator->setBlinkValue(ControlIndicator::ON);
                    break;
                }
            } else {
                // Cue indicator should be off when freely playing
                m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
            }
        }
    }
}

void CueControl::resetIndicators() {
    m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
    m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
}

CueControl::TrackAt CueControl::getTrackAt() const {
    SampleOfTrack sot = getSampleOfTrack();
    if (sot.current >= sot.total) {
        return TrackAt::End;
    }
    double cue = m_pCuePoint->get();
    if (cue != -1 && fabs(sot.current - cue) < 1.0f) {
        return TrackAt::Cue;
    }
    return TrackAt::ElseWhere;
}

double CueControl::quantizeCurrentPosition(QuantizeMode mode) {
    SampleOfTrack sampleOfTrack = getSampleOfTrack();

    // Don't quantize if quantization is disabled.
    if (!m_pQuantizeEnabled->toBool()) {
        return sampleOfTrack.current;
    }

    if (mode == QuantizeMode::ClosestBeat) {
        double closestBeat = m_pClosestBeat->get();
        return closestBeat != -1.0 ? closestBeat : sampleOfTrack.current;
    }

    double prevBeat = m_pPrevBeat->get();
    double nextBeat = m_pNextBeat->get();

    if (mode == QuantizeMode::PreviousBeat) {
        // Quantize to previous beat, fall back to next beat.
        return prevBeat != -1.0 ? prevBeat : (nextBeat != -1.0 ? nextBeat : sampleOfTrack.current);
    } else if (mode == QuantizeMode::NextBeat) {
        // Quantize to next beat, fall back to previous beat.
        return nextBeat != -1.0 ? nextBeat : (prevBeat != -1.0 ? prevBeat : sampleOfTrack.current);
    } else {
        qWarning() << "PROGRAMMING ERROR: Invalid quantize mode" << static_cast<int>(mode);
        return -1.0;
    }
}

double CueControl::quantizeCuePoint(double position, Cue::CueSource source, QuantizeMode mode) {
    // Don't quantize unset cues, manual cues or when quantization is disabled.
    if (position == -1.0 || source == Cue::MANUAL || !m_pQuantizeEnabled->toBool()) {
        return position;
    }

    BeatsPointer pBeats = m_pLoadedTrack->getBeats();
    if (!pBeats) {
        return position;
    }

    if (mode == QuantizeMode::ClosestBeat) {
        double closestBeat = pBeats->findClosestBeat(position);
        return closestBeat != -1.0 ? closestBeat : position;
    }

    double prevBeat, nextBeat;
    pBeats->findPrevNextBeats(position, &prevBeat, &nextBeat);

    if (mode == QuantizeMode::PreviousBeat) {
        // Quantize to previous beat, fall back to next beat.
        return prevBeat != -1.0 ? prevBeat : (nextBeat != -1.0 ? nextBeat : position);
    } else if (mode == QuantizeMode::NextBeat) {
        // Quantize to next beat, fall back to previous beat.
        return nextBeat != -1.0 ? nextBeat : (prevBeat != -1.0 ? prevBeat : position);
    } else {
        qWarning() << "PROGRAMMING ERROR: Invalid quantize mode" << static_cast<int>(mode);
        return -1.0;
    }
}

bool CueControl::isTrackAtZeroPos() {
    return (fabs(getSampleOfTrack().current) < 1.0f);
}

bool CueControl::isTrackAtIntroCue() {
    return (fabs(getSampleOfTrack().current - m_pIntroStartPosition->get()) < 1.0f);
}

bool CueControl::isPlayingByPlayButton() {
    return m_pPlay->toBool() &&
            !m_iCurrentlyPreviewingHotcues && !m_bPreviewing;
}

bool CueControl::isCueRecallEnabled() {
    // Note that [Controls],CueRecall == 0 corresponds to "ON", not "OFF".
    return getConfig()->getValue(ConfigKey("[Controls]", "CueRecall"), 0) == 0;
}

SeekOnLoadMode CueControl::getSeekOnLoadMode() {
    return seekOnLoadModeFromDouble(m_pSeekOnLoadMode->get());
}


ConfigKey HotcueControl::keyForControl(int hotcue, const char* name) {
    ConfigKey key;
    key.group = m_group;
    // Add one to hotcue so that we don't have a hotcue_0
    key.item = QLatin1String("hotcue_") % QString::number(hotcue+1) % "_" % name;
    return key;
}

HotcueControl::HotcueControl(QString group, int i)
        : m_group(group),
          m_iHotcueNumber(i),
          m_pCue(NULL),
          m_bPreviewing(false),
          m_previewingPosition(-1) {
    m_hotcuePosition = new ControlObject(keyForControl(i, "position"));
    connect(m_hotcuePosition, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcuePositionChanged,
            Qt::DirectConnection);
    m_hotcuePosition->set(-1);

    m_hotcueEnabled = new ControlObject(keyForControl(i, "enabled"));
    m_hotcueEnabled->setReadOnly();

    // The id of the predefined color assigned to this color.
    m_hotcueColor = new ControlObject(keyForControl(i, "color_id"));
    connect(m_hotcueColor,
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueColorChanged,
            Qt::DirectConnection);

    m_hotcueSet = new ControlPushButton(keyForControl(i, "set"));
    connect(m_hotcueSet, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcueSet,
            Qt::DirectConnection);

    m_hotcueGoto = new ControlPushButton(keyForControl(i, "goto"));
    connect(m_hotcueGoto, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcueGoto,
            Qt::DirectConnection);

    m_hotcueGotoAndPlay = new ControlPushButton(keyForControl(i, "gotoandplay"));
    connect(m_hotcueGotoAndPlay, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcueGotoAndPlay,
            Qt::DirectConnection);

    m_hotcueGotoAndStop = new ControlPushButton(keyForControl(i, "gotoandstop"));
    connect(m_hotcueGotoAndStop, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcueGotoAndStop,
            Qt::DirectConnection);

    m_hotcueActivate = new ControlPushButton(keyForControl(i, "activate"));
    connect(m_hotcueActivate, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcueActivate,
            Qt::DirectConnection);

    m_hotcueActivatePreview = new ControlPushButton(keyForControl(i, "activate_preview"));
    connect(m_hotcueActivatePreview, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcueActivatePreview,
            Qt::DirectConnection);

    m_hotcueClear = new ControlPushButton(keyForControl(i, "clear"));
    connect(m_hotcueClear, &ControlObject::valueChanged,
            this, &HotcueControl::slotHotcueClear,
            Qt::DirectConnection);
}

HotcueControl::~HotcueControl() {
    delete m_hotcuePosition;
    delete m_hotcueEnabled;
    delete m_hotcueColor;
    delete m_hotcueSet;
    delete m_hotcueGoto;
    delete m_hotcueGotoAndPlay;
    delete m_hotcueGotoAndStop;
    delete m_hotcueActivate;
    delete m_hotcueActivatePreview;
    delete m_hotcueClear;
}

void HotcueControl::slotHotcueSet(double v) {
    emit(hotcueSet(this, v));
}

void HotcueControl::slotHotcueGoto(double v) {
    emit(hotcueGoto(this, v));
}

void HotcueControl::slotHotcueGotoAndPlay(double v) {
    emit(hotcueGotoAndPlay(this, v));
}

void HotcueControl::slotHotcueGotoAndStop(double v) {
    emit(hotcueGotoAndStop(this, v));
}

void HotcueControl::slotHotcueActivate(double v) {
    emit(hotcueActivate(this, v));
}

void HotcueControl::slotHotcueActivatePreview(double v) {
    emit(hotcueActivatePreview(this, v));
}

void HotcueControl::slotHotcueClear(double v) {
    emit(hotcueClear(this, v));
}

void HotcueControl::slotHotcuePositionChanged(double newPosition) {
    m_hotcueEnabled->forceSet(newPosition == -1 ? 0.0 : 1.0);
    emit(hotcuePositionChanged(this, newPosition));
}

void HotcueControl::slotHotcueColorChanged(double newColorId) {
    m_pCue->setColor(Color::kPredefinedColorsSet.predefinedColorFromId(newColorId));
    emit(hotcueColorChanged(this, newColorId));
}

double HotcueControl::getPosition() const {
    return m_hotcuePosition->get();
}

void HotcueControl::setCue(CuePointer pCue) {
    setPosition(pCue->getPosition());
    setColor(pCue->getColor());
    // set pCue only if all other data is in place
    // because we have a null check for valid data else where in the code
    m_pCue = pCue;
}
PredefinedColorPointer HotcueControl::getColor() const {
    return Color::kPredefinedColorsSet.predefinedColorFromId(m_hotcueColor->get());
}

void HotcueControl::setColor(PredefinedColorPointer newColor) {
    m_hotcueColor->set(static_cast<double>(newColor->m_iId));
}
void HotcueControl::resetCue() {
    // clear pCue first because we have a null check for valid data else where
    // in the code
    m_pCue.reset();
    setPosition(-1.0);
}

void HotcueControl::setPosition(double position) {
    m_hotcuePosition->set(position);
    m_hotcueEnabled->forceSet(position == -1.0 ? 0.0 : 1.0);
}
