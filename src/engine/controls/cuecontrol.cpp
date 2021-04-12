#include "engine/controls/cuecontrol.h"

#include <QMutexLocker>

#include "control/controlindicator.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "engine/enginebuffer.h"
#include "moc_cuecontrol.cpp"
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
constexpr int kNoHotCueNumber = 0;
/// Used for a common tracking of the previewing Hotcue in m_currentlyPreviewingIndex
constexpr int kMainCueIndex = NUM_HOT_CUES;

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

/// Convert hot cue index to 1-based number
///
/// Works independent of if the hot cue index is either 0-based
/// or 1..n-based.
inline int hotcueIndexToHotcueNumber(int hotcueIndex) {
    if (hotcueIndex >= mixxx::kFirstHotCueIndex) {
        DEBUG_ASSERT(hotcueIndex != Cue::kNoHotCue);
        return (hotcueIndex - mixxx::kFirstHotCueIndex) + 1; // to 1-based numbering
    } else {
        DEBUG_ASSERT(hotcueIndex == Cue::kNoHotCue);
        return kNoHotCueNumber;
    }
}

/// Convert 1-based hot cue number to hot cue index.
///
/// Works independent of if the hot cue index is either 0-based
/// or 1..n-based.
inline int hotcueNumberToHotcueIndex(int hotcueNumber) {
    if (hotcueNumber >= 1) {
        DEBUG_ASSERT(hotcueNumber != kNoHotCueNumber);
        return mixxx::kFirstHotCueIndex + (hotcueNumber - 1); // from 1-based numbering
    } else {
        DEBUG_ASSERT(hotcueNumber == kNoHotCueNumber);
        return Cue::kNoHotCue;
    }
}

} // namespace

CueControl::CueControl(const QString& group,
        UserSettingsPointer pConfig)
        : EngineControl(group, pConfig),
          m_pConfig(pConfig),
          m_colorPaletteSettings(ColorPaletteSettings(pConfig)),
          m_currentlyPreviewingIndex(Cue::kNoHotCue),
          m_pPlay(ControlObject::getControl(ConfigKey(group, "play"))),
          m_pStopButton(ControlObject::getControl(ConfigKey(group, "stop"))),
          m_bypassCueSetByPlay(false),
          m_iNumHotCues(NUM_HOT_CUES),
          m_pCurrentSavedLoopControl(nullptr),
          m_trackMutex(QMutex::Recursive) {
    // To silence a compiler warning about CUE_MODE_PIONEER.
    Q_UNUSED(CUE_MODE_PIONEER);
    createControls();

    m_pTrackSamples = ControlObject::getControl(ConfigKey(group, "track_samples"));

    m_pQuantizeEnabled = ControlObject::getControl(ConfigKey(group, "quantize"));
    connect(m_pQuantizeEnabled, &ControlObject::valueChanged,
            this, &CueControl::quantizeChanged,
            Qt::DirectConnection);

    m_pClosestBeat = ControlObject::getControl(ConfigKey(group, "beat_closest"));
    m_pLoopStartPosition = make_parented<ControlProxy>(group, "loop_start_position", this);
    m_pLoopEndPosition = make_parented<ControlProxy>(group, "loop_end_position", this);
    m_pLoopEnabled = make_parented<ControlProxy>(group, "loop_enabled", this);
    m_pBeatLoopActivate = make_parented<ControlProxy>(group, "beatloop_activate", this);
    m_pBeatLoopSize = make_parented<ControlProxy>(group, "beatloop_size", this);

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

    m_pPlayLatched = new ControlObject(ConfigKey(group, "play_latched"));
    m_pPlayLatched->setReadOnly();

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
    setHotcueFocusIndex(Cue::kNoHotCue);

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
    delete m_pPlayLatched;
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
        connect(pControl,
                &HotcueControl::hotcueEndPositionChanged,
                this,
                &CueControl::hotcueEndPositionChanged,
                Qt::DirectConnection);
        connect(pControl,
                &HotcueControl::hotcueSet,
                this,
                &CueControl::hotcueSet,
                Qt::DirectConnection);
        connect(pControl,
                &HotcueControl::hotcueGoto,
                this,
                &CueControl::hotcueGoto,
                Qt::DirectConnection);
        connect(pControl,
                &HotcueControl::hotcueGotoAndPlay,
                this,
                &CueControl::hotcueGotoAndPlay,
                Qt::DirectConnection);
        connect(pControl,
                &HotcueControl::hotcueGotoAndStop,
                this,
                &CueControl::hotcueGotoAndStop,
                Qt::DirectConnection);
        connect(pControl,
                &HotcueControl::hotcueGotoAndLoop,
                this,
                &CueControl::hotcueGotoAndLoop,
                Qt::DirectConnection);
        connect(pControl,
                &HotcueControl::hotcueCueLoop,
                this,
                &CueControl::hotcueCueLoop,
                Qt::DirectConnection);
        connect(pControl,
                &HotcueControl::hotcueActivate,
                this,
                &CueControl::hotcueActivate,
                Qt::DirectConnection);
        connect(pControl,
                &HotcueControl::hotcueActivatePreview,
                this,
                &CueControl::hotcueActivatePreview,
                Qt::DirectConnection);
        connect(pControl,
                &HotcueControl::hotcueClear,
                this,
                &CueControl::hotcueClear,
                Qt::DirectConnection);

        m_hotcueControls.append(pControl);
    }
}

void CueControl::attachCue(const CuePointer& pCue, HotcueControl* pControl) {
    VERIFY_OR_DEBUG_ASSERT(pControl) {
        return;
    }
    detachCue(pControl);
    connect(pCue.get(),
            &Cue::updated,
            this,
            &CueControl::cueUpdated,
            Qt::DirectConnection);

    pControl->setCue(pCue);
}

void CueControl::detachCue(HotcueControl* pControl) {
    VERIFY_OR_DEBUG_ASSERT(pControl) {
        return;
    }

    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }

    disconnect(pCue.get(), nullptr, this, nullptr);
    m_pCurrentSavedLoopControl.testAndSetRelease(pControl, nullptr);
    pControl->resetCue();
}

// This is called from the EngineWokerThread and ends with the initial seek
// via seekOnLoad(). There is the theoretical and pending issue of a delayed control
// command intended for the old track that might be performed instead.
void CueControl::trackLoaded(TrackPointer pNewTrack) {
    QMutexLocker lock(&m_trackMutex);
    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(), nullptr, this, nullptr);

        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);

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
        setHotcueFocusIndex(Cue::kNoHotCue);
        m_pLoadedTrack.reset();
        m_usedSeekOnLoadPosition.setValue(kDefaultLoadPosition);
    }

    if (!pNewTrack) {
        return;
    }
    m_pLoadedTrack = pNewTrack;

    connect(m_pLoadedTrack.get(),
            &Track::analyzed,
            this,
            &CueControl::trackAnalyzed,
            Qt::DirectConnection);

    connect(m_pLoadedTrack.get(),
            &Track::cuesUpdated,
            this,
            &CueControl::trackCuesUpdated,
            Qt::DirectConnection);
    lock.unlock();

    // Use pNewTrack from now, because m_pLoadedTrack might have been reset
    // immediately after leaving the locking scope!

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
        CuePointer pAudibleSound =
                pNewTrack->findCueByType(mixxx::CueType::AudibleSound);
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
    QMutexLocker lock(&m_trackMutex);
    if (!m_pLoadedTrack) {
        return;
    }

    QSet<int> active_hotcues;
    CuePointer pMainCue;
    CuePointer pIntroCue;
    CuePointer pOutroCue;

    const QList<CuePointer> cues = m_pLoadedTrack->getCuePoints();
    for (const auto& pCue : cues) {
        switch (pCue->getType()) {
        case mixxx::CueType::MainCue:
            DEBUG_ASSERT(!pMainCue); // There should be only one MainCue cue
            pMainCue = pCue;
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
                Cue::StartAndEndPositions pos = pCue->getStartAndEndPosition();
                pControl->setPosition(pos.startPosition);
                pControl->setEndPosition(pos.endPosition);
                pControl->setColor(pCue->getColor());
                pControl->setType(pCue->getType());
            }
            // Add the hotcue to the list of active hotcues
            active_hotcues.insert(hotcue);
            break;
        }
        default:
            break;
        }
    }

    // Detach all hotcues that are no longer present
    for (int hotCueIndex = 0; hotCueIndex < m_iNumHotCues; ++hotCueIndex) {
        if (!active_hotcues.contains(hotCueIndex)) {
            HotcueControl* pControl = m_hotcueControls.at(hotCueIndex);
            detachCue(pControl);
        }
    }

    if (pIntroCue) {
        double startPosition = pIntroCue->getPosition();
        double endPosition = pIntroCue->getEndPosition();

        m_pIntroStartPosition->set(quantizeCuePoint(startPosition));
        m_pIntroStartEnabled->forceSet(
                startPosition == Cue::kNoPosition ? 0.0 : 1.0);
        m_pIntroEndPosition->set(quantizeCuePoint(endPosition));
        m_pIntroEndEnabled->forceSet(
                endPosition == Cue::kNoPosition ? 0.0 : 1.0);
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
        m_pOutroStartEnabled->forceSet(
                startPosition == Cue::kNoPosition ? 0.0 : 1.0);
        m_pOutroEndPosition->set(quantizeCuePoint(endPosition));
        m_pOutroEndEnabled->forceSet(
                endPosition == Cue::kNoPosition ? 0.0 : 1.0);
    } else {
        m_pOutroStartPosition->set(Cue::kNoPosition);
        m_pOutroStartEnabled->forceSet(0.0);
        m_pOutroEndPosition->set(Cue::kNoPosition);
        m_pOutroEndEnabled->forceSet(0.0);
    }

    // Because of legacy, we store the main cue point twice and need to
    // sync both values.
    // The mixxx::CueType::MainCue from getCuePoints() has the priority
    CuePosition mainCuePoint;
    if (pMainCue) {
        double position = pMainCue->getPosition();
        mainCuePoint.setPosition(position);
        // adjust the track cue accordingly
        m_pLoadedTrack->setCuePoint(mainCuePoint);
    } else {
        // If no load cue point is stored, read from track
        // Note: This is 0:00 for new tracks
        mainCuePoint = m_pLoadedTrack->getCuePoint();
        // Than add the load cue to the list of cue
        CuePointer pCue = m_pLoadedTrack->createAndAddCue(
                mixxx::CueType::MainCue,
                Cue::kNoHotCue,
                mainCuePoint.getPosition(),
                Cue::kNoPosition);
    }
    m_pCuePoint->set(quantizeCuePoint(mainCuePoint.getPosition()));
}

void CueControl::trackAnalyzed() {
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

void CueControl::hotcueSet(HotcueControl* pControl, double value, HotcueSetMode mode) {
    //qDebug() << "CueControl::hotcueSet" << value;

    if (value <= 0) {
        return;
    }

    QMutexLocker lock(&m_trackMutex);
    if (!m_pLoadedTrack) {
        return;
    }

    // Note: the cue is just detached from the hotcue control
    // It remains in the database for later use
    // TODO: find a rule, that allows us to delete the cue as well
    // https://bugs.launchpad.net/mixxx/+bug/1653276
    hotcueClear(pControl, value);

    double cueStartPosition = Cue::kNoPosition;
    double cueEndPosition = Cue::kNoPosition;
    mixxx::CueType cueType = mixxx::CueType::Invalid;

    bool loopEnabled = m_pLoopEnabled->toBool();
    if (mode == HotcueSetMode::Auto) {
        mode = loopEnabled ? HotcueSetMode::Loop : HotcueSetMode::Cue;
    }

    switch (mode) {
    case HotcueSetMode::Cue: {
        // If no loop is enabled, just store regular jump cue
        cueStartPosition = getQuantizedCurrentPosition();
        cueType = mixxx::CueType::HotCue;
        break;
    }
    case HotcueSetMode::Loop: {
        if (loopEnabled) {
            // If a loop is enabled, save the current loop
            cueStartPosition = m_pLoopStartPosition->get();
            cueEndPosition = m_pLoopEndPosition->get();
        } else {
            // If no loop is enabled, save a loop starting from the current
            // position and with the current beatloop size
            cueStartPosition = getQuantizedCurrentPosition();
            double beatloopSize = m_pBeatLoopSize->get();
            const mixxx::BeatsPointer pBeats = m_pLoadedTrack->getBeats();
            if (beatloopSize <= 0 || !pBeats) {
                return;
            }
            cueEndPosition = pBeats->findNBeatsFromSample(cueStartPosition, beatloopSize);
        }
        cueType = mixxx::CueType::Loop;
        break;
    }
    default:
        DEBUG_ASSERT(!"Invalid HotcueSetMode");
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(cueType != mixxx::CueType::Invalid) {
        return;
    }

    // Abort if no position has been found.
    VERIFY_OR_DEBUG_ASSERT(cueStartPosition != Cue::kNoPosition &&
            (cueType != mixxx::CueType::Loop ||
                    cueEndPosition != Cue::kNoPosition)) {
        return;
    }

    int hotcueIndex = pControl->getHotcueIndex();

    CuePointer pCue = m_pLoadedTrack->createAndAddCue(
            cueType,
            hotcueIndex,
            cueStartPosition,
            cueEndPosition);

    // TODO(XXX) deal with spurious signals
    attachCue(pCue, pControl);

    if (cueType == mixxx::CueType::Loop) {
        ConfigKey autoLoopColorsKey("[Controls]", "auto_loop_colors");
        if (getConfig()->getValue(autoLoopColorsKey, false)) {
            auto hotcueColorPalette =
                    m_colorPaletteSettings.getHotcueColorPalette();
            pCue->setColor(hotcueColorPalette.colorForHotcueIndex(hotcueIndex));
        } else {
            pCue->setColor(mixxx::PredefinedColorPalettes::kDefaultLoopColor);
        }
    } else {
        ConfigKey autoHotcueColorsKey("[Controls]", "auto_hotcue_colors");
        if (getConfig()->getValue(autoHotcueColorsKey, false)) {
            auto hotcueColorPalette =
                    m_colorPaletteSettings.getHotcueColorPalette();
            pCue->setColor(hotcueColorPalette.colorForHotcueIndex(hotcueIndex));
        } else {
            pCue->setColor(mixxx::PredefinedColorPalettes::kDefaultCueColor);
        }
    }

    if (cueType == mixxx::CueType::Loop) {
        setCurrentSavedLoopControlAndActivate(pControl);
    }

    // If quantize is enabled and we are not playing, jump to the cue point
    // since it's not necessarily where we currently are. TODO(XXX) is this
    // potentially invalid for vinyl control?
    bool playing = m_pPlay->toBool();
    if (!playing && m_pQuantizeEnabled->toBool()) {
        lock.unlock(); // prevent deadlock.
        // Enginebuffer will quantize more exactly than we can.
        seekAbs(cueStartPosition);
    }
}

void CueControl::hotcueGoto(HotcueControl* pControl, double value) {
    if (value <= 0) {
        return;
    }
    double position = pControl->getPosition();
    if (position != Cue::kNoPosition) {
        seekAbs(position);
    }
}

void CueControl::hotcueGotoAndStop(HotcueControl* pControl, double value) {
    if (value <= 0) {
        return;
    }

    double position = pControl->getPosition();
    if (position != Cue::kNoPosition) {
        if (m_currentlyPreviewingIndex == Cue::kNoHotCue) {
            m_pPlay->set(0.0);
            seekExact(position);
        } else {
            // this becomes a play latch command if we are previewing
            m_pPlay->set(0.0);
        }
    }
}

void CueControl::hotcueGotoAndPlay(HotcueControl* pControl, double value) {
    if (value <= 0) {
        return;
    }
    double position = pControl->getPosition();
    if (position != Cue::kNoPosition) {
        seekAbs(position);
        // End previewing to not jump back if a sticking finger on a cue
        // button is released (just in case)
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        if (!m_pPlay->toBool()) {
            // don't move the cue point to the hot cue point in DENON mode
            m_bypassCueSetByPlay = true;
            m_pPlay->set(1.0);
        }
    }

    setHotcueFocusIndex(pControl->getHotcueIndex());
}

void CueControl::hotcueGotoAndLoop(HotcueControl* pControl, double value) {
    if (value == 0) {
        return;
    }
    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }

    double startPosition = pCue->getPosition();
    if (startPosition == Cue::kNoPosition) {
        return;
    }

    if (pCue->getType() == mixxx::CueType::Loop) {
        seekAbs(startPosition);
        setCurrentSavedLoopControlAndActivate(pControl);
    } else if (pCue->getType() == mixxx::CueType::HotCue) {
        seekAbs(startPosition);
        setBeatLoop(startPosition, true);
    } else {
        return;
    }

    // End previewing to not jump back if a sticking finger on a cue
    // button is released (just in case)
    updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
    if (!m_pPlay->toBool()) {
        // don't move the cue point to the hot cue point in DENON mode
        m_bypassCueSetByPlay = true;
        m_pPlay->set(1.0);
    }

    setHotcueFocusIndex(pControl->getHotcueIndex());
}

void CueControl::hotcueCueLoop(HotcueControl* pControl, double value) {
    if (value == 0) {
        return;
    }

    CuePointer pCue = pControl->getCue();

    if (!pCue || pCue->getPosition() == Cue::kNoPosition) {
        hotcueSet(pControl, value, HotcueSetMode::Cue);
        pCue = pControl->getCue();
        VERIFY_OR_DEBUG_ASSERT(pCue && pCue->getPosition() != Cue::kNoPosition) {
            return;
        }
    }

    switch (pCue->getType()) {
    case mixxx::CueType::Loop: {
        // The hotcue_X_cueloop CO was invoked for a saved loop, set it as
        // active the first time this happens and toggle the loop_enabled state
        // on subsequent invocations.
        if (m_pCurrentSavedLoopControl != pControl) {
            setCurrentSavedLoopControlAndActivate(pControl);
        } else {
            bool loopActive = pControl->getStatus() == HotcueControl::Status::Active;
            Cue::StartAndEndPositions pos = pCue->getStartAndEndPosition();
            setLoop(pos.startPosition, pos.endPosition, !loopActive);
        }
    } break;
    case mixxx::CueType::HotCue: {
        // The hotcue_X_cueloop CO was invoked for a hotcue. In that case,
        // create a beatloop starting at the hotcue position. This is useful for
        // mapping the CUE LOOP mode labeled on some controllers.
        setCurrentSavedLoopControlAndActivate(nullptr);
        double startPosition = pCue->getPosition();
        bool loopActive = m_pLoopEnabled->toBool() &&
                (startPosition == m_pLoopStartPosition->get());
        setBeatLoop(startPosition, !loopActive);
        break;
    }
    default:
        return;
    }

    setHotcueFocusIndex(pControl->getHotcueIndex());
}

void CueControl::hotcueActivate(HotcueControl* pControl, double value, HotcueSetMode mode) {
    //qDebug() << "CueControl::hotcueActivate" << value;

    CuePointer pCue = pControl->getCue();
    if (value > 0) {
        // pressed
        if (pCue && pCue->getPosition() != Cue::kNoPosition &&
                pCue->getType() != mixxx::CueType::Invalid) {
            if (m_pPlay->toBool() && m_currentlyPreviewingIndex == Cue::kNoHotCue) {
                // playing by Play button
                switch (pCue->getType()) {
                case mixxx::CueType::HotCue:
                    hotcueGoto(pControl, value);
                    break;
                case mixxx::CueType::Loop:
                    if (m_pCurrentSavedLoopControl != pControl) {
                        setCurrentSavedLoopControlAndActivate(pControl);
                    } else {
                        bool loopActive = pControl->getStatus() ==
                                HotcueControl::Status::Active;
                        Cue::StartAndEndPositions pos = pCue->getStartAndEndPosition();
                        setLoop(pos.startPosition, pos.endPosition, !loopActive);
                    }
                    break;
                default:
                    DEBUG_ASSERT(!"Invalid CueType!");
                }
            } else {
                // pressed during pause or preview
                hotcueActivatePreview(pControl, value);
            }
        } else {
            // pressed a not existing cue
            hotcueSet(pControl, value, mode);
        }
    } else {
        // released
        hotcueActivatePreview(pControl, value);
    }

    setHotcueFocusIndex(pControl->getHotcueIndex());
}

void CueControl::hotcueActivatePreview(HotcueControl* pControl, double value) {
    CuePointer pCue = pControl->getCue();
    int index = pControl->getHotcueIndex();
    if (value > 0) {
        if (m_currentlyPreviewingIndex != index) {
            pControl->cachePreviewingStartState();
            double position = pControl->getPreviewingPosition();
            mixxx::CueType type = pControl->getPreviewingType();
            ;
            if (type != mixxx::CueType::Invalid &&
                    position != Cue::kNoPosition) {
                updateCurrentlyPreviewingIndex(index);
                m_bypassCueSetByPlay = true;
                if (type == mixxx::CueType::Loop) {
                    setCurrentSavedLoopControlAndActivate(pControl);
                } else if (pControl->getStatus() == HotcueControl::Status::Set) {
                    pControl->setStatus(HotcueControl::Status::Active);
                }
                seekAbs(position);
                m_pPlay->set(1.0);
            }
        }
    } else if (m_currentlyPreviewingIndex == index) {
        // This is a release of a previewing hotcue
        double position = pControl->getPreviewingPosition();
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        m_pPlay->set(0.0);
        seekExact(position);
    }

    setHotcueFocusIndex(pControl->getHotcueIndex());
}

void CueControl::updateCurrentlyPreviewingIndex(int hotcueIndex) {
    int oldPreviewingIndex = m_currentlyPreviewingIndex.fetchAndStoreRelease(hotcueIndex);
    if (oldPreviewingIndex >= 0 && oldPreviewingIndex < m_iNumHotCues) {
        // We where already in previewing state, clean up ..
        HotcueControl* pLastControl = m_hotcueControls.at(oldPreviewingIndex);
        mixxx::CueType lastType = pLastControl->getPreviewingType();
        if (lastType == mixxx::CueType::Loop) {
            m_pLoopEnabled->set(0);
        }
        CuePointer pLastCue(pLastControl->getCue());
        pLastControl->setStatus((pLastCue->getType() == mixxx::CueType::Invalid)
                        ? HotcueControl::Status::Empty
                        : HotcueControl::Status::Set);
    }
}

void CueControl::hotcueClear(HotcueControl* pControl, double value) {
    if (value <= 0) {
        return;
    }

    QMutexLocker lock(&m_trackMutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }
    detachCue(pControl);
    m_pLoadedTrack->removeCue(pCue);
    setHotcueFocusIndex(Cue::kNoHotCue);
}

void CueControl::hotcuePositionChanged(
        HotcueControl* pControl, double newPosition) {
    QMutexLocker lock(&m_trackMutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue = pControl->getCue();
    if (pCue) {
        // Setting the position to Cue::kNoPosition is the same as calling hotcue_x_clear
        if (newPosition == Cue::kNoPosition) {
            detachCue(pControl);
        } else if (newPosition > 0 && newPosition < m_pTrackSamples->get()) {
            if (pCue->getType() == mixxx::CueType::Loop && newPosition >= pCue->getEndPosition()) {
                return;
            }
            pCue->setStartPosition(newPosition);
        }
    }
}

void CueControl::hotcueEndPositionChanged(
        HotcueControl* pControl, double newEndPosition) {
    CuePointer pCue = pControl->getCue();
    if (pCue) {
        // Setting the end position of a loop cue to Cue::kNoPosition converts
        // it into a regular jump cue
        if (pCue->getType() == mixxx::CueType::Loop &&
                newEndPosition == Cue::kNoPosition) {
            pCue->setType(mixxx::CueType::HotCue);
            pCue->setEndPosition(Cue::kNoPosition);
        } else {
            if (newEndPosition > pCue->getPosition()) {
                pCue->setEndPosition(newEndPosition);
            }
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
    if (value <= 0) {
        return;
    }

    QMutexLocker lock(&m_trackMutex);
    double cue = getQuantizedCurrentPosition();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Store cue point in loaded track
    // The m_pCuePoint CO is set via loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        pLoadedTrack->setCuePoint(CuePosition(cue));
    }
}

void CueControl::cueClear(double value) {
    if (value <= 0) {
        return;
    }

    // the m_pCuePoint CO is set via loadCuesFromTrack()
    // no locking required
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    if (pLoadedTrack) {
        pLoadedTrack->setCuePoint(CuePosition());
    }
}

void CueControl::cueGoto(double value) {
    if (value <= 0) {
        return;
    }

    QMutexLocker lock(&m_trackMutex);
    // Seek to cue point
    double cuePoint = m_pCuePoint->get();

    // Note: We do not mess with play here, we continue playing or previewing.

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    seekAbs(cuePoint);
}

void CueControl::cueGotoAndPlay(double value) {
    if (value <= 0) {
        return;
    }

    cueGoto(value);
    QMutexLocker lock(&m_trackMutex);
    // Start playing if not already

    // End previewing to not jump back if a sticking finger on a cue
    // button is released (just in case)
    updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
    if (!m_pPlay->toBool()) {
        // don't move the cue point to the hot cue point in DENON mode
        m_bypassCueSetByPlay = true;
        m_pPlay->set(1.0);
    }
}

void CueControl::cueGotoAndStop(double value) {
    if (value <= 0) {
        return;
    }

    if (m_currentlyPreviewingIndex == Cue::kNoHotCue) {
        m_pPlay->set(0.0);
        double position = m_pCuePoint->get();
        seekExact(position);
    } else {
        // this becomes a play latch command if we are previewing
        m_pPlay->set(0.0);
    }
}

void CueControl::cuePreview(double value) {
    if (value > 0) {
        if (m_currentlyPreviewingIndex != kMainCueIndex) {
            updateCurrentlyPreviewingIndex(kMainCueIndex);
            seekAbs(m_pCuePoint->get());
            m_pPlay->set(1.0);
        }
    } else if (m_currentlyPreviewingIndex == kMainCueIndex) {
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        m_pPlay->set(0.0);
        seekExact(m_pCuePoint->get());
    }
}

void CueControl::cueCDJ(double value) {
    // This is how Pioneer cue buttons work:
    // If pressed while freely playing (i.e. playing and platter NOT being touched), stop playback and go to cue.
    // If pressed while NOT freely playing (i.e. stopped or playing but platter IS being touched), set new cue point.
    // If pressed while stopped and at cue, play while pressed.
    // If play is pressed while holding cue, the deck is now playing. (Handled in playFromCuePreview().)

    const auto freely_playing =
            m_pPlay->toBool() && !getEngineBuffer()->getScratching();
    TrackAt trackAt = getTrackAt();

    if (value > 0) {
        if (m_currentlyPreviewingIndex == kMainCueIndex) {
            // already previewing, do nothing
            return;
        } else if (m_currentlyPreviewingIndex != Cue::kNoHotCue) {
            // we are already previewing by hotcues
            // just jump to cue point and continue previewing
            updateCurrentlyPreviewingIndex(kMainCueIndex);
            seekAbs(m_pCuePoint->get());
        } else if (freely_playing || trackAt == TrackAt::End) {
            // Jump to cue when playing or when at end position
            m_pPlay->set(0.0);
            seekAbs(m_pCuePoint->get());
        } else if (trackAt == TrackAt::Cue) {
            // paused at cue point
            updateCurrentlyPreviewingIndex(kMainCueIndex);
            m_pPlay->set(1.0);
        } else {
            // Paused not at cue point and not at end position
            cueSet(value);

            // If quantize is enabled, jump to the cue point since it's not
            // necessarily where we currently are
            if (m_pQuantizeEnabled->toBool()) {
                // Enginebuffer will quantize more exactly than we can.
                seekAbs(m_pCuePoint->get());
            }
        }
    } else if (m_currentlyPreviewingIndex == kMainCueIndex) {
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        m_pPlay->set(0.0);
        // Need to unlock before emitting any signals to prevent deadlock.
        seekExact(m_pCuePoint->get());
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

    bool playing = (m_pPlay->toBool());
    TrackAt trackAt = getTrackAt();

    if (value > 0) {
        if (m_currentlyPreviewingIndex == kMainCueIndex) {
            // already previewing, do nothing
            return;
        } else if (m_currentlyPreviewingIndex != Cue::kNoHotCue) {
            // we are already previewing by hotcues
            // just jump to cue point and continue previewing
            updateCurrentlyPreviewingIndex(kMainCueIndex);
            seekAbs(m_pCuePoint->get());
        } else if (!playing && trackAt == TrackAt::Cue) {
            // paused at cue point
            updateCurrentlyPreviewingIndex(kMainCueIndex);
            m_pPlay->set(1.0);
        } else {
            m_pPlay->set(0.0);
            seekExact(m_pCuePoint->get());
        }
    } else if (m_currentlyPreviewingIndex == kMainCueIndex) {
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        m_pPlay->set(0.0);
        seekExact(m_pCuePoint->get());
    }
}

void CueControl::cuePlay(double value) {
    // This is how CUP button works:
    // If freely playing (i.e. playing and platter NOT being touched), press to go to cue and stop.
    // If not freely playing (i.e. stopped or platter IS being touched), press to go to cue and stop.
    // On release, start playing from cue point.

    const auto freely_playing =
            m_pPlay->toBool() && !getEngineBuffer()->getScratching();
    TrackAt trackAt = getTrackAt();

    // pressed
    if (value > 0) {
        if (freely_playing) {
            updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
            m_pPlay->set(0.0);
            seekAbs(m_pCuePoint->get());
        } else if (trackAt == TrackAt::ElseWhere) {
            // Pause not at cue point and not at end position
            cueSet(value);
            // Just in case.
            updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
            m_pPlay->set(0.0);
            // If quantize is enabled, jump to the cue point since it's not
            // necessarily where we currently are
            if (m_pQuantizeEnabled->toBool()) {
                // Enginebuffer will quantize more exactly than we can.
                seekAbs(m_pCuePoint->get());
            }
        }
    } else if (trackAt == TrackAt::Cue) {
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        m_pPlay->set(1.0);
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
    QMutexLocker lock(&m_trackMutex);
    //qDebug() << "CueControl::pause()" << v;
    if (v > 0.0) {
        m_pPlay->set(0.0);
    }
}

void CueControl::playStutter(double v) {
    QMutexLocker lock(&m_trackMutex);
    //qDebug() << "playStutter" << v;
    if (v > 0.0) {
        if (m_pPlay->toBool()) {
            if (m_currentlyPreviewingIndex != Cue::kNoHotCue) {
                // latch playing
                updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
            } else {
                // Stutter
                cueGoto(1.0);
            }
        } else {
            m_pPlay->set(1.0);
        }
    }
}

void CueControl::introStartSet(double value) {
    if (value <= 0) {
        return;
    }

    QMutexLocker lock(&m_trackMutex);

    double position = getQuantizedCurrentPosition();

    // Make sure user is not trying to place intro start cue on or after
    // other intro/outro cues.
    double introEnd = m_pIntroEndPosition->get();
    double outroStart = m_pOutroStartPosition->get();
    double outroEnd = m_pOutroEndPosition->get();
    if (introEnd != Cue::kNoPosition && position >= introEnd) {
        qWarning()
                << "Trying to place intro start cue on or after intro end cue.";
        return;
    }
    if (outroStart != Cue::kNoPosition && position >= outroStart) {
        qWarning() << "Trying to place intro start cue on or after outro start "
                      "cue.";
        return;
    }
    if (outroEnd != Cue::kNoPosition && position >= outroEnd) {
        qWarning()
                << "Trying to place intro start cue on or after outro end cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Intro);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue(
                    mixxx::CueType::Intro,
                    Cue::kNoHotCue,
                    position,
                    introEnd);
        } else {
            pCue->setStartAndEndPosition(position, introEnd);
        }
    }
}

void CueControl::introStartClear(double value) {
    if (value <= 0) {
        return;
    }

    QMutexLocker lock(&m_trackMutex);
    double introEnd = m_pIntroEndPosition->get();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
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
    if (value <= 0) {
        return;
    }

    double introStart = m_pIntroStartPosition->get();
    if (introStart == Cue::kNoPosition) {
        introStartSet(1.0);
    } else {
        seekAbs(introStart);
    }
}

void CueControl::introEndSet(double value) {
    if (value <= 0) {
        return;
    }

    QMutexLocker lock(&m_trackMutex);

    double position = getQuantizedCurrentPosition();

    // Make sure user is not trying to place intro end cue on or before
    // intro start cue, or on or after outro start/end cue.
    double introStart = m_pIntroStartPosition->get();
    double outroStart = m_pOutroStartPosition->get();
    double outroEnd = m_pOutroEndPosition->get();
    if (introStart != Cue::kNoPosition && position <= introStart) {
        qWarning() << "Trying to place intro end cue on or before intro start "
                      "cue.";
        return;
    }
    if (outroStart != Cue::kNoPosition && position >= outroStart) {
        qWarning()
                << "Trying to place intro end cue on or after outro start cue.";
        return;
    }
    if (outroEnd != Cue::kNoPosition && position >= outroEnd) {
        qWarning()
                << "Trying to place intro end cue on or after outro end cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Intro);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue(
                    mixxx::CueType::Intro,
                    Cue::kNoHotCue,
                    introStart,
                    position);
        } else {
            pCue->setStartAndEndPosition(introStart, position);
        }
    }
}

void CueControl::introEndClear(double value) {
    if (value <= 0) {
        return;
    }

    QMutexLocker lock(&m_trackMutex);
    double introStart = m_pIntroStartPosition->get();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
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

    QMutexLocker lock(&m_trackMutex);
    double introEnd = m_pIntroEndPosition->get();
    lock.unlock();

    if (introEnd == Cue::kNoPosition) {
        introEndSet(1.0);
    } else {
        seekAbs(introEnd);
    }
}

void CueControl::outroStartSet(double value) {
    if (value <= 0) {
        return;
    }

    QMutexLocker lock(&m_trackMutex);

    double position = getQuantizedCurrentPosition();

    // Make sure user is not trying to place outro start cue on or before
    // intro end cue or on or after outro end cue.
    double introStart = m_pIntroStartPosition->get();
    double introEnd = m_pIntroEndPosition->get();
    double outroEnd = m_pOutroEndPosition->get();
    if (introStart != Cue::kNoPosition && position <= introStart) {
        qWarning() << "Trying to place outro start cue on or before intro "
                      "start cue.";
        return;
    }
    if (introEnd != Cue::kNoPosition && position <= introEnd) {
        qWarning() << "Trying to place outro start cue on or before intro end "
                      "cue.";
        return;
    }
    if (outroEnd != Cue::kNoPosition && position >= outroEnd) {
        qWarning()
                << "Trying to place outro start cue on or after outro end cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Outro);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue(
                    mixxx::CueType::Outro,
                    Cue::kNoHotCue,
                    position,
                    outroEnd);
        } else {
            pCue->setStartAndEndPosition(position, outroEnd);
        }
    }
}

void CueControl::outroStartClear(double value) {
    if (value <= 0) {
        return;
    }

    QMutexLocker lock(&m_trackMutex);
    double outroEnd = m_pOutroEndPosition->get();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
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
    if (value <= 0) {
        return;
    }

    QMutexLocker lock(&m_trackMutex);
    double outroStart = m_pOutroStartPosition->get();
    lock.unlock();

    if (outroStart == Cue::kNoPosition) {
        outroStartSet(1.0);
    } else {
        seekAbs(outroStart);
    }
}

void CueControl::outroEndSet(double value) {
    if (value <= 0) {
        return;
    }

    QMutexLocker lock(&m_trackMutex);

    double position = getQuantizedCurrentPosition();

    // Make sure user is not trying to place outro end cue on or before
    // other intro/outro cues.
    double introStart = m_pIntroStartPosition->get();
    double introEnd = m_pIntroEndPosition->get();
    double outroStart = m_pOutroStartPosition->get();
    if (introStart != Cue::kNoPosition && position <= introStart) {
        qWarning() << "Trying to place outro end cue on or before intro start "
                      "cue.";
        return;
    }
    if (introEnd != Cue::kNoPosition && position <= introEnd) {
        qWarning()
                << "Trying to place outro end cue on or before intro end cue.";
        return;
    }
    if (outroStart != Cue::kNoPosition && position <= outroStart) {
        qWarning() << "Trying to place outro end cue on or before outro start "
                      "cue.";
        return;
    }

    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Outro);
        if (!pCue) {
            pCue = pLoadedTrack->createAndAddCue(
                    mixxx::CueType::Outro,
                    Cue::kNoHotCue,
                    outroStart,
                    position);
        } else {
            pCue->setStartAndEndPosition(outroStart, position);
        }
    }
}

void CueControl::outroEndClear(double value) {
    if (value <= 0) {
        return;
    }

    QMutexLocker lock(&m_trackMutex);
    double outroStart = m_pOutroStartPosition->get();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
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
    if (value <= 0) {
        return;
    }

    QMutexLocker lock(&m_trackMutex);
    double outroEnd = m_pOutroEndPosition->get();
    lock.unlock();

    if (outroEnd == Cue::kNoPosition) {
        outroEndSet(1.0);
    } else {
        seekAbs(outroEnd);
    }
}

// This is also called from the engine thread. No locking allowed.
bool CueControl::updateIndicatorsAndModifyPlay(
        bool newPlay, bool oldPlay, bool playPossible) {
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
    if (m_currentlyPreviewingIndex != Cue::kNoHotCue) {
        if (!newPlay && oldPlay) {
            // play latch request: stop previewing and go into normal play mode.
            updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
            newPlay = true;
            m_pPlayLatched->forceSet(1.0);
        } else {
            previewing = true;
            m_pPlayLatched->forceSet(0.0);
        }
    }

    TrackAt trackAt = getTrackAt();

    if (!playPossible) {
        // play not possible
        newPlay = false;
        m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
        m_pStopButton->set(0.0);
        m_pPlayLatched->forceSet(0.0);
    } else if (newPlay && !previewing) {
        // Play: Indicates a latched Play
        m_pPlayIndicator->setBlinkValue(ControlIndicator::ON);
        m_pStopButton->set(0.0);
        m_pPlayLatched->forceSet(1.0);
    } else {
        // Pause:
        m_pStopButton->set(1.0);
        m_pPlayLatched->forceSet(0.0);
        if (cueMode == CueMode::Denon) {
            if (trackAt == TrackAt::Cue || previewing) {
                m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
            } else {
                // Flashing indicates that a following play would move cue point
                m_pPlayIndicator->setBlinkValue(
                        ControlIndicator::RATIO1TO1_500MS);
            }
        } else if (cueMode == CueMode::Mixxx ||
                cueMode == CueMode::MixxxNoBlinking ||
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
                    m_pCueIndicator->setBlinkValue(
                            ControlIndicator::RATIO1TO1_500MS);
                } else if (cueMode == CueMode::MixxxNoBlinking) {
                    m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
                } else {
                    // in Pioneer mode Cue Button is flashing fast if CUE will move Cue point
                    m_pCueIndicator->setBlinkValue(
                            ControlIndicator::RATIO1TO1_250MS);
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
                    m_pPlayIndicator->setBlinkValue(
                            ControlIndicator::RATIO1TO1_500MS);
                } else {
                    // At track end
                    m_pPlayIndicator->setBlinkValue(ControlIndicator::OFF);
                }
            }
        }
    } else {
        // Here we have CUE_MODE_PIONEER or CUE_MODE_MIXXX
        // default to Pioneer mode
        if (m_currentlyPreviewingIndex != kMainCueIndex) {
            const auto freely_playing =
                    m_pPlay->toBool() && !getEngineBuffer()->getScratching();
            if (!freely_playing) {
                switch (trackAt) {
                case TrackAt::ElseWhere:
                    if (cueMode == CUE_MODE_MIXXX) {
                        // in Mixxx mode Cue Button is flashing slow if CUE will move Cue point
                        m_pCueIndicator->setBlinkValue(
                                ControlIndicator::RATIO1TO1_500MS);
                    } else if (cueMode == CUE_MODE_MIXXX_NO_BLINK) {
                        m_pCueIndicator->setBlinkValue(ControlIndicator::OFF);
                    } else {
                        // in Pioneer mode Cue Button is flashing fast if CUE will move Cue point
                        m_pCueIndicator->setBlinkValue(
                                ControlIndicator::RATIO1TO1_250MS);
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
        } else {
            // Preview
            m_pCueIndicator->setBlinkValue(ControlIndicator::ON);
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

    const mixxx::BeatsPointer pBeats = m_pLoadedTrack->getBeats();
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
    return (fabs(getSampleOfTrack().current - m_pIntroStartPosition->get()) <
            1.0f);
}

SeekOnLoadMode CueControl::getSeekOnLoadPreference() {
    int configValue =
            getConfig()->getValue(ConfigKey("[Controls]", "CueRecall"),
                    static_cast<int>(SeekOnLoadMode::IntroStart));
    return static_cast<SeekOnLoadMode>(configValue);
}

void CueControl::hotcueFocusColorPrev(double value) {
    if (value <= 0) {
        return;
    }

    int hotcueIndex = getHotcueFocusIndex();
    if (hotcueIndex < 0 || hotcueIndex >= m_hotcueControls.size()) {
        return;
    }

    HotcueControl* pControl = m_hotcueControls.at(hotcueIndex);
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
    if (value <= 0) {
        return;
    }

    int hotcueIndex = getHotcueFocusIndex();
    if (hotcueIndex < 0 || hotcueIndex >= m_hotcueControls.size()) {
        return;
    }

    HotcueControl* pControl = m_hotcueControls.at(hotcueIndex);
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

void CueControl::setCurrentSavedLoopControlAndActivate(HotcueControl* pControl) {
    HotcueControl* pOldSavedLoopControl = m_pCurrentSavedLoopControl.fetchAndStoreAcquire(nullptr);
    if (pOldSavedLoopControl && pOldSavedLoopControl != pControl) {
        // Disable previous saved loop
        DEBUG_ASSERT(pOldSavedLoopControl->getStatus() != HotcueControl::Status::Empty);
        pOldSavedLoopControl->setStatus(HotcueControl::Status::Set);
    }

    if (!pControl) {
        return;
    }
    CuePointer pCue = pControl->getCue();
    VERIFY_OR_DEBUG_ASSERT(pCue) {
        return;
    }

    mixxx::CueType type = pCue->getType();
    Cue::StartAndEndPositions pos = pCue->getStartAndEndPosition();

    VERIFY_OR_DEBUG_ASSERT(
            type == mixxx::CueType::Loop &&
            pos.endPosition != Cue::kNoPosition) {
        return;
    }

    // Set new control as active
    setLoop(pos.startPosition, pos.endPosition, true);
    pControl->setStatus(HotcueControl::Status::Active);
    m_pCurrentSavedLoopControl.storeRelease(pControl);
}

void CueControl::slotLoopReset() {
    setCurrentSavedLoopControlAndActivate(nullptr);
}

void CueControl::slotLoopEnabledChanged(bool enabled) {
    HotcueControl* pSavedLoopControl = m_pCurrentSavedLoopControl;
    if (!pSavedLoopControl) {
        return;
    }

    DEBUG_ASSERT(pSavedLoopControl->getStatus() != HotcueControl::Status::Empty);
    DEBUG_ASSERT(
            pSavedLoopControl->getCue() &&
            pSavedLoopControl->getCue()->getPosition() ==
                    m_pLoopStartPosition->get());
    DEBUG_ASSERT(
            pSavedLoopControl->getCue() &&
            pSavedLoopControl->getCue()->getEndPosition() ==
                    m_pLoopEndPosition->get());

    if (enabled) {
        pSavedLoopControl->setStatus(HotcueControl::Status::Active);
    } else {
        pSavedLoopControl->setStatus(HotcueControl::Status::Set);
    }
}

void CueControl::slotLoopUpdated(double startPosition, double endPosition) {
    HotcueControl* pSavedLoopControl = m_pCurrentSavedLoopControl;
    if (!pSavedLoopControl) {
        return;
    }

    if (pSavedLoopControl->getStatus() != HotcueControl::Status::Active) {
        slotLoopReset();
        return;
    }

    CuePointer pCue = pSavedLoopControl->getCue();
    if (!pCue) {
        // this can happen if the cue is deleted while this slot is cued
        return;
    }

    VERIFY_OR_DEBUG_ASSERT(pCue->getType() == mixxx::CueType::Loop) {
        setCurrentSavedLoopControlAndActivate(nullptr);
        return;
    }

    DEBUG_ASSERT(startPosition != Cue::kNoPosition);
    DEBUG_ASSERT(endPosition != Cue::kNoPosition);
    DEBUG_ASSERT(startPosition < endPosition);

    DEBUG_ASSERT(pSavedLoopControl->getStatus() == HotcueControl::Status::Active);
    pCue->setStartPosition(startPosition);
    pCue->setEndPosition(endPosition);
    DEBUG_ASSERT(pSavedLoopControl->getStatus() == HotcueControl::Status::Active);
}

void CueControl::setHotcueFocusIndex(int hotcueIndex) {
    m_pHotcueFocus->set(hotcueIndexToHotcueNumber(hotcueIndex));
}

int CueControl::getHotcueFocusIndex() const {
    return hotcueNumberToHotcueIndex(static_cast<int>(m_pHotcueFocus->get()));
}

ConfigKey HotcueControl::keyForControl(const QString& name) {
    ConfigKey key;
    key.group = m_group;
    // Add one to hotcue so that we don't have a hotcue_0
    key.item = QStringLiteral("hotcue_") +
            QString::number(hotcueIndexToHotcueNumber(m_hotcueIndex)) +
            QChar('_') + name;
    return key;
}

HotcueControl::HotcueControl(const QString& group, int hotcueIndex)
        : m_group(group),
          m_hotcueIndex(hotcueIndex),
          m_pCue(nullptr) {
    m_hotcuePosition = std::make_unique<ControlObject>(keyForControl(QStringLiteral("position")));
    connect(m_hotcuePosition.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcuePositionChanged,
            Qt::DirectConnection);
    m_hotcuePosition->set(Cue::kNoPosition);

    m_hotcueEndPosition = std::make_unique<ControlObject>(
            keyForControl(QStringLiteral("endposition")));
    connect(m_hotcueEndPosition.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueEndPositionChanged,
            Qt::DirectConnection);
    m_hotcueEndPosition->set(Cue::kNoPosition);

    m_pHotcueStatus = std::make_unique<ControlObject>(keyForControl(QStringLiteral("status")));
    m_pHotcueStatus->setReadOnly();

    // Add an alias for the legacy hotcue_X_enabled CO
    ControlDoublePrivate::insertAlias(keyForControl(QStringLiteral("enabled")),
            keyForControl(QStringLiteral("status")));

    m_hotcueType = std::make_unique<ControlObject>(keyForControl(QStringLiteral("type")));
    m_hotcueType->setReadOnly();

    // The rgba value  of the color assigned to this color.
    m_hotcueColor = std::make_unique<ControlObject>(keyForControl(QStringLiteral("color")));
    m_hotcueColor->connectValueChangeRequest(
            this,
            &HotcueControl::slotHotcueColorChangeRequest,
            Qt::DirectConnection);
    connect(m_hotcueColor.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueColorChanged,
            Qt::DirectConnection);

    m_hotcueSet = std::make_unique<ControlPushButton>(keyForControl(QStringLiteral("set")));
    connect(m_hotcueSet.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueSet,
            Qt::DirectConnection);

    m_hotcueSetCue = std::make_unique<ControlPushButton>(keyForControl(QStringLiteral("setcue")));
    connect(m_hotcueSetCue.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueSetCue,
            Qt::DirectConnection);

    m_hotcueSetLoop = std::make_unique<ControlPushButton>(keyForControl(QStringLiteral("setloop")));
    connect(m_hotcueSetLoop.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueSetLoop,
            Qt::DirectConnection);

    m_hotcueGoto = std::make_unique<ControlPushButton>(keyForControl(QStringLiteral("goto")));
    connect(m_hotcueGoto.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueGoto,
            Qt::DirectConnection);

    m_hotcueGotoAndPlay = std::make_unique<ControlPushButton>(
            keyForControl(QStringLiteral("gotoandplay")));
    connect(m_hotcueGotoAndPlay.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueGotoAndPlay,
            Qt::DirectConnection);

    m_hotcueGotoAndStop = std::make_unique<ControlPushButton>(
            keyForControl(QStringLiteral("gotoandstop")));
    connect(m_hotcueGotoAndStop.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueGotoAndStop,
            Qt::DirectConnection);

    m_hotcueGotoAndLoop = std::make_unique<ControlPushButton>(
            keyForControl(QStringLiteral("gotoandloop")));
    connect(m_hotcueGotoAndLoop.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueGotoAndLoop,
            Qt::DirectConnection);

    // Enable/disable the loop associated with this hotcue (either a saved loop
    // or a beatloop from the hotcue position if this is a regular hotcue).
    m_hotcueCueLoop = std::make_unique<ControlPushButton>(keyForControl(QStringLiteral("cueloop")));
    connect(m_hotcueCueLoop.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueCueLoop,
            Qt::DirectConnection);

    m_hotcueActivate = std::make_unique<ControlPushButton>(
            keyForControl(QStringLiteral("activate")));
    connect(m_hotcueActivate.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueActivate,
            Qt::DirectConnection);

    m_hotcueActivateCue = std::make_unique<ControlPushButton>(
            keyForControl(QStringLiteral("activatecue")));
    connect(m_hotcueActivateCue.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueActivateCue,
            Qt::DirectConnection);

    m_hotcueActivateLoop = std::make_unique<ControlPushButton>(
            keyForControl(QStringLiteral("activateloop")));
    connect(m_hotcueActivateLoop.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueActivateLoop,
            Qt::DirectConnection);

    m_hotcueActivatePreview = std::make_unique<ControlPushButton>(
            keyForControl(QStringLiteral("activate_preview")));
    connect(m_hotcueActivatePreview.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueActivatePreview,
            Qt::DirectConnection);

    m_hotcueClear = std::make_unique<ControlPushButton>(keyForControl(QStringLiteral("clear")));
    connect(m_hotcueClear.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueClear,
            Qt::DirectConnection);

    m_previewingType.setValue(mixxx::CueType::Invalid);
    m_previewingPosition.setValue(Cue::kNoPosition);
}

HotcueControl::~HotcueControl() = default;

void HotcueControl::slotHotcueSet(double v) {
    emit hotcueSet(this, v, HotcueSetMode::Auto);
}

void HotcueControl::slotHotcueSetCue(double v) {
    emit hotcueSet(this, v, HotcueSetMode::Cue);
}

void HotcueControl::slotHotcueSetLoop(double v) {
    emit hotcueSet(this, v, HotcueSetMode::Loop);
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

void HotcueControl::slotHotcueGotoAndLoop(double v) {
    emit hotcueGotoAndLoop(this, v);
}

void HotcueControl::slotHotcueCueLoop(double v) {
    emit hotcueCueLoop(this, v);
}

void HotcueControl::slotHotcueActivate(double v) {
    emit hotcueActivate(this, v, HotcueSetMode::Auto);
}

void HotcueControl::slotHotcueActivateCue(double v) {
    emit hotcueActivate(this, v, HotcueSetMode::Cue);
}

void HotcueControl::slotHotcueActivateLoop(double v) {
    emit hotcueActivate(this, v, HotcueSetMode::Loop);
}

void HotcueControl::slotHotcueActivatePreview(double v) {
    emit hotcueActivatePreview(this, v);
}

void HotcueControl::slotHotcueClear(double v) {
    emit hotcueClear(this, v);
}

void HotcueControl::slotHotcuePositionChanged(double newPosition) {
    emit hotcuePositionChanged(this, newPosition);
}

void HotcueControl::slotHotcueEndPositionChanged(double newEndPosition) {
    emit hotcueEndPositionChanged(this, newEndPosition);
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

double HotcueControl::getEndPosition() const {
    return m_hotcueEndPosition->get();
}

void HotcueControl::setCue(const CuePointer& pCue) {
    DEBUG_ASSERT(!m_pCue);
    Cue::StartAndEndPositions pos = pCue->getStartAndEndPosition();
    setPosition(pos.startPosition);
    setEndPosition(pos.endPosition);
    setColor(pCue->getColor());
    setStatus((pCue->getType() == mixxx::CueType::Invalid)
                    ? HotcueControl::Status::Empty
                    : HotcueControl::Status::Set);
    setType(pCue->getType());
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
    setEndPosition(Cue::kNoPosition);
    setType(mixxx::CueType::Invalid);
    setStatus(Status::Empty);
}

void HotcueControl::setPosition(double position) {
    m_hotcuePosition->set(position);
}

void HotcueControl::setEndPosition(double endPosition) {
    m_hotcueEndPosition->set(endPosition);
}

void HotcueControl::setType(mixxx::CueType type) {
    m_hotcueType->forceSet(static_cast<double>(type));
}

void HotcueControl::setStatus(HotcueControl::Status status) {
    m_pHotcueStatus->forceSet(static_cast<double>(status));
}

HotcueControl::Status HotcueControl::getStatus() const {
    // Cast to int before casting to the int-based enum class because MSVC will
    // throw a hissy fit otherwise.
    return static_cast<Status>(static_cast<int>(m_pHotcueStatus->get()));
}
