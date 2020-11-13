// cuecontrol.cpp
// Created 11/5/2009 by RJ Ryan (rryan@mit.edu)

#include "engine/controls/cuecontrol.h"

#include <QMutexLocker>

#include "control/controlindicator.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "engine/enginebuffer.h"
#include "preferences/colorpalettesettings.h"
#include "track/track.h"
#include "util/color/color.h"
#include "util/color/predefinedcolorpalettes.h"
#include "util/sample.h"
#include "vinylcontrol/defs_vinylcontrol.h"

namespace {

// TODO: Convert these doubles to a standard enum
// and convert elseif logic to switch statements
constexpr double CUE_MODE_MIXXX = 0.0;
constexpr double CUE_MODE_PIONEER = 1.0;
constexpr double CUE_MODE_DENON = 2.0;
constexpr double CUE_MODE_NUMARK = 3.0;
constexpr double CUE_MODE_MIXXX_NO_BLINK = 4.0;
constexpr double CUE_MODE_CUP = 5.0;

/// This is the position of a fresh loaded tack without any seek
constexpr double kDefaultLoadPosition = 0.0;

// Helper function to convert control values (i.e. doubles) into RgbColor
// instances (or nullopt if value < 0). This happens by using the integer
// component as RGB color codes (e.g. 0xFF0000).
inline mixxx::RgbColor::optional_t doubleToRgbColor(double value) {
    if (value < 0) {
        return std::nullopt;
    }
    auto colorCode = static_cast<mixxx::RgbColor::code_t>(value);
    if (value != mixxx::RgbColor::validateCode(colorCode)) {
        return std::nullopt;
    }
    return mixxx::RgbColor::optional(colorCode);
}

} // namespace

CueControl::CueControl(QString group,
        UserSettingsPointer pConfig)
        : EngineControl(group, pConfig),
          m_pConfig(pConfig),
          m_colorPaletteSettings(ColorPaletteSettings(pConfig)),
          m_bPreviewing(false),
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

    m_pClosestBeat = ControlObject::getControl(ConfigKey(group, "beat_closest"));

    m_pCuePoint = new ControlObject(ConfigKey(group, "cue_point"));
    m_pCuePoint->set(Cue::kNoPosition);

    m_pCueMode = new ControlObject(ConfigKey(group, "cue_mode"));

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
    m_pIntroStartPosition->set(Cue::kNoPosition);

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
    m_pIntroEndPosition->set(Cue::kNoPosition);

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
    m_pOutroStartPosition->set(Cue::kNoPosition);

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
    m_pOutroEndPosition->set(Cue::kNoPosition);

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

    m_pHotcueFocus = new ControlObject(ConfigKey(group, "hotcue_focus"));
    m_pHotcueFocus->set(Cue::kNoHotCue);

    m_pHotcueFocusColorPrev = new ControlObject(ConfigKey(group, "hotcue_focus_color_prev"));
    connect(m_pHotcueFocusColorPrev,
            &ControlObject::valueChanged,
            this,
            &CueControl::hotcueFocusColorPrev,
            Qt::DirectConnection);

    m_pHotcueFocusColorNext = new ControlObject(ConfigKey(group, "hotcue_focus_color_next"));
    connect(m_pHotcueFocusColorNext,
            &ControlObject::valueChanged,
            this,
            &CueControl::hotcueFocusColorNext,
            Qt::DirectConnection);
}

CueControl::~CueControl() {
    delete m_pCuePoint;
    delete m_pCueMode;
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
    delete m_pHotcueFocus;
    delete m_pHotcueFocusColorPrev;
    delete m_pHotcueFocusColorNext;
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

void CueControl::attachCue(CuePointer pCue, HotcueControl* pControl) {
    VERIFY_OR_DEBUG_ASSERT(pControl) {
        return;
    }
    detachCue(pControl);
    connect(pCue.get(), &Cue::updated,
            this, &CueControl::cueUpdated,
            Qt::DirectConnection);

    pControl->setCue(pCue);
}

void CueControl::detachCue(HotcueControl* pControl) {
    VERIFY_OR_DEBUG_ASSERT(pControl) {
        return;
    }
    CuePointer pCue(pControl->getCue());
    if (!pCue) {
        return;
    }
    disconnect(pCue.get(), 0, this, 0);
    pControl->resetCue();
}

void CueControl::trackLoaded(TrackPointer pNewTrack) {
    QMutexLocker lock(&m_mutex);
    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(), 0, this, 0);
        for (const auto& pControl : qAsConst(m_hotcueControls)) {
            detachCue(pControl);
        }

        m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
        m_pCuePoint->set(Cue::kNoPosition);
        m_pIntroStartPosition->set(Cue::kNoPosition);
        m_pIntroStartEnabled->forceSet(0.0);
        m_pIntroEndPosition->set(Cue::kNoPosition);
        m_pIntroEndEnabled->forceSet(0.0);
        m_pOutroStartPosition->set(Cue::kNoPosition);
        m_pOutroStartEnabled->forceSet(0.0);
        m_pOutroEndPosition->set(Cue::kNoPosition);
        m_pOutroEndEnabled->forceSet(0.0);
        m_pHotcueFocus->set(Cue::kNoHotCue);
        m_pLoadedTrack.reset();
        m_usedSeekOnLoadPosition.setValue(kDefaultLoadPosition);
    }

    if (!pNewTrack) {
        return;
    }
    m_pLoadedTrack = pNewTrack;

    connect(m_pLoadedTrack.get(), &Track::analyzed, this, &CueControl::trackAnalyzed, Qt::DirectConnection);

    connect(m_pLoadedTrack.get(), &Track::cuesUpdated,
            this, &CueControl::trackCuesUpdated,
            Qt::DirectConnection);

    CuePointer pMainCue;
    const QList<CuePointer> cuePoints = m_pLoadedTrack->getCuePoints();
    for (const CuePointer& pCue : cuePoints) {
        if (pCue->getType() == mixxx::CueType::MainCue) {
            DEBUG_ASSERT(!pMainCue);
            pMainCue = pCue;
        }
    }

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();
    // Use pNewTrack from now, because m_pLoadedTrack might have been reset
    // immediately after leaving the locking scope!

    // Because of legacy, we store the (load) cue point twice and need to
    // sync both values.
    // The mixxx::CueType::MainCue from getCuePoints() has the priority
    CuePosition mainCuePoint;
    if (pMainCue) {
        mainCuePoint.setPosition(pMainCue->getPosition());
        // adjust the track cue accordingly
        pNewTrack->setCuePoint(mainCuePoint);
    } else {
        // If no load cue point is stored, read from track
        // Note: This is 0:00 for new tracks
        mainCuePoint = pNewTrack->getCuePoint();
        // Than add the load cue to the list of cue
        CuePointer pCue(pNewTrack->createAndAddCue());
        pCue->setStartPosition(mainCuePoint.getPosition());
        pCue->setHotCue(Cue::kNoHotCue);
        pCue->setType(mixxx::CueType::MainCue);
    }
    m_pCuePoint->set(mainCuePoint.getPosition());

    // Update COs with cues from track.
    loadCuesFromTrack();

    // Seek track according to SeekOnLoadMode.
    SeekOnLoadMode seekOnLoadMode = getSeekOnLoadPreference();

    switch (seekOnLoadMode) {
    case SeekOnLoadMode::Beginning:
        // This allows users to load tracks and have the needle-drop be maintained.
        if (!(m_pVinylControlEnabled->toBool() &&
                    m_pVinylControlMode->get() == MIXXX_VCMODE_ABSOLUTE)) {
            seekOnLoad(0.0);
        }
        break;
    case SeekOnLoadMode::FirstSound: {
        CuePointer pAudibleSound = pNewTrack->findCueByType(mixxx::CueType::AudibleSound);
        double audibleSoundPosition = Cue::kNoPosition;
        if (pAudibleSound) {
            audibleSoundPosition = pAudibleSound->getPosition();
        }
        if (audibleSoundPosition != Cue::kNoPosition) {
            seekOnLoad(audibleSoundPosition);
        } else {
            seekOnLoad(0.0);
        }
        break;
    }
    case SeekOnLoadMode::MainCue: {
        // Take main cue position from CO instead of cue point list because
        // value in CO will be quantized if quantization is enabled
        // while value in cue point list will never be quantized.
        // This prevents jumps when track analysis finishes while quantization is enabled.
        double cuePoint = m_pCuePoint->get();
        if (cuePoint != Cue::kNoPosition) {
            seekOnLoad(cuePoint);
        } else {
            seekOnLoad(0.0);
        }
        break;
    }
    case SeekOnLoadMode::IntroStart: {
        double introStart = m_pIntroStartPosition->get();
        if (introStart != Cue::kNoPosition) {
            seekOnLoad(introStart);
        } else {
            seekOnLoad(0.0);
        }
        break;
    }
    default:
        DEBUG_ASSERT(!"Unknown enum value");
        seekOnLoad(0.0);
        break;
    }
}

void CueControl::seekOnLoad(double seekOnLoadPosition) {
    seekExact(seekOnLoadPosition);
    m_usedSeekOnLoadPosition.setValue(seekOnLoadPosition);
}

void CueControl::cueUpdated() {
    //QMutexLocker lock(&m_mutex);
    // We should get a trackCuesUpdated call anyway, so do nothing.
}

void CueControl::loadCuesFromTrack() {
    QMutexLocker lock(&m_mutex);
    QSet<int> active_hotcues;
    CuePointer pLoadCue, pIntroCue, pOutroCue;

    if (!m_pLoadedTrack) {
        return;
    }

    const QList<CuePointer> cues = m_pLoadedTrack->getCuePoints();
    for (const auto& pCue : cues) {
        switch (pCue->getType()) {
        case mixxx::CueType::MainCue:
            DEBUG_ASSERT(!pLoadCue); // There should be only one MainCue cue
            pLoadCue = pCue;
            break;
        case mixxx::CueType::Intro:
            DEBUG_ASSERT(!pIntroCue); // There should be only one Intro cue
            pIntroCue = pCue;
            break;
        case mixxx::CueType::Outro:
            DEBUG_ASSERT(!pOutroCue); // There should be only one Outro cue
            pOutroCue = pCue;
            break;
        case mixxx::CueType::HotCue:
        case mixxx::CueType::Loop: {
            // FIXME: While it's not possible to save Loops in Mixxx yet, we do
            // support importing them from Serato and Rekordbox. For the time
            // being we treat them like regular hotcues and ignore their end
            // position until #2194 has been merged.
            if (pCue->getHotCue() == Cue::kNoHotCue) {
                continue;
            }

            int hotcue = pCue->getHotCue();
            HotcueControl* pControl = m_hotcueControls.value(hotcue, NULL);

            // Cue's hotcue doesn't have a hotcue control.
            if (pControl == nullptr) {
                continue;
            }

            CuePointer pOldCue(pControl->getCue());

            // If the old hotcue is different than this one.
            if (pOldCue != pCue) {
                // old cue is detached if required
                attachCue(pCue, pControl);
            } else {
                // If the old hotcue is the same, then we only need to update
                pControl->setPosition(pCue->getPosition());
                pControl->setColor(pCue->getColor());
            }
            // Add the hotcue to the list of active hotcues
            active_hotcues.insert(hotcue);
            break;
        }
        default:
            break;
        }
    }

    if (pIntroCue) {
        double startPosition = pIntroCue->getPosition();
        double endPosition = pIntroCue->getEndPosition();

        m_pIntroStartPosition->set(quantizeCuePoint(startPosition));
        m_pIntroStartEnabled->forceSet(startPosition == Cue::kNoPosition ? 0.0 : 1.0);
        m_pIntroEndPosition->set(quantizeCuePoint(endPosition));
        m_pIntroEndEnabled->forceSet(endPosition == Cue::kNoPosition ? 0.0 : 1.0);
    } else {
        m_pIntroStartPosition->set(Cue::kNoPosition);
        m_pIntroStartEnabled->forceSet(0.0);
        m_pIntroEndPosition->set(Cue::kNoPosition);
        m_pIntroEndEnabled->forceSet(0.0);
    }

    if (pOutroCue) {
        double startPosition = pOutroCue->getPosition();
        double endPosition = pOutroCue->getEndPosition();

        m_pOutroStartPosition->set(quantizeCuePoint(startPosition));
        m_pOutroStartEnabled->forceSet(startPosition == Cue::kNoPosition ? 0.0 : 1.0);
        m_pOutroEndPosition->set(quantizeCuePoint(endPosition));
        m_pOutroEndEnabled->forceSet(endPosition == Cue::kNoPosition ? 0.0 : 1.0);
    } else {
        m_pOutroStartPosition->set(Cue::kNoPosition);
        m_pOutroStartEnabled->forceSet(0.0);
        m_pOutroEndPosition->set(Cue::kNoPosition);
        m_pOutroEndEnabled->forceSet(0.0);
    }

    if (pLoadCue) {
        double position = pLoadCue->getPosition();
        m_pCuePoint->set(quantizeCuePoint(position));
    } else {
        m_pCuePoint->set(Cue::kNoPosition);
    }

    // Detach all hotcues that are no longer present
    for (int hotCue = 0; hotCue < m_iNumHotCues; ++hotCue) {
        if (!active_hotcues.contains(hotCue)) {
            HotcueControl* pControl = m_hotcueControls.at(hotCue);
            detachCue(pControl);
        }
    }
}

void CueControl::trackAnalyzed() {
    if (!m_pLoadedTrack) {
        return;
    }

    SampleOfTrack sampleOfTrack = getSampleOfTrack();
    if (sampleOfTrack.current != m_usedSeekOnLoadPosition.getValue()) {
        // the track is already manual cued, don't re-cue
        return;
    }

    // Make track follow the updated cues.
    SeekOnLoadMode seekOnLoadMode = getSeekOnLoadPreference();

    if (seekOnLoadMode == SeekOnLoadMode::MainCue) {
        double cue = m_pCuePoint->get();
        if (cue != Cue::kNoPosition) {
            seekOnLoad(cue);
        }
    } else if (seekOnLoadMode == SeekOnLoadMode::IntroStart) {
        double intro = m_pIntroStartPosition->get();
        if (intro != Cue::kNoPosition) {
            seekOnLoad(intro);
        }
    }
}

void CueControl::trackCuesUpdated() {
    loadCuesFromTrack();
}

void CueControl::trackBeatsUpdated(mixxx::BeatsPointer pBeats) {
    Q_UNUSED(pBeats);
    loadCuesFromTrack();
}

void CueControl::quantizeChanged(double v) {
    Q_UNUSED(v);

    // check if we were at the cue point before
    bool wasTrackAtCue = getTrackAt() == TrackAt::Cue;
    bool wasTrackAtIntro = isTrackAtIntroCue();

    loadCuesFromTrack();

    // if we are playing (no matter what reason for) do not seek
    if (m_pPlay->toBool()) {
        return;
    }

    // Retrieve new cue pos and follow
    double cue = m_pCuePoint->get();
    if (wasTrackAtCue && cue != Cue::kNoPosition) {
        seekExact(cue);
    }
    // Retrieve new intro start pos and follow
    double intro = m_pIntroStartPosition->get();
    if (wasTrackAtIntro && intro != Cue::kNoPosition) {
        seekExact(intro);
    }
}

void CueControl::hotcueSet(HotcueControl* pControl, double value) {
    //qDebug() << "CueControl::hotcueSet" << value;

    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    int hotcue = pControl->getHotcueNumber();
    // Note: the cue is just detached from the hotcue control
    // It remains in the database for later use
    // TODO: find a rule, that allows us to delete the cue as well
    // https://bugs.launchpad.net/mixxx/+bug/1653276
    hotcueClear(pControl, value);

    CuePointer pCue(m_pLoadedTrack->createAndAddCue());
    double cuePosition = getQuantizedCurrentPosition();
    pCue->setStartPosition(cuePosition);
    pCue->setHotCue(hotcue);
    pCue->setLabel();
    pCue->setType(mixxx::CueType::HotCue);

    const ColorPalette hotcueColorPalette =
            m_colorPaletteSettings.getHotcueColorPalette();
    if (getConfig()->getValue(ConfigKey("[Controls]", "auto_hotcue_colors"), false)) {
        pCue->setColor(hotcueColorPalette.colorForHotcueIndex(hotcue));
    } else {
        int hotcueDefaultColorIndex = m_pConfig->getValue(ConfigKey("[Controls]", "HotcueDefaultColorIndex"), -1);
        if (hotcueDefaultColorIndex < 0 || hotcueDefaultColorIndex >= hotcueColorPalette.size()) {
            hotcueDefaultColorIndex = hotcueColorPalette.size() - 1; // default to last color (orange)
        }
        pCue->setColor(hotcueColorPalette.at(hotcueDefaultColorIndex));
    }

    // TODO(XXX) deal with spurious signals
    attachCue(pCue, pControl);

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

void CueControl::hotcueGoto(HotcueControl* pControl, double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue(pControl->getCue());

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (pCue) {
        double position = pCue->getPosition();
        if (position != Cue::kNoPosition) {
            seekAbs(position);
        }
    }
}

void CueControl::hotcueGotoAndStop(HotcueControl* pControl, double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    CuePointer pCue(pControl->getCue());

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (pCue) {
        double position = pCue->getPosition();
        if (position != Cue::kNoPosition) {
            m_pPlay->set(0.0);
            seekExact(position);
        }
    }
}

void CueControl::hotcueGotoAndPlay(HotcueControl* pControl, double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue(pControl->getCue());

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (pCue) {
        double position = pCue->getPosition();
        if (position != Cue::kNoPosition) {
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

void CueControl::hotcueActivate(HotcueControl* pControl, double value) {
    //qDebug() << "CueControl::hotcueActivate" << value;

    QMutexLocker lock(&m_mutex);

    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue(pControl->getCue());

    lock.unlock();

    if (pCue) {
        if (value != 0) {
            if (pCue->getPosition() == Cue::kNoPosition) {
                hotcueSet(pControl, value);
            } else {
                if (isPlayingByPlayButton()) {
                    hotcueGoto(pControl, value);
                } else {
                    hotcueActivatePreview(pControl, value);
                }
            }
        } else {
            if (pCue->getPosition() != Cue::kNoPosition) {
                hotcueActivatePreview(pControl, value);
            }
        }
    } else {
        // The cue is non-existent ...
        if (value != 0) {
            // set it to the current position
            hotcueSet(pControl, value);
        } else if (m_iCurrentlyPreviewingHotcues) {
            // yet we got a release for it and are
            // currently previewing a hotcue. This is indicative of a corner
            // case where the cue was detached while we were pressing it. Let
            // hotcueActivatePreview handle it.
            hotcueActivatePreview(pControl, value);
        }
    }

    m_pHotcueFocus->set(pControl->getHotcueNumber());
}

void CueControl::hotcueActivatePreview(HotcueControl* pControl, double value) {
    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack) {
        return;
    }
    CuePointer pCue(pControl->getCue());

    if (value != 0) {
        if (pCue && pCue->getPosition() != Cue::kNoPosition) {
            m_iCurrentlyPreviewingHotcues++;
            double position = pCue->getPosition();
            m_bypassCueSetByPlay = true;
            pControl->setPreviewing(true);
            pControl->setPreviewingPosition(position);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(position);
            m_pPlay->set(1.0);
        }
    } else if (m_iCurrentlyPreviewingHotcues) {
        // This is a activate release and we are previewing at least one
        // hotcue. If this hotcue is previewing:
        if (pControl->isPreviewing()) {
            // Mark this hotcue as not previewing.
            double position = pControl->getPreviewingPosition();
            pControl->setPreviewing(false);
            pControl->setPreviewingPosition(Cue::kNoPosition);

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

void CueControl::hotcueClear(HotcueControl* pControl, double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue(pControl->getCue());
    if (!pCue) {
        return;
    }
    detachCue(pControl);
    m_pLoadedTrack->removeCue(pCue);
    m_pHotcueFocus->set(Cue::kNoHotCue);
}

void CueControl::hotcuePositionChanged(HotcueControl* pControl, double newPosition) {
    QMutexLocker lock(&m_mutex);
    if (!m_pLoadedTrack)
        return;

    CuePointer pCue(pControl->getCue());
    if (pCue) {
        // Setting the position to Cue::kNoPosition is the same as calling hotcue_x_clear
        if (newPosition == Cue::kNoPosition) {
            detachCue(pControl);
        } else if (newPosition > 0 && newPosition < m_pTrackSamples->get()) {
            pCue->setStartPosition(newPosition);
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
    for (const auto& pControl : qAsConst(m_hotcueControls)) {
        double position = pControl->getPosition();
        if (position != Cue::kNoPosition) {
            cue_hint.frame = SampleUtil::floorPlayPosToFrame(position);
            cue_hint.frameCount = Hint::kFrameCountForward;
            cue_hint.priority = 10;
            pHintList->append(cue_hint);
        }
    }
}

// Moves the cue point to current position or to closest beat in case
// quantize is enabled
void CueControl::cueSet(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);

    double cue = getQuantizedCurrentPosition();
    m_pCuePoint->set(cue);
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Store cue point in loaded track
    if (pLoadedTrack) {
        pLoadedTrack->setCuePoint(CuePosition(cue));
    }
}

void CueControl::cueClear(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    m_pCuePoint->set(Cue::kNoPosition);
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        pLoadedTrack->setCuePoint(CuePosition());
    }
}

void CueControl::cueGoto(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    // Seek to cue point
    double cuePoint = m_pCuePoint->get();

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    seekAbs(cuePoint);
}

void CueControl::cueGotoAndPlay(double value) {
    if (value == 0) {
        return;
    }

    cueGoto(value);
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

void CueControl::cueGotoAndStop(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    m_pPlay->set(0.0);
    double cuePoint = m_pCuePoint->get();

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    seekExact(cuePoint);
}

void CueControl::cuePreview(double value) {
    QMutexLocker lock(&m_mutex);

    if (value != 0) {
        m_bPreviewing = true;
        m_bypassCueSetByPlay = true;
        m_pPlay->set(1.0);
    } else if (m_bPreviewing) {
        m_bPreviewing = false;
        if (m_iCurrentlyPreviewingHotcues) {
            return;
        }
        m_pPlay->set(0.0);
    } else {
        return;
    }

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    seekAbs(m_pCuePoint->get());
}

void CueControl::cueCDJ(double value) {
    // This is how Pioneer cue buttons work:
    // If pressed while freely playing (i.e. playing and platter NOT being touched), stop playback and go to cue.
    // If pressed while NOT freely playing (i.e. stopped or playing but platter IS being touched), set new cue point.
    // If pressed while stopped and at cue, play while pressed.
    // If play is pressed while holding cue, the deck is now playing. (Handled in playFromCuePreview().)

    QMutexLocker lock(&m_mutex);
    const auto freely_playing = m_pPlay->toBool() && !getEngineBuffer()->getScratching();
    TrackAt trackAt = getTrackAt();

    if (value != 0) {
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
            cueSet(value);
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

void CueControl::cueDenon(double value) {
    // This is how Denon DN-S 3700 cue buttons work:
    // If pressed go to cue and stop.
    // If pressed while stopped and at cue, play while pressed.
    // Cue Point is moved by play from pause

    QMutexLocker lock(&m_mutex);
    bool playing = (m_pPlay->toBool());
    TrackAt trackAt = getTrackAt();

    if (value != 0) {
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

void CueControl::cuePlay(double value) {
    // This is how CUP button works:
    // If freely playing (i.e. playing and platter NOT being touched), press to go to cue and stop.
    // If not freely playing (i.e. stopped or platter IS being touched), press to go to cue and stop.
    // On release, start playing from cue point.


    QMutexLocker lock(&m_mutex);
    const auto freely_playing = m_pPlay->toBool() && !getEngineBuffer()->getScratching();
    TrackAt trackAt = getTrackAt();

    // pressed
    if (value != 0) {
        if (freely_playing) {
            m_bPreviewing = false;
            m_pPlay->set(0.0);

            // Need to unlock before emitting any signals to prevent deadlock.
            lock.unlock();

            seekAbs(m_pCuePoint->get());
        } else if (trackAt == TrackAt::ElseWhere) {
            // Pause not at cue point and not at end position
            cueSet(value);
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
    } else if (trackAt == TrackAt::Cue) {
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

void CueControl::introStartSet(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);

    double position = getQuantizedCurrentPosition();

    // Make sure user is not trying to place intro start cue on or after
    // other intro/outro cues.
    double introEnd = m_pIntroEndPosition->get();
    double outroStart = m_pOutroStartPosition->get();
    double outroEnd = m_pOutroEndPosition->get();
    if (introEnd != Cue::kNoPosition && position >= introEnd) {
        qWarning() << "Trying to place intro start cue on or after intro end cue.";
        return;
    }
    if (outroStart != Cue::kNoPosition && position >= outroStart) {
        qWarning() << "Trying to place intro start cue on or after outro start cue.";
        return;
    }
    if (outroEnd != Cue::kNoPosition && position >= outroEnd) {
        qWarning() << "Trying to place intro start cue on or after outro end cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Intro);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue();
            pCue->setType(mixxx::CueType::Intro);
        }
        pCue->setStartPosition(position);
        pCue->setEndPosition(introEnd);
    }
}

void CueControl::introStartClear(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    double introEnd = m_pIntroEndPosition->get();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Intro);
        if (introEnd != Cue::kNoPosition) {
            pCue->setStartPosition(Cue::kNoPosition);
            pCue->setEndPosition(introEnd);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::introStartActivate(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    double introStart = m_pIntroStartPosition->get();
    lock.unlock();

    if (introStart == Cue::kNoPosition) {
        introStartSet(1.0);
    } else {
        seekAbs(introStart);
    }
}

void CueControl::introEndSet(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);

    double position = getQuantizedCurrentPosition();

    // Make sure user is not trying to place intro end cue on or before
    // intro start cue, or on or after outro start/end cue.
    double introStart = m_pIntroStartPosition->get();
    double outroStart = m_pOutroStartPosition->get();
    double outroEnd = m_pOutroEndPosition->get();
    if (introStart != Cue::kNoPosition && position <= introStart) {
        qWarning() << "Trying to place intro end cue on or before intro start cue.";
        return;
    }
    if (outroStart != Cue::kNoPosition && position >= outroStart) {
        qWarning() << "Trying to place intro end cue on or after outro start cue.";
        return;
    }
    if (outroEnd != Cue::kNoPosition && position >= outroEnd) {
        qWarning() << "Trying to place intro end cue on or after outro end cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Intro);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue();
            pCue->setType(mixxx::CueType::Intro);
        }
        pCue->setStartPosition(introStart);
        pCue->setEndPosition(position);
    }
}

void CueControl::introEndClear(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    double introStart = m_pIntroStartPosition->get();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Intro);
        if (introStart != Cue::kNoPosition) {
            pCue->setStartPosition(introStart);
            pCue->setEndPosition(Cue::kNoPosition);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::introEndActivate(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    double introEnd = m_pIntroEndPosition->get();
    lock.unlock();

    if (introEnd == Cue::kNoPosition) {
        introEndSet(1.0);
    } else {
        seekAbs(introEnd);
    }
}

void CueControl::outroStartSet(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);

    double position = getQuantizedCurrentPosition();

    // Make sure user is not trying to place outro start cue on or before
    // intro end cue or on or after outro end cue.
    double introStart = m_pIntroStartPosition->get();
    double introEnd = m_pIntroEndPosition->get();
    double outroEnd = m_pOutroEndPosition->get();
    if (introStart != Cue::kNoPosition && position <= introStart) {
        qWarning() << "Trying to place outro start cue on or before intro start cue.";
        return;
    }
    if (introEnd != Cue::kNoPosition && position <= introEnd) {
        qWarning() << "Trying to place outro start cue on or before intro end cue.";
        return;
    }
    if (outroEnd != Cue::kNoPosition && position >= outroEnd) {
        qWarning() << "Trying to place outro start cue on or after outro end cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Outro);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue();
            pCue->setType(mixxx::CueType::Outro);
        }
        pCue->setStartPosition(position);
        pCue->setEndPosition(outroEnd);
    }
}

void CueControl::outroStartClear(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    double outroEnd = m_pOutroEndPosition->get();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Outro);
        if (outroEnd != Cue::kNoPosition) {
            pCue->setStartPosition(Cue::kNoPosition);
            pCue->setEndPosition(outroEnd);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::outroStartActivate(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    double outroStart = m_pOutroStartPosition->get();
    lock.unlock();

    if (outroStart == Cue::kNoPosition) {
        outroStartSet(1.0);
    } else {
        seekAbs(outroStart);
    }
}

void CueControl::outroEndSet(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);

    double position = getQuantizedCurrentPosition();

    // Make sure user is not trying to place outro end cue on or before
    // other intro/outro cues.
    double introStart = m_pIntroStartPosition->get();
    double introEnd = m_pIntroEndPosition->get();
    double outroStart = m_pOutroStartPosition->get();
    if (introStart != Cue::kNoPosition && position <= introStart) {
        qWarning() << "Trying to place outro end cue on or before intro start cue.";
        return;
    }
    if (introEnd != Cue::kNoPosition && position <= introEnd) {
        qWarning() << "Trying to place outro end cue on or before intro end cue.";
        return;
    }
    if (outroStart != Cue::kNoPosition && position <= outroStart) {
        qWarning() << "Trying to place outro end cue on or before outro start cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Outro);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue();
            pCue->setType(mixxx::CueType::Outro);
        }
        pCue->setStartPosition(outroStart);
        pCue->setEndPosition(position);
    }
}

void CueControl::outroEndClear(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    double outroStart = m_pOutroStartPosition->get();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Outro);
        if (outroStart != Cue::kNoPosition) {
            pCue->setStartPosition(outroStart);
            pCue->setEndPosition(Cue::kNoPosition);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::outroEndActivate(double value) {
    if (value == 0) {
        return;
    }

    QMutexLocker lock(&m_mutex);
    double outroEnd = m_pOutroEndPosition->get();
    lock.unlock();

    if (outroEnd == Cue::kNoPosition) {
        outroEndSet(1.0);
    } else {
        seekAbs(outroEnd);
    }
}

// This is also called from the engine thread. No locking allowed.
bool CueControl::updateIndicatorsAndModifyPlay(bool newPlay, bool oldPlay, bool playPossible) {
    //qDebug() << "updateIndicatorsAndModifyPlay" << newPlay << playPossible
    //        << m_iCurrentlyPreviewingHotcues << m_bPreviewing;
    CueMode cueMode = static_cast<CueMode>(static_cast<int>(m_pCueMode->get()));
    if ((cueMode == CueMode::Denon || cueMode == CueMode::Numark) &&
            newPlay && !oldPlay && playPossible &&
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
        if (!newPlay && oldPlay) {
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
        if (cueMode == CueMode::Denon) {
            if (trackAt == TrackAt::Cue || previewing) {
                m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
            } else {
                // Flashing indicates that a following play would move cue point
                m_pPlayIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
            }
        } else if (cueMode == CueMode::Mixxx || cueMode == CueMode::MixxxNoBlinking ||
                cueMode == CueMode::Numark) {
            m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
        } else {
            // Flashing indicates that play is possible in Pioneer mode
            m_pPlayIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
        }
    }

    if (cueMode != CueMode::Denon && cueMode != CueMode::Numark) {
        if (m_pCuePoint->get() != Cue::kNoPosition) {
            if (newPlay == 0.0 && trackAt == TrackAt::ElseWhere) {
                if (cueMode == CueMode::Mixxx) {
                    // in Mixxx mode Cue Button is flashing slow if CUE will move Cue point
                    m_pCueIndicator->setBlinkValue(ControlIndicator::RATIO1TO1_500MS);
                } else if (cueMode == CueMode::MixxxNoBlinking) {
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
    // Note: current can be in the padded silence after the track end > total.
    if (sot.current >= sot.total) {
        return TrackAt::End;
    }
    double cue = m_pCuePoint->get();
    if (cue != Cue::kNoPosition && fabs(sot.current - cue) < 1.0f) {
        return TrackAt::Cue;
    }
    return TrackAt::ElseWhere;
}

double CueControl::getQuantizedCurrentPosition() {
    SampleOfTrack sampleOfTrack = getSampleOfTrack();
    double currentPos = sampleOfTrack.current;
    const double total = sampleOfTrack.total;

    // Note: currentPos can be past the end of the track, in the padded
    // silence of the last buffer. This position might be not reachable in
    // a future runs, depending on the buffering.
    currentPos = math_min(currentPos, total);

    // Don't quantize if quantization is disabled.
    if (!m_pQuantizeEnabled->toBool()) {
        return currentPos;
    }

    double closestBeat = m_pClosestBeat->get();
    // Note: closestBeat can be an interpolated beat past the end of the track,
    // which cannot be reached.
    if (closestBeat != -1.0 && closestBeat <= total) {
        return closestBeat;
    }

    return currentPos;
}

double CueControl::quantizeCuePoint(double cuePos) {
    // we need to use m_pTrackSamples here because SampleOfTrack
    // is set later by the engine and not during EngineBuffer::slotTrackLoaded
    const double total = m_pTrackSamples->get();

    if (cuePos > total) {
        // This can happen if the track length has changed or the cue was set in the
        // the padded silence after the track.
        cuePos = total;
    }

    // Don't quantize unset cues, manual cues or when quantization is disabled.
    if (cuePos == Cue::kNoPosition || !m_pQuantizeEnabled->toBool()) {
        return cuePos;
    }

    mixxx::BeatsPointer pBeats = m_pLoadedTrack->getBeats();
    if (!pBeats) {
        return cuePos;
    }

    double closestBeat = pBeats->findClosestBeat(cuePos);
    // The closest beat can be an unreachable  interpolated beat past the end of
    // the track.
    if (closestBeat != -1.0 && closestBeat <= total) {
        return closestBeat;
    }

    return cuePos;
}

bool CueControl::isTrackAtIntroCue() {
    return (fabs(getSampleOfTrack().current - m_pIntroStartPosition->get()) < 1.0f);
}

bool CueControl::isPlayingByPlayButton() {
    return m_pPlay->toBool() &&
            !m_iCurrentlyPreviewingHotcues && !m_bPreviewing;
}

SeekOnLoadMode CueControl::getSeekOnLoadPreference() {
    int configValue = getConfig()->getValue(ConfigKey("[Controls]", "CueRecall"),
            static_cast<int>(SeekOnLoadMode::IntroStart));
    return static_cast<SeekOnLoadMode>(configValue);
}

void CueControl::hotcueFocusColorPrev(double value) {
    if (value == 0) {
        return;
    }

    int hotcueNumber = static_cast<int>(m_pHotcueFocus->get());
    if (hotcueNumber < 0 || hotcueNumber >= m_hotcueControls.size()) {
        return;
    }

    HotcueControl* pControl = m_hotcueControls.at(hotcueNumber);
    if (!pControl) {
        return;
    }

    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }

    mixxx::RgbColor::optional_t color = pCue->getColor();
    if (!color) {
        return;
    }

    ColorPalette colorPalette = m_colorPaletteSettings.getHotcueColorPalette();
    pCue->setColor(colorPalette.previousColor(*color));
}

void CueControl::hotcueFocusColorNext(double value) {
    if (value == 0) {
        return;
    }

    int hotcueNumber = static_cast<int>(m_pHotcueFocus->get());
    if (hotcueNumber < 0 || hotcueNumber >= m_hotcueControls.size()) {
        return;
    }

    HotcueControl* pControl = m_hotcueControls.at(hotcueNumber);
    if (!pControl) {
        return;
    }

    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }

    mixxx::RgbColor::optional_t color = pCue->getColor();
    if (!color) {
        return;
    }

    ColorPalette colorPalette = m_colorPaletteSettings.getHotcueColorPalette();
    pCue->setColor(colorPalette.nextColor(*color));
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
    m_hotcuePosition->set(Cue::kNoPosition);

    m_hotcueEnabled = new ControlObject(keyForControl(i, "enabled"));
    m_hotcueEnabled->setReadOnly();

    // The rgba value  of the color assigned to this color.
    m_hotcueColor = new ControlObject(keyForControl(i, "color"));
    m_hotcueColor->connectValueChangeRequest(
            this,
            &HotcueControl::slotHotcueColorChangeRequest,
            Qt::DirectConnection);
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
    emit hotcueSet(this, v);
}

void HotcueControl::slotHotcueGoto(double v) {
    emit hotcueGoto(this, v);
}

void HotcueControl::slotHotcueGotoAndPlay(double v) {
    emit hotcueGotoAndPlay(this, v);
}

void HotcueControl::slotHotcueGotoAndStop(double v) {
    emit hotcueGotoAndStop(this, v);
}

void HotcueControl::slotHotcueActivate(double v) {
    emit hotcueActivate(this, v);
}

void HotcueControl::slotHotcueActivatePreview(double v) {
    emit hotcueActivatePreview(this, v);
}

void HotcueControl::slotHotcueClear(double v) {
    emit hotcueClear(this, v);
}

void HotcueControl::slotHotcuePositionChanged(double newPosition) {
    m_hotcueEnabled->forceSet(newPosition == Cue::kNoPosition ? 0.0 : 1.0);
    emit hotcuePositionChanged(this, newPosition);
}

void HotcueControl::slotHotcueColorChangeRequest(double color) {
    if (color < 0 || color > 0xFFFFFF) {
        qWarning() << "slotHotcueColorChanged got invalid value:" << color;
        return;
    }
    m_hotcueColor->setAndConfirm(color);
}

void HotcueControl::slotHotcueColorChanged(double newColor) {
    if (!m_pCue) {
        return;
    }

    mixxx::RgbColor::optional_t color = doubleToRgbColor(newColor);
    VERIFY_OR_DEBUG_ASSERT(color) {
        return;
    }

    m_pCue->setColor(*color);
    emit hotcueColorChanged(this, newColor);
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
mixxx::RgbColor::optional_t HotcueControl::getColor() const {
    return doubleToRgbColor(m_hotcueColor->get());
}

void HotcueControl::setColor(mixxx::RgbColor::optional_t newColor) {
    if (newColor) {
        m_hotcueColor->set(*newColor);
    }
}
void HotcueControl::resetCue() {
    // clear pCue first because we have a null check for valid data else where
    // in the code
    m_pCue.reset();
    setPosition(Cue::kNoPosition);
}

void HotcueControl::setPosition(double position) {
    m_hotcuePosition->set(position);
    m_hotcueEnabled->forceSet(position == Cue::kNoPosition ? 0.0 : 1.0);
}
