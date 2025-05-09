#include "engine/controls/cuecontrol.h"

#include "control/controlindicator.h"
#include "control/controlobject.h"
#include "control/controlpushbutton.h"
#include "engine/enginebuffer.h"
#include "moc_cuecontrol.cpp"
#include "preferences/colorpalettesettings.h"
#include "track/track.h"
#include "util/color/predefinedcolorpalettes.h"
#include "util/defs.h"
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
constexpr int kNoHotCueNumber = 0;

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

void appendCueHint(gsl::not_null<HintVector*> pHintList,
        const mixxx::audio::FramePos& frame,
        Hint::Type type) {
    if (frame.isValid()) {
        const Hint cueHint = {
                /*.frame =*/static_cast<SINT>(frame.toLowerFrameBoundary().value()),
                /*.frameCount =*/Hint::kFrameCountForward,
                /*.type =*/type};
        pHintList->append(cueHint);
    }
}

void appendCueHint(gsl::not_null<HintVector*> pHintList, const double playPos, Hint::Type type) {
    const auto frame = mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(playPos);
    appendCueHint(pHintList, frame, type);
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
          m_pCurrentSavedLoopControl(nullptr),
          m_trackMutex(QT_RECURSIVE_MUTEX_INIT) {
    // To silence a compiler warning about CUE_MODE_PIONEER.
    Q_UNUSED(CUE_MODE_PIONEER);

    createControls();
    connectControls();

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

    m_pPassthrough = make_parented<ControlProxy>(group, "passthrough", this);
    m_pPassthrough->connectValueChanged(this,
            &CueControl::passthroughChanged,
            Qt::DirectConnection);

    m_pSortHotcuesByPos = std::make_unique<ControlPushButton>(ConfigKey(group, "sort_hotcues"));
    connect(m_pSortHotcuesByPos.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::setHotcueIndicesSortedByPosition,
            Qt::DirectConnection);
    m_pSortHotcuesByPosCompress = std::make_unique<ControlPushButton>(
            ConfigKey(group, "sort_hotcues_remove_offsets"));
    connect(m_pSortHotcuesByPosCompress.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::setHotcueIndicesSortedByPositionCompress,
            Qt::DirectConnection);
}

CueControl::~CueControl() {
    delete m_pCuePoint;
    delete m_pCueMode;
    qDeleteAll(m_hotcueControls);
}

void CueControl::createControls() {
    m_pCueSet = std::make_unique<ControlPushButton>(ConfigKey(m_group, "cue_set"));
    m_pCueSet->setButtonMode(mixxx::control::ButtonMode::Trigger);
    m_pCueClear = std::make_unique<ControlPushButton>(ConfigKey(m_group, "cue_clear"));
    m_pCueClear->setButtonMode(mixxx::control::ButtonMode::Trigger);
    m_pCueGoto = std::make_unique<ControlPushButton>(ConfigKey(m_group, "cue_goto"));
    m_pCueGotoAndPlay = std::make_unique<ControlPushButton>(ConfigKey(m_group, "cue_gotoandplay"));
    m_pCuePlay = std::make_unique<ControlPushButton>(ConfigKey(m_group, "cue_play"));
    m_pCueGotoAndStop = std::make_unique<ControlPushButton>(ConfigKey(m_group, "cue_gotoandstop"));
    m_pCuePreview = std::make_unique<ControlPushButton>(ConfigKey(m_group, "cue_preview"));
    m_pCueCDJ = std::make_unique<ControlPushButton>(ConfigKey(m_group, "cue_cdj"));
    m_pCueDefault = std::make_unique<ControlPushButton>(ConfigKey(m_group, "cue_default"));
    m_pPlayStutter = std::make_unique<ControlPushButton>(ConfigKey(m_group, "play_stutter"));

    m_pPlayLatched = std::make_unique<ControlObject>(ConfigKey(m_group, "play_latched"));
    m_pPlayLatched->setReadOnly();

    m_pCueIndicator = std::make_unique<ControlIndicator>(ConfigKey(m_group, "cue_indicator"));
    m_pPlayIndicator = std::make_unique<ControlIndicator>(ConfigKey(m_group, "play_indicator"));

    m_pIntroStartPosition = std::make_unique<ControlObject>(
            ConfigKey(m_group, "intro_start_position"));
    m_pIntroStartPosition->set(Cue::kNoPosition);
    m_pIntroStartEnabled = std::make_unique<ControlObject>(
            ConfigKey(m_group, "intro_start_enabled"));
    m_pIntroStartEnabled->setReadOnly();
    m_pIntroStartSet = std::make_unique<ControlPushButton>(ConfigKey(m_group, "intro_start_set"));
    m_pIntroStartClear = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "intro_start_clear"));
    m_pIntroStartActivate = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "intro_start_activate"));
    m_pIntroEndPosition = std::make_unique<ControlObject>(ConfigKey(m_group, "intro_end_position"));
    m_pIntroEndPosition->set(Cue::kNoPosition);
    m_pIntroEndEnabled = std::make_unique<ControlObject>(ConfigKey(m_group, "intro_end_enabled"));
    m_pIntroEndEnabled->setReadOnly();
    m_pIntroEndSet = std::make_unique<ControlPushButton>(ConfigKey(m_group, "intro_end_set"));
    m_pIntroEndClear = std::make_unique<ControlPushButton>(ConfigKey(m_group, "intro_end_clear"));
    m_pIntroEndActivate = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "intro_end_activate"));

    m_pOutroStartPosition = std::make_unique<ControlObject>(
            ConfigKey(m_group, "outro_start_position"));
    m_pOutroStartPosition->set(Cue::kNoPosition);
    m_pOutroStartEnabled = std::make_unique<ControlObject>(
            ConfigKey(m_group, "outro_start_enabled"));
    m_pOutroStartEnabled->setReadOnly();
    m_pOutroStartSet = std::make_unique<ControlPushButton>(ConfigKey(m_group, "outro_start_set"));
    m_pOutroStartClear = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "outro_start_clear"));
    m_pOutroStartActivate = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "outro_start_activate"));
    m_pOutroEndPosition = std::make_unique<ControlObject>(ConfigKey(m_group, "outro_end_position"));
    m_pOutroEndPosition->set(Cue::kNoPosition);
    m_pOutroEndEnabled = std::make_unique<ControlObject>(ConfigKey(m_group, "outro_end_enabled"));
    m_pOutroEndEnabled->setReadOnly();
    m_pOutroEndSet = std::make_unique<ControlPushButton>(ConfigKey(m_group, "outro_end_set"));
    m_pOutroEndClear = std::make_unique<ControlPushButton>(ConfigKey(m_group, "outro_end_clear"));
    m_pOutroEndActivate = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "outro_end_activate"));

    m_pVinylControlEnabled = std::make_unique<ControlProxy>(m_group, "vinylcontrol_enabled");
    m_pVinylControlMode = std::make_unique<ControlProxy>(m_group, "vinylcontrol_mode");

    m_pHotcueFocus = std::make_unique<ControlObject>(ConfigKey(m_group, "hotcue_focus"));
    setHotcueFocusIndex(Cue::kNoHotCue);
    m_pHotcueFocusColorPrev = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "hotcue_focus_color_prev"));
    m_pHotcueFocusColorNext = std::make_unique<ControlPushButton>(
            ConfigKey(m_group, "hotcue_focus_color_next"));

    // Create hotcue controls
    for (int i = 0; i < kMaxNumberOfHotcues; ++i) {
        HotcueControl* pControl = new HotcueControl(m_group, i);
        m_hotcueControls.append(pControl);
    }
}

void CueControl::connectControls() {
    // Main Cue controls
    connect(m_pCueSet.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::cueSet,
            Qt::DirectConnection);
    connect(m_pCueClear.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::cueClear,
            Qt::DirectConnection);
    connect(m_pCueGoto.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::cueGoto,
            Qt::DirectConnection);
    connect(m_pCueGotoAndPlay.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::cueGotoAndPlay,
            Qt::DirectConnection);
    connect(m_pCuePlay.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::cuePlay,
            Qt::DirectConnection);
    connect(m_pCueGotoAndStop.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::cueGotoAndStop,
            Qt::DirectConnection);
    connect(m_pCuePreview.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::cuePreview,
            Qt::DirectConnection);
    connect(m_pCueCDJ.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::cueCDJ,
            Qt::DirectConnection);
    connect(m_pCueDefault.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::cueDefault,
            Qt::DirectConnection);
    connect(m_pPlayStutter.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::playStutter,
            Qt::DirectConnection);

    connect(m_pIntroStartSet.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::introStartSet,
            Qt::DirectConnection);
    connect(m_pIntroStartClear.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::introStartClear,
            Qt::DirectConnection);
    connect(m_pIntroStartActivate.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::introStartActivate,
            Qt::DirectConnection);
    connect(m_pIntroEndSet.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::introEndSet,
            Qt::DirectConnection);
    connect(m_pIntroEndClear.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::introEndClear,
            Qt::DirectConnection);
    connect(m_pIntroEndActivate.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::introEndActivate,
            Qt::DirectConnection);

    connect(m_pOutroStartSet.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::outroStartSet,
            Qt::DirectConnection);
    connect(m_pOutroStartClear.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::outroStartClear,
            Qt::DirectConnection);
    connect(m_pOutroStartActivate.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::outroStartActivate,
            Qt::DirectConnection);
    connect(m_pOutroEndSet.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::outroEndSet,
            Qt::DirectConnection);
    connect(m_pOutroEndClear.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::outroEndClear,
            Qt::DirectConnection);
    connect(m_pOutroEndActivate.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::outroEndActivate,
            Qt::DirectConnection);

    connect(m_pHotcueFocusColorPrev.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::hotcueFocusColorPrev,
            Qt::DirectConnection);
    connect(m_pHotcueFocusColorNext.get(),
            &ControlObject::valueChanged,
            this,
            &CueControl::hotcueFocusColorNext,
            Qt::DirectConnection);

    // Hotcue controls
    for (const auto& pControl : std::as_const(m_hotcueControls)) {
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
        connect(pControl,
                &HotcueControl::hotcueSwap,
                this,
                &CueControl::hotcueSwap,
                Qt::DirectConnection);
    }
}

void CueControl::disconnectControls() {
    disconnect(m_pCueSet.get(), nullptr, this, nullptr);
    disconnect(m_pCueClear.get(), nullptr, this, nullptr);
    disconnect(m_pCueGoto.get(), nullptr, this, nullptr);
    disconnect(m_pCueGotoAndPlay.get(), nullptr, this, nullptr);
    disconnect(m_pCuePlay.get(), nullptr, this, nullptr);
    disconnect(m_pCueGotoAndStop.get(), nullptr, this, nullptr);
    disconnect(m_pCuePreview.get(), nullptr, this, nullptr);
    disconnect(m_pCueCDJ.get(), nullptr, this, nullptr);
    disconnect(m_pCueDefault.get(), nullptr, this, nullptr);
    disconnect(m_pPlayStutter.get(), nullptr, this, nullptr);

    disconnect(m_pIntroStartSet.get(), nullptr, this, nullptr);
    disconnect(m_pIntroStartClear.get(), nullptr, this, nullptr);
    disconnect(m_pIntroStartActivate.get(), nullptr, this, nullptr);
    disconnect(m_pIntroEndSet.get(), nullptr, this, nullptr);
    disconnect(m_pIntroEndClear.get(), nullptr, this, nullptr);
    disconnect(m_pIntroEndActivate.get(), nullptr, this, nullptr);

    disconnect(m_pOutroStartSet.get(), nullptr, this, nullptr);
    disconnect(m_pOutroStartClear.get(), nullptr, this, nullptr);
    disconnect(m_pOutroStartActivate.get(), nullptr, this, nullptr);
    disconnect(m_pOutroEndSet.get(), nullptr, this, nullptr);
    disconnect(m_pOutroEndClear.get(), nullptr, this, nullptr);
    disconnect(m_pOutroEndActivate.get(), nullptr, this, nullptr);

    disconnect(m_pHotcueFocusColorPrev.get(), nullptr, this, nullptr);
    disconnect(m_pHotcueFocusColorNext.get(), nullptr, this, nullptr);

    for (const auto& pControl : std::as_const(m_hotcueControls)) {
        disconnect(pControl, nullptr, this, nullptr);
    }
}

void CueControl::passthroughChanged(double enabled) {
    if (enabled > 0) {
        // If passthrough was enabled seeking and playing is prohibited, and the
        // waveform and overview are blocked.
        // Disconnect all cue controls to prevent cue changes without UI feedback.
        disconnectControls();
    } else {
        // Reconnect all controls when deck returns to regular mode.
        connectControls();
    }
}

void CueControl::setHotcueIndicesSortedByPosition(double v) {
    if (v > 0 && m_pLoadedTrack) {
        m_pLoadedTrack->setHotcueIndicesSortedByPosition(HotcueSortMode::KeepOffsets);
    }
}

void CueControl::setHotcueIndicesSortedByPositionCompress(double v) {
    if (v > 0 && m_pLoadedTrack) {
        m_pLoadedTrack->setHotcueIndicesSortedByPosition(HotcueSortMode::RemoveOffsets);
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
    auto lock = lockMutex(&m_trackMutex);
    if (m_pLoadedTrack) {
        disconnect(m_pLoadedTrack.get(), nullptr, this, nullptr);

        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);

        for (const auto& pControl : std::as_const(m_hotcueControls)) {
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
        m_n60dBSoundStartPosition.setValue(Cue::kNoPosition);
        setHotcueFocusIndex(Cue::kNoHotCue);
        m_pLoadedTrack.reset();
        m_usedSeekOnLoadPosition.setValue(mixxx::audio::kStartFramePos);
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

    connect(m_pLoadedTrack.get(),
            &Track::loopRemove,
            this,
            &CueControl::loopRemove);

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
            seekOnLoad(mixxx::audio::kStartFramePos);
        }
        return;
    case SeekOnLoadMode::FirstSound: {
        CuePointer pN60dBSound =
                pNewTrack->findCueByType(mixxx::CueType::N60dBSound);
        mixxx::audio::FramePos n60dBSoundPosition;
        if (pN60dBSound) {
            n60dBSoundPosition = pN60dBSound->getPosition();
        }
        if (n60dBSoundPosition.isValid()) {
            seekOnLoad(n60dBSoundPosition);
        } else {
            seekOnLoad(mixxx::audio::kStartFramePos);
        }
        break;
    }
    case SeekOnLoadMode::MainCue: {
        // Take main cue position from CO instead of cue point list because
        // value in CO will be quantized if quantization is enabled
        // while value in cue point list will never be quantized.
        // This prevents jumps when track analysis finishes while quantization is enabled.
        const auto mainCuePosition =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_pCuePoint->get());
        if (mainCuePosition.isValid()) {
            seekOnLoad(mainCuePosition);
            return;
        }
        break;
    }
    case SeekOnLoadMode::FirstHotcue: {
        mixxx::audio::FramePos firstHotcuePosition;
        HotcueControl* pControl = m_hotcueControls.value(0, nullptr);
        if (pControl) {
            firstHotcuePosition = pControl->getPosition();
            if (firstHotcuePosition.isValid()) {
                seekOnLoad(firstHotcuePosition);
                return;
            }
        }
        break;
    }
    case SeekOnLoadMode::IntroStart: {
        const auto introStartPosition =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_pIntroStartPosition->get());
        if (introStartPosition.isValid()) {
            seekOnLoad(introStartPosition);
            return;
        }
        break;
    }
    default:
        DEBUG_ASSERT(!"Unknown enum value");
        break;
    }
    seekOnLoad(mixxx::audio::kStartFramePos);
}

void CueControl::seekOnLoad(mixxx::audio::FramePos seekOnLoadPosition) {
    DEBUG_ASSERT(seekOnLoadPosition.isValid());
    seekExact(seekOnLoadPosition);
    m_usedSeekOnLoadPosition.setValue(seekOnLoadPosition);
}

void CueControl::cueUpdated() {
    //auto lock = lockMutex(&m_mutex);
    // We should get a trackCuesUpdated call anyway, so do nothing.
}

void CueControl::loadCuesFromTrack() {
    auto lock = lockMutex(&m_trackMutex);
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
            HotcueControl* pControl = m_hotcueControls.value(hotcue, nullptr);

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
        case mixxx::CueType::N60dBSound: {
            Cue::StartAndEndPositions pos = pCue->getStartAndEndPosition();
            m_n60dBSoundStartPosition.setValue(pos.startPosition.toEngineSamplePos());
            break;
        }
        case mixxx::CueType::Beat:
        case mixxx::CueType::Jump:
        case mixxx::CueType::Invalid:
        default:
            break;
        }
    }

    // Detach all hotcues that are no longer present
    for (int hotCueIndex = 0; hotCueIndex < kMaxNumberOfHotcues; ++hotCueIndex) {
        if (!active_hotcues.contains(hotCueIndex)) {
            HotcueControl* pControl = m_hotcueControls.at(hotCueIndex);
            detachCue(pControl);
        }
    }

    if (pIntroCue) {
        const auto startPosition = quantizeCuePoint(pIntroCue->getPosition());
        const auto endPosition = quantizeCuePoint(pIntroCue->getEndPosition());

        m_pIntroStartPosition->set(startPosition.toEngineSamplePosMaybeInvalid());
        m_pIntroStartEnabled->forceSet(startPosition.isValid());
        m_pIntroEndPosition->set(endPosition.toEngineSamplePosMaybeInvalid());
        m_pIntroEndEnabled->forceSet(endPosition.isValid());
    } else {
        m_pIntroStartPosition->set(Cue::kNoPosition);
        m_pIntroStartEnabled->forceSet(0.0);
        m_pIntroEndPosition->set(Cue::kNoPosition);
        m_pIntroEndEnabled->forceSet(0.0);
    }

    if (pOutroCue) {
        const auto startPosition = quantizeCuePoint(pOutroCue->getPosition());
        const auto endPosition = quantizeCuePoint(pOutroCue->getEndPosition());

        m_pOutroStartPosition->set(startPosition.toEngineSamplePosMaybeInvalid());
        m_pOutroStartEnabled->forceSet(startPosition.isValid());
        m_pOutroEndPosition->set(endPosition.toEngineSamplePosMaybeInvalid());
        m_pOutroEndEnabled->forceSet(endPosition.isValid());
    } else {
        m_pOutroStartPosition->set(Cue::kNoPosition);
        m_pOutroStartEnabled->forceSet(0.0);
        m_pOutroEndPosition->set(Cue::kNoPosition);
        m_pOutroEndEnabled->forceSet(0.0);
    }

    // Because of legacy, we store the main cue point twice and need to
    // sync both values.
    // The mixxx::CueType::MainCue from getCuePoints() has the priority
    mixxx::audio::FramePos mainCuePosition;
    if (pMainCue) {
        mainCuePosition = pMainCue->getPosition();
        // adjust the track cue accordingly
        m_pLoadedTrack->setMainCuePosition(mainCuePosition);
    } else {
        // If no load cue point is stored, read from track
        // Note: This is mixxx::audio::kStartFramePos for new tracks
        // and always a valid position.
        mainCuePosition = m_pLoadedTrack->getMainCuePosition();
        // A main cue point only needs to be added if the position
        // differs from the default position.
        if (mainCuePosition.isValid() &&
                mainCuePosition != mixxx::audio::kStartFramePos) {
            qInfo()
                    << "Adding missing main cue point at"
                    << mainCuePosition
                    << "for track"
                    << m_pLoadedTrack->getLocation();
            m_pLoadedTrack->createAndAddCue(
                    mixxx::CueType::MainCue,
                    Cue::kNoHotCue,
                    mainCuePosition,
                    mixxx::audio::kInvalidFramePos);
        }
    }

    DEBUG_ASSERT(mainCuePosition.isValid());
    const auto quantizedMainCuePosition = quantizeCuePoint(mainCuePosition);
    m_pCuePoint->set(quantizedMainCuePosition.toEngineSamplePosMaybeInvalid());
}

void CueControl::trackAnalyzed() {
    if (frameInfo().currentPosition != m_usedSeekOnLoadPosition.getValue()) {
        // the track is already manual cued, don't re-cue
        return;
    }

    // Make track follow the updated cues.
    SeekOnLoadMode seekOnLoadMode = getSeekOnLoadPreference();

    switch (seekOnLoadMode) {
    case SeekOnLoadMode::MainCue: {
        const auto position =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_pCuePoint->get());
        if (position.isValid()) {
            seekOnLoad(position);
        }
        break;
    }
    case SeekOnLoadMode::IntroStart: {
        const auto position =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_pIntroStartPosition->get());
        if (position.isValid()) {
            seekOnLoad(position);
        }
        break;
    }
    default:
        // nothing to do here
        break;
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
    const auto cuePosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pCuePoint->get());
    if (wasTrackAtCue && cuePosition.isValid()) {
        seekExact(cuePosition);
    }
    // Retrieve new intro start pos and follow
    const auto introPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pIntroStartPosition->get());
    if (wasTrackAtIntro && introPosition.isValid()) {
        seekExact(introPosition);
    }
}

mixxx::RgbColor CueControl::colorFromConfig(const ConfigKey& configKey) {
    auto hotcueColorPalette =
            m_colorPaletteSettings.getHotcueColorPalette();
    int colorIndex = m_pConfig->getValue(configKey, -1);
    if (colorIndex < 0 || colorIndex >= hotcueColorPalette.size()) {
        return hotcueColorPalette.defaultColor();
    }
    return hotcueColorPalette.at(colorIndex);
};

void CueControl::hotcueSet(HotcueControl* pControl, double value, HotcueSetMode mode) {
    //qDebug() << "CueControl::hotcueSet" << value;

    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    if (!m_pLoadedTrack) {
        return;
    }

    // Note: the cue is just detached from the hotcue control
    // It remains in the database for later use
    // TODO: find a rule, that allows us to delete the cue as well
    // https://github.com/mixxxdj/mixxx/issues/8740
    hotcueClear(pControl, value);

    mixxx::audio::FramePos cueStartPosition;
    mixxx::audio::FramePos cueEndPosition;
    mixxx::CueType cueType = mixxx::CueType::Invalid;

    bool loopEnabled = m_pLoopEnabled->toBool();
    if (mode == HotcueSetMode::Auto) {
        if (loopEnabled) {
            // Don't create a hotcue at loop start if there is one already.
            // This allows to set a hotuce inside an active, saved loop with
            // 'hotcue_X_activate'.
            auto* pSavedLoopControl = m_pCurrentSavedLoopControl.loadAcquire();
            if (pSavedLoopControl &&
                    pSavedLoopControl->getPosition() ==
                            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                                    m_pLoopStartPosition->get()) &&
                    pSavedLoopControl->getEndPosition() ==
                            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                                    m_pLoopEndPosition->get())) {
                mode = HotcueSetMode::Cue;
            } else {
                mode = HotcueSetMode::Loop;
            }
        } else {
            mode = HotcueSetMode::Cue;
        }
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
            cueStartPosition =
                    mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                            m_pLoopStartPosition->get());
            cueEndPosition =
                    mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                            m_pLoopEndPosition->get());
        } else {
            // If no loop is enabled, save a loop starting from the current
            // position and with the current beatloop size
            cueStartPosition = getQuantizedCurrentPosition();
            double beatloopSize = m_pBeatLoopSize->get();
            const mixxx::BeatsPointer pBeats = m_pLoadedTrack->getBeats();
            if (beatloopSize <= 0 || !pBeats) {
                return;
            }
            if (cueStartPosition.isValid()) {
                cueEndPosition = pBeats->findNBeatsFromPosition(cueStartPosition, beatloopSize);
            }
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
    VERIFY_OR_DEBUG_ASSERT(cueStartPosition.isValid() &&
            (cueType != mixxx::CueType::Loop || cueEndPosition.isValid())) {
        return;
    }

    int hotcueIndex = pControl->getHotcueIndex();

    mixxx::RgbColor color = mixxx::PredefinedColorPalettes::kDefaultCueColor;
    if (cueType == mixxx::CueType::Loop) {
        ConfigKey autoLoopColorsKey("[Controls]", "auto_loop_colors");
        if (getConfig()->getValue(autoLoopColorsKey, false)) {
            color = m_colorPaletteSettings.getHotcueColorPalette().colorForHotcueIndex(hotcueIndex);
        } else {
            color = colorFromConfig(ConfigKey("[Controls]", "LoopDefaultColorIndex"));
        }
    } else {
        ConfigKey autoHotcueColorsKey("[Controls]", "auto_hotcue_colors");
        if (getConfig()->getValue(autoHotcueColorsKey, false)) {
            color = m_colorPaletteSettings.getHotcueColorPalette().colorForHotcueIndex(hotcueIndex);
        } else {
            color = colorFromConfig(ConfigKey("[Controls]", "HotcueDefaultColorIndex"));
        }
    }

    CuePointer pCue = m_pLoadedTrack->createAndAddCue(
            cueType,
            hotcueIndex,
            cueStartPosition,
            cueEndPosition,
            color);

    // TODO(XXX) deal with spurious signals
    attachCue(pCue, pControl);

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
    const mixxx::audio::FramePos position = pControl->getPosition();
    if (position.isValid()) {
        seekAbs(position);
    }
}

void CueControl::hotcueGotoAndStop(HotcueControl* pControl, double value) {
    if (value <= 0) {
        return;
    }

    const mixxx::audio::FramePos position = pControl->getPosition();
    if (!position.isValid()) {
        return;
    }

    if (m_currentlyPreviewingIndex == Cue::kNoHotCue) {
        m_pPlay->set(0.0);
        seekExact(position);
    } else {
        // this becomes a play latch command if we are previewing
        m_pPlay->set(0.0);
    }
}

void CueControl::hotcueGotoAndPlay(HotcueControl* pControl, double value) {
    if (value <= 0) {
        return;
    }
    const mixxx::audio::FramePos position = pControl->getPosition();
    if (position.isValid()) {
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

    const mixxx::audio::FramePos startPosition = pCue->getPosition();
    if (!startPosition.isValid()) {
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

    if (!pCue || !pCue->getPosition().isValid()) {
        hotcueSet(pControl, value, HotcueSetMode::Cue);
        pCue = pControl->getCue();
        VERIFY_OR_DEBUG_ASSERT(pCue && pCue->getPosition().isValid()) {
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
        const mixxx::audio::FramePos startPosition = pCue->getPosition();
        const bool loopActive = m_pLoopEnabled->toBool() &&
                startPosition ==
                        mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                                m_pLoopStartPosition->get());
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
        if (pCue && pCue->getPosition().isValid() &&
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
            const mixxx::audio::FramePos position = pControl->getPreviewingPosition();
            mixxx::CueType type = pControl->getPreviewingType();
            if (type != mixxx::CueType::Invalid && position.isValid()) {
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
        const mixxx::audio::FramePos position = pControl->getPreviewingPosition();
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        m_pPlay->set(0.0);
        if (position.isValid()) {
            seekExact(position);
        }
    }

    setHotcueFocusIndex(pControl->getHotcueIndex());
}

void CueControl::updateCurrentlyPreviewingIndex(int hotcueIndex) {
    int oldPreviewingIndex = m_currentlyPreviewingIndex.fetchAndStoreRelease(hotcueIndex);
    if (oldPreviewingIndex >= 0 && oldPreviewingIndex < kMaxNumberOfHotcues) {
        // We where already in previewing state, clean up ..
        HotcueControl* pLastControl = m_hotcueControls.at(oldPreviewingIndex);
        mixxx::CueType lastType = pLastControl->getPreviewingType();
        if (lastType == mixxx::CueType::Loop) {
            m_pLoopEnabled->set(0);
        }
        CuePointer pLastCue(pLastControl->getCue());
        if (pLastCue && pLastCue->getType() != mixxx::CueType::Invalid) {
            pLastControl->setStatus(HotcueControl::Status::Set);
        }
    }
}

void CueControl::hotcueClear(HotcueControl* pControl, double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
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

void CueControl::hotcueSwap(HotcueControl* pControl, double v) {
    // 1-based GUI/human index to 0-based internal index
    int newCuenum = static_cast<int>(v) - 1;
    if (newCuenum < mixxx::kFirstHotCueIndex || newCuenum >= kMaxNumberOfHotcues) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }

    m_pLoadedTrack->swapHotcues(pCue->getHotCue(), newCuenum);
}

void CueControl::hotcuePositionChanged(
        HotcueControl* pControl, double value) {
    auto lock = lockMutex(&m_trackMutex);
    if (!m_pLoadedTrack) {
        return;
    }

    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }

    const auto newPosition = mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(value);
    // Setting the position to Cue::kNoPosition is the same as calling hotcue_x_clear
    if (!newPosition.isValid()) {
        detachCue(pControl);
        return;
    }

    // TODO: Remove this check if we support positions < 0
    const auto trackEndPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pTrackSamples->get());
    if (newPosition <= mixxx::audio::kStartFramePos ||
            (trackEndPosition.isValid() && newPosition >= trackEndPosition)) {
        return;
    }

    if (pCue->getType() == mixxx::CueType::Loop && newPosition >= pCue->getEndPosition()) {
        return;
    }
    pCue->setStartPosition(newPosition);
}

void CueControl::hotcueEndPositionChanged(
        HotcueControl* pControl, double value) {
    CuePointer pCue = pControl->getCue();
    if (!pCue) {
        return;
    }

    const auto newEndPosition = mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(value);

    // Setting the end position of a loop cue to Cue::kNoPosition converts
    // it into a regular jump cue
    if (pCue->getType() == mixxx::CueType::Loop && !newEndPosition.isValid()) {
        pCue->setType(mixxx::CueType::HotCue);
        pCue->setEndPosition(mixxx::audio::kInvalidFramePos);
    } else {
        const mixxx::audio::FramePos startPosition = pCue->getPosition();
        if (startPosition.isValid() && newEndPosition > startPosition) {
            pCue->setEndPosition(newEndPosition);
        }
    }
}

void CueControl::hintReader(gsl::not_null<HintVector*> pHintList) {
    appendCueHint(pHintList, m_pCuePoint->get(), Hint::Type::MainCue);

    // this is called from the engine thread
    // it is no locking required, because m_hotcueControl is filled during the
    // constructor and getPosition()->get() is a ControlObject
    for (const auto& pControl : std::as_const(m_hotcueControls)) {
        appendCueHint(pHintList, pControl->getPosition(), Hint::Type::HotCue);
    }

    appendCueHint(pHintList, m_n60dBSoundStartPosition.getValue(), Hint::Type::FirstSound);
    appendCueHint(pHintList, m_pIntroStartPosition->get(), Hint::Type::IntroStart);
    appendCueHint(pHintList, m_pIntroEndPosition->get(), Hint::Type::IntroEnd);
    appendCueHint(pHintList, m_pOutroStartPosition->get(), Hint::Type::OutroStart);
}

// Moves the cue point to current position or to closest beat in case
// quantize is enabled
void CueControl::cueSet(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    const mixxx::audio::FramePos position = getQuantizedCurrentPosition();
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Store cue point in loaded track
    // The m_pCuePoint CO is set via loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        pLoadedTrack->setMainCuePosition(position);
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
        pLoadedTrack->setMainCuePosition(mixxx::audio::kStartFramePos);
    }
}

void CueControl::cueGoto(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    // Seek to cue point
    const auto mainCuePosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pCuePoint->get());

    // Note: We do not mess with play here, we continue playing or previewing.

    // Need to unlock before emitting any signals to prevent deadlock.
    lock.unlock();

    if (mainCuePosition.isValid()) {
        seekAbs(mainCuePosition);
    }
}

void CueControl::cueGotoAndPlay(double value) {
    if (value <= 0) {
        return;
    }

    cueGoto(value);
    auto lock = lockMutex(&m_trackMutex);
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
        const auto mainCuePosition =
                mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_pCuePoint->get());
        if (mainCuePosition.isValid()) {
            seekExact(mainCuePosition);
        }
    } else {
        // this becomes a play latch command if we are previewing
        m_pPlay->set(0.0);
    }
}

void CueControl::cuePreview(double value) {
    const auto mainCuePosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pCuePoint->get());
    if (!mainCuePosition.isValid()) {
        return;
    }

    if (value > 0) {
        if (m_currentlyPreviewingIndex == kMainCueIndex) {
            return;
        }

        updateCurrentlyPreviewingIndex(kMainCueIndex);
        seekAbs(mainCuePosition);
        m_pPlay->set(1.0);
    } else if (m_currentlyPreviewingIndex == kMainCueIndex) {
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        m_pPlay->set(0.0);
        seekExact(mainCuePosition);
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

    const auto mainCuePosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pCuePoint->get());
    if (!mainCuePosition.isValid()) {
        return;
    }

    if (value > 0) {
        if (m_currentlyPreviewingIndex == kMainCueIndex) {
            // already previewing, do nothing
            return;
        } else if (m_currentlyPreviewingIndex != Cue::kNoHotCue) {
            // we are already previewing by hotcues
            // just jump to cue point and continue previewing
            updateCurrentlyPreviewingIndex(kMainCueIndex);
            seekAbs(mainCuePosition);
        } else if (freely_playing || trackAt == TrackAt::End) {
            // Jump to cue when playing or when at end position
            m_pPlay->set(0.0);
            seekAbs(mainCuePosition);
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
                // We need to re-get the cue point since it changed.
                const auto newCuePosition = mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_pCuePoint->get());
                if (newCuePosition.isValid()) {
                    // Enginebuffer will quantize more exactly than we can.
                    seekAbs(newCuePosition);
                }
            }
        }
    } else if (m_currentlyPreviewingIndex == kMainCueIndex) {
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        m_pPlay->set(0.0);
        // Need to unlock before emitting any signals to prevent deadlock.
        seekExact(mainCuePosition);
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

    const auto mainCuePosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pCuePoint->get());
    if (!mainCuePosition.isValid()) {
        return;
    }

    if (value > 0) {
        if (m_currentlyPreviewingIndex == kMainCueIndex) {
            // already previewing, do nothing
            return;
        } else if (m_currentlyPreviewingIndex != Cue::kNoHotCue) {
            // we are already previewing by hotcues
            // just jump to cue point and continue previewing
            updateCurrentlyPreviewingIndex(kMainCueIndex);
            seekAbs(mainCuePosition);
        } else if (!playing && trackAt == TrackAt::Cue) {
            // paused at cue point
            updateCurrentlyPreviewingIndex(kMainCueIndex);
            m_pPlay->set(1.0);
        } else {
            m_pPlay->set(0.0);
            seekExact(mainCuePosition);
        }
    } else if (m_currentlyPreviewingIndex == kMainCueIndex) {
        updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
        m_pPlay->set(0.0);
        seekExact(mainCuePosition);
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

    const auto mainCuePosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pCuePoint->get());
    if (!mainCuePosition.isValid()) {
        return;
    }

    // pressed
    if (value > 0) {
        if (freely_playing) {
            updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
            m_pPlay->set(0.0);
            seekAbs(mainCuePosition);
        } else if (trackAt == TrackAt::ElseWhere) {
            // Pause not at cue point and not at end position
            cueSet(value);
            // Just in case.
            updateCurrentlyPreviewingIndex(Cue::kNoHotCue);
            m_pPlay->set(0.0);
            // If quantize is enabled, jump to the cue point since it's not
            // necessarily where we currently are
            if (m_pQuantizeEnabled->toBool()) {
                const auto newCuePosition = mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                        m_pCuePoint->get());
                if (newCuePosition.isValid()) {
                    // Enginebuffer will quantize more exactly than we can.
                    seekAbs(newCuePosition);
                }
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
    auto lock = lockMutex(&m_trackMutex);
    //qDebug() << "CueControl::pause()" << v;
    if (v > 0.0) {
        m_pPlay->set(0.0);
    }
}

void CueControl::playStutter(double v) {
    auto lock = lockMutex(&m_trackMutex);
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

    auto lock = lockMutex(&m_trackMutex);

    const mixxx::audio::FramePos position = getQuantizedCurrentPosition();
    if (!position.isValid()) {
        return;
    }

    // Make sure user is not trying to place intro start cue on or after
    // other intro/outro cues.
    const auto introEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pIntroEndPosition->get());
    const auto outroStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pOutroStartPosition->get());
    const auto outroEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pOutroEndPosition->get());
    if (introEnd.isValid() && position >= introEnd) {
        qWarning()
                << "Trying to place intro start cue on or after intro end cue.";
        return;
    }
    if (outroStart.isValid() && position >= outroStart) {
        qWarning() << "Trying to place intro start cue on or after outro start "
                      "cue.";
        return;
    }
    if (outroEnd.isValid() && position >= outroEnd) {
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

    auto lock = lockMutex(&m_trackMutex);
    const auto introEndPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pIntroEndPosition->get());
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Intro);
        if (introEndPosition.isValid()) {
            pCue->setStartPosition(mixxx::audio::kInvalidFramePos);
            pCue->setEndPosition(introEndPosition);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::introStartActivate(double value) {
    if (value <= 0) {
        return;
    }

    const auto introStartPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pIntroStartPosition->get());
    if (introStartPosition.isValid()) {
        seekAbs(introStartPosition);
    } else {
        introStartSet(1.0);
    }
}

void CueControl::introEndSet(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);

    const mixxx::audio::FramePos position = getQuantizedCurrentPosition();
    if (!position.isValid()) {
        return;
    }

    // Make sure user is not trying to place intro end cue on or before
    // intro start cue, or on or after outro start/end cue.
    const auto introStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pIntroStartPosition->get());
    const auto outroStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pOutroStartPosition->get());
    const auto outroEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pOutroEndPosition->get());
    if (introStart.isValid() && position <= introStart) {
        qWarning() << "Trying to place intro end cue on or before intro start "
                      "cue.";
        return;
    }
    if (outroStart.isValid() && position >= outroStart) {
        qWarning()
                << "Trying to place intro end cue on or after outro start cue.";
        return;
    }
    if (outroEnd.isValid() && position >= outroEnd) {
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

    auto lock = lockMutex(&m_trackMutex);
    const auto introStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pIntroStartPosition->get());
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Intro);
        if (introStart.isValid()) {
            pCue->setStartPosition(introStart);
            pCue->setEndPosition(mixxx::audio::kInvalidFramePos);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::introEndActivate(double value) {
    if (value == 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    const auto introEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pIntroEndPosition->get());
    lock.unlock();

    if (introEnd.isValid()) {
        seekAbs(introEnd);
    } else {
        introEndSet(1.0);
    }
}

void CueControl::outroStartSet(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);

    const mixxx::audio::FramePos position = getQuantizedCurrentPosition();
    if (!position.isValid()) {
        return;
    }

    // Make sure user is not trying to place outro start cue on or before
    // intro end cue or on or after outro end cue.
    const auto introStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pIntroStartPosition->get());
    const auto introEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pIntroEndPosition->get());
    const auto outroEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pOutroEndPosition->get());
    if (introStart.isValid() && position <= introStart) {
        qWarning() << "Trying to place outro start cue on or before intro "
                      "start cue.";
        return;
    }
    if (introEnd.isValid() && position <= introEnd) {
        qWarning() << "Trying to place outro start cue on or before intro end "
                      "cue.";
        return;
    }
    if (outroEnd.isValid() && position >= outroEnd) {
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

    auto lock = lockMutex(&m_trackMutex);
    const auto outroEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pOutroEndPosition->get());
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Outro);
        if (outroEnd.isValid()) {
            pCue->setStartPosition(mixxx::audio::kInvalidFramePos);
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

    auto lock = lockMutex(&m_trackMutex);
    const auto outroStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pOutroStartPosition->get());
    lock.unlock();

    if (outroStart.isValid()) {
        seekAbs(outroStart);
    } else {
        outroStartSet(1.0);
    }
}

void CueControl::outroEndSet(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);

    const mixxx::audio::FramePos position = getQuantizedCurrentPosition();
    if (!position.isValid()) {
        return;
    }

    // Make sure user is not trying to place outro end cue on or before
    // other intro/outro cues.
    const auto introStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pIntroStartPosition->get());
    const auto introEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pIntroEndPosition->get());
    const auto outroStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pOutroStartPosition->get());
    if (introStart.isValid() && position <= introStart) {
        qWarning() << "Trying to place outro end cue on or before intro start "
                      "cue.";
        return;
    }
    if (introEnd.isValid() && position <= introEnd) {
        qWarning()
                << "Trying to place outro end cue on or before intro end cue.";
        return;
    }
    if (outroStart.isValid() && position <= outroStart) {
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

    auto lock = lockMutex(&m_trackMutex);
    const auto outroStart =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pOutroStartPosition->get());
    TrackPointer pLoadedTrack = m_pLoadedTrack;
    lock.unlock();

    // Update Track's cue.
    // CO's are updated in loadCuesFromTrack()
    // this can be done outside the locking scope
    if (pLoadedTrack) {
        CuePointer pCue = pLoadedTrack->findCueByType(mixxx::CueType::Outro);
        if (outroStart.isValid()) {
            pCue->setStartPosition(outroStart);
            pCue->setEndPosition(mixxx::audio::kInvalidFramePos);
        } else if (pCue) {
            pLoadedTrack->removeCue(pCue);
        }
    }
}

void CueControl::outroEndActivate(double value) {
    if (value <= 0) {
        return;
    }

    auto lock = lockMutex(&m_trackMutex);
    const auto outroEnd =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pOutroEndPosition->get());
    lock.unlock();

    if (outroEnd.isValid()) {
        seekAbs(outroEnd);
    } else {
        outroEndSet(1.0);
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
            int oldPreviewingIndex =
                    m_currentlyPreviewingIndex.fetchAndStoreRelease(
                            Cue::kNoHotCue);
            if (oldPreviewingIndex >= 0 && oldPreviewingIndex < kMaxNumberOfHotcues) {
                HotcueControl* pLastControl = m_hotcueControls.at(oldPreviewingIndex);
                mixxx::CueType lastType = pLastControl->getPreviewingType();
                if (lastType != mixxx::CueType::Loop) {
                    CuePointer pLastCue(pLastControl->getCue());
                    if (pLastCue && pLastCue->getType() != mixxx::CueType::Invalid) {
                        pLastControl->setStatus(HotcueControl::Status::Set);
                    }
                }
            }
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
    FrameInfo info = frameInfo();
    // Note: current can be in the padded silence after the track end > total.
    if (info.trackEndPosition.isValid() && info.currentPosition >= info.trackEndPosition) {
        return TrackAt::End;
    }
    const auto mainCuePosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pCuePoint->get());
    if (mainCuePosition.isValid() && fabs(info.currentPosition - mainCuePosition) < 0.5) {
        return TrackAt::Cue;
    }
    return TrackAt::ElseWhere;
}

mixxx::audio::FramePos CueControl::getQuantizedCurrentPosition() {
    FrameInfo info = frameInfo();

    // Note: currentPos can be past the end of the track, in the padded
    // silence of the last buffer. This position might be not reachable in
    // a future runs, depending on the buffering.

    // Don't quantize if quantization is disabled.
    if (!m_pQuantizeEnabled->toBool()) {
        return info.currentPosition;
    }

    const auto closestBeat =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pClosestBeat->get());
    // Note: closestBeat can be an interpolated beat past the end of the track,
    // which cannot be reached.
    if (closestBeat.isValid() && info.trackEndPosition.isValid() &&
            closestBeat <= info.trackEndPosition) {
        return closestBeat;
    }

    return info.currentPosition;
}

mixxx::audio::FramePos CueControl::quantizeCuePoint(mixxx::audio::FramePos position) {
    // Don't quantize unset cues.
    if (!position.isValid()) {
        return mixxx::audio::kInvalidFramePos;
    }

    // We need to use m_pTrackSamples here because FrameInfo is set later by
    // the engine and not during EngineBuffer::slotTrackLoaded.
    const auto trackEndPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pTrackSamples->get());

    VERIFY_OR_DEBUG_ASSERT(trackEndPosition.isValid()) {
        return mixxx::audio::kInvalidFramePos;
    }

    // Don't quantize when quantization is disabled.
    if (!m_pQuantizeEnabled->toBool()) {
        return position;
    }

    if (position > trackEndPosition) {
        // This can happen if the track length has changed or the cue was set in the
        // the padded silence after the track.
        position = trackEndPosition;
    }

    const mixxx::BeatsPointer pBeats = m_pLoadedTrack->getBeats();
    if (!pBeats) {
        return position;
    }

    const auto quantizedPosition = pBeats->findClosestBeat(position);
    // The closest beat can be an unreachable interpolated beat past the end of
    // the track.
    if (quantizedPosition.isValid() && quantizedPosition <= trackEndPosition) {
        return quantizedPosition;
    }

    return position;
}

bool CueControl::isTrackAtIntroCue() {
    const auto introStartPosition =
            mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                    m_pIntroStartPosition->get());
    return introStartPosition.isValid() &&
            (fabs(frameInfo().currentPosition - introStartPosition) < 0.5);
}

SeekOnLoadMode CueControl::getSeekOnLoadPreference() {
    return getConfig()->getValue(ConfigKey("[Controls]", "CueRecall"), SeekOnLoadMode::IntroStart);
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
            pos.startPosition.isValid() &&
            pos.endPosition.isValid()) {
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

    if (pSavedLoopControl->getType() != mixxx::CueType::Loop) {
        slotLoopReset();
        return;
    }

    DEBUG_ASSERT(pSavedLoopControl->getStatus() != HotcueControl::Status::Empty);
    DEBUG_ASSERT(pSavedLoopControl->getCue() &&
            pSavedLoopControl->getCue()->getPosition() ==
                    mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                            m_pLoopStartPosition->get()));
    DEBUG_ASSERT(pSavedLoopControl->getCue() &&
            pSavedLoopControl->getCue()->getEndPosition() ==
                    mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(
                            m_pLoopEndPosition->get()));

    if (enabled) {
        pSavedLoopControl->setStatus(HotcueControl::Status::Active);
    } else {
        pSavedLoopControl->setStatus(HotcueControl::Status::Set);
    }
}

void CueControl::slotLoopUpdated(mixxx::audio::FramePos startPosition,
        mixxx::audio::FramePos endPosition) {
    HotcueControl* pSavedLoopControl = m_pCurrentSavedLoopControl;
    if (!pSavedLoopControl) {
        return;
    }

    if (pSavedLoopControl->getStatus() != HotcueControl::Status::Active ||
            pSavedLoopControl->getType() != mixxx::CueType::Loop) {
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

    VERIFY_OR_DEBUG_ASSERT(startPosition.isValid() && endPosition.isValid() &&
            startPosition < endPosition) {
        return;
    }

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
    m_pHotcueStatus->addAlias(keyForControl(QStringLiteral("enabled")));

    m_hotcueType = std::make_unique<ControlObject>(keyForControl(QStringLiteral("type")));
    m_hotcueType->setReadOnly();

    // The rgba value  of the color assigned to this color.
    m_hotcueColor = std::make_unique<ControlObject>(keyForControl(QStringLiteral("color")));
    m_hotcueColor->connectValueChangeRequest(
            this,
            &HotcueControl::slotHotcueColorChangeRequest,
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

    m_hotcueSwap = std::make_unique<ControlPushButton>(keyForControl(QStringLiteral("swap")));
    connect(m_hotcueSwap.get(),
            &ControlObject::valueChanged,
            this,
            &HotcueControl::slotHotcueSwap,
            Qt::DirectConnection);

    m_previewingType.setValue(mixxx::CueType::Invalid);
    m_previewingPosition.setValue(mixxx::audio::kInvalidFramePos);
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

void HotcueControl::slotHotcueSwap(double v) {
    emit hotcueSwap(this, v);
}

void HotcueControl::slotHotcuePositionChanged(double newPosition) {
    emit hotcuePositionChanged(this, newPosition);
}

void HotcueControl::slotHotcueEndPositionChanged(double newEndPosition) {
    emit hotcueEndPositionChanged(this, newEndPosition);
}

void HotcueControl::slotHotcueColorChangeRequest(double newColor) {
    if (newColor < 0 || newColor > 0xFFFFFF) {
        qWarning() << "slotHotcueColorChangeRequest got invalid value:" << newColor;
        return;
    }
    // qDebug() << "HotcueControl::slotHotcueColorChangeRequest" << newColor;
    if (!m_pCue) {
        return;
    }

    mixxx::RgbColor::optional_t color = doubleToRgbColor(newColor);
    VERIFY_OR_DEBUG_ASSERT(color) {
        return;
    }

    m_pCue->setColor(*color);
    m_hotcueColor->setAndConfirm(newColor);
}

mixxx::audio::FramePos HotcueControl::getPosition() const {
    return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(m_hotcuePosition->get());
}

mixxx::audio::FramePos HotcueControl::getEndPosition() const {
    return mixxx::audio::FramePos::fromEngineSamplePosMaybeInvalid(m_hotcueEndPosition->get());
}

void HotcueControl::setCue(const CuePointer& pCue) {
    DEBUG_ASSERT(!m_pCue);
    Cue::StartAndEndPositions pos = pCue->getStartAndEndPosition();
    setPosition(pos.startPosition);
    setEndPosition(pos.endPosition);
    // qDebug() << "HotcueControl::setCue";
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
    // qDebug() << "HotcueControl::setColor()" << newColor;
    if (newColor) {
        m_hotcueColor->setAndConfirm(*newColor);
    }
}

void HotcueControl::resetCue() {
    // clear pCue first because we have a null check for valid data else where
    // in the code
    m_pCue.reset();
    setPosition(mixxx::audio::kInvalidFramePos);
    setEndPosition(mixxx::audio::kInvalidFramePos);
    setType(mixxx::CueType::Invalid);
    setStatus(Status::Empty);
}

void HotcueControl::setPosition(mixxx::audio::FramePos position) {
    m_hotcuePosition->set(position.toEngineSamplePosMaybeInvalid());
}

void HotcueControl::setEndPosition(mixxx::audio::FramePos endPosition) {
    m_hotcueEndPosition->set(endPosition.toEngineSamplePosMaybeInvalid());
}

mixxx::CueType HotcueControl::getType() const {
    // Cast to int before casting to the int-based enum class because MSVC will
    // throw a hissy fit otherwise.
    return static_cast<mixxx::CueType>(static_cast<int>(m_hotcueType->get()));
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
